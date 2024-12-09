/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "main_window.h"

#include "data/data_manager.h"
#include "database/database.h"
#include "lists/event_roles_management_window.h"
#include "lists/event_types_management_window.h"
#include "lists/name_origins_management_window.h"
#include "main/person_list_widget.h"
#include "person_detail/person_detail_view.h"
#include "person_placeholder_widget.h"
#include "ui_settings.h"
#include "utils/placeholder_widget.h"
#include "welcome/welcome_view.h"

#include <KActionCollection>
#include <KConfigDialog>
#include <KLocalizedString>
#include <QDockWidget>
#include <QFileDialog>

void openOrSelectPerson(IntegerPrimaryKey personId, bool activate) {
    auto allWindows = QApplication::topLevelWidgets();
    const auto it = std::find_if(allWindows.constBegin(), allWindows.constEnd(), [](QWidget* element) {
        return qobject_cast<MainWindow*>(element) != nullptr;
    });

    if (it == allWindows.constEnd()) {
        qCritical() << "There does not seem to be a main window. How is that possible?";
        return;
    }

    auto* mainWindow = qobject_cast<MainWindow*>(*it);
    assert(mainWindow != nullptr);
    mainWindow->openOrSelectPerson(personId);
    if (activate) {
        mainWindow->activateWindow();
    }
}

MainWindow::MainWindow() {
    const KSharedConfigPtr config = KSharedConfig::openConfig();

    setAttribute(Qt::WA_DeleteOnClose);

    manageNameOrigins_ = new QAction(this);
    manageNameOrigins_->setText(i18n("Manage name origins"));
    connect(manageNameOrigins_, &QAction::triggered, this, &MainWindow::openNameOriginManager);

    manageEventRoles_ = new QAction(this);
    manageEventRoles_->setText(i18n("Manage event roles"));
    connect(manageEventRoles_, &QAction::triggered, this, &MainWindow::openEventRolesManager);

    manageEventTypes_ = new QAction(this);
    manageEventTypes_->setText(i18n("Manage event types"));
    connect(manageEventTypes_, &QAction::triggered, this, &MainWindow::openEventTypesManager);

    auto* actionCollection = KXMLGUIClient::actionCollection();
    actionCollection->addAction(QStringLiteral("manage_name_origins"), manageNameOrigins_);
    actionCollection->addAction(QStringLiteral("manage_event_roles"), manageEventRoles_);
    actionCollection->addAction(QStringLiteral("manage_event_types"), manageEventTypes_);

    openNewAction_ = KStandardAction::openNew(this, &MainWindow::newFile, actionCollection);
    openAction_ = KStandardAction::open(this, &MainWindow::openFile, actionCollection);
    quitAction_ = KStandardAction::quit(QCoreApplication::instance(), &QApplication::closeAllWindows, actionCollection);
    closeAction_ = KStandardAction::close(this, &MainWindow::closeFile, actionCollection);

    KStandardAction::preferences(this, &MainWindow::settingsConfigure, actionCollection);
    recentFilesAction_ = KStandardAction::openRecent(this, &MainWindow::openUrl, actionCollection);
    recentFilesAction_->loadEntries(config->group(QStringLiteral("RecentFiles")));

    setupGUI();
    showWelcomeScreen();
    syncActions();
}

void MainWindow::loadFile(const QString& filename, bool isNew) {
    qDebug() << "Loading file..." << filename << "new?" << isNew;
    // If there is an existing open file, close it.
    if (!currentFile.isEmpty()) {
        DataManager::reset();
        closeDatabase();
        currentFile.clear();
    }

    clearUi();

    // Save the file we will open and open it.
    currentFile = filename;
    recentFilesAction_->addUrl(QUrl::fromLocalFile(filename));
    openDatabase(filename, isNew);
    DataManager::initialize(this);

    auto* tabWidget = new QTabWidget;
    tabWidget->setTabsClosable(true);
    tabWidget->setMovable(true);
    tabWidget->setDocumentMode(true);
    connect(tabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::closeTab);

    auto* placeholderWidget = new TabWidgetPlaceholderWidget;
    placeholderWidget->setTabWidget(tabWidget);
    placeholderWidget->setPlaceholder(new PersonPlaceholderWidget);
    setCentralWidget(placeholderWidget);

    // Initialise the dock by default.
    auto* dockWidget = new QDockWidget(tr("People"), this);
    dockWidget->setObjectName("person_dock");
    dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    dockWidget->setFeatures(QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
    auto* tableView = new PersonListWidget(this);
    dockWidget->setWidget(tableView);
    addDockWidget(Qt::LeftDockWidgetArea, dockWidget);
    // Connect stuff.
    connect(tableView, &PersonListWidget::handlePersonSelected, this, &MainWindow::openOrSelectPerson);
    syncActions();
}

void MainWindow::clearUi() {
    // Remove the main widget.
    auto* oldCentralWidget = takeCentralWidget();
    delete oldCentralWidget;

    // Delete the person dock.
    for (auto* dockWidget: findChildren<QDockWidget*>()) {
        if (dockWidgetArea(dockWidget) != Qt::NoDockWidgetArea) {
            removeDockWidget(dockWidget);
            delete dockWidget;
        }
    }
}
void MainWindow::syncActions() {
    const QList manageActions{manageEventRoles_, manageEventTypes_, manageNameOrigins_};
    for (auto* manageAction: manageActions) {
        manageAction->setEnabled(!currentFile.isEmpty());
    }
    closeAction_->setEnabled(!currentFile.isEmpty());

    if (currentFile.isEmpty()) {
        setWindowTitle(QStringLiteral("opa"));
    } else {
        setWindowTitle(QStringLiteral("%1 - opa").arg(currentFile));
    }
}
QTabWidget* MainWindow::getTabWidget() const {
    return qobject_cast<TabWidgetPlaceholderWidget*>(centralWidget())->getTabWidget();
}

KRecentFilesAction* MainWindow::recentFilesAction() const {
    return recentFilesAction_;
}

void MainWindow::showWelcomeScreen() {
    clearUi();
    auto* welcomeScreen = new WelcomeView(this);
    setCentralWidget(welcomeScreen);
    syncActions();
}

void MainWindow::saveProperties(KConfigGroup& config) {
    config.writePathEntry("currentFile", currentFile);
}

void MainWindow::readProperties(const KConfigGroup& config) {
    if (config.hasKey("currentFile")) {
        loadFile(config.readPathEntry("currentFile", QStringLiteral("")));
    }
}

void MainWindow::newFile() {
    // TODO: support using in-memory databases here.
    qDebug() << "Open new file...";
    const auto fileNameFromDialog = QFileDialog::getSaveFileName(this, i18n("Save File As"));
    if (!fileNameFromDialog.isEmpty()) {
        loadFile(fileNameFromDialog, true);
    }
}

void MainWindow::openFile() {
    qDebug() << "Open file...";
    const auto fileNameFromDialog = QFileDialog::getOpenFileName(this, i18n("Open File"));
    if (!fileNameFromDialog.isEmpty()) {
        loadFile(fileNameFromDialog);
    }
}
void MainWindow::openUrl(const QUrl& url) {
    qDebug() << "Opening url" << url;
    loadFile(url.toLocalFile());
}

void MainWindow::closeFile() {
    DataManager::reset();
    closeDatabase();
    currentFile.clear();
    this->showWelcomeScreen();
}


void MainWindow::settingsConfigure() {
    if (KConfigDialog::showDialog(QStringLiteral("settings"))) {
        return;
    }

    auto* dialog = new KConfigDialog(this, QStringLiteral("settings"), opaSettings::self());
    auto* generalSettingsPage = new QWidget;
    m_settings = new Ui::Settings();
    m_settings->setupUi(generalSettingsPage);
    dialog->addPage(generalSettingsPage, i18nc("@title:tab", "General"), QStringLiteral("package_setting"));
    // TODO: handle this
    //    connect(dialog, &KConfigDialog::settingsChanged, m_opaView,
    //    &opaView::handleSettingsChanged);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}


void MainWindow::openOrSelectPerson(IntegerPrimaryKey personId) {
    // First, check if we already have a tab for the person in question.
    auto existingTab = this->findTabFor(personId);
    if (existingTab >= 0) {
        getTabWidget()->setCurrentIndex(existingTab);
        return;
    }

    auto* detailView = new PersonDetailView(personId, this);
    auto addedIndex = getTabWidget()->addTab(detailView, detailView->getDisplayName());

    // Go to the tab.
    getTabWidget()->setCurrentIndex(addedIndex);

    // Connect to the detail view to update the name of the person when needed.
    connect(detailView, &PersonDetailView::dataChanged, detailView, [this](IntegerPrimaryKey id) {
        const int tabIndex = this->findTabFor(id);
        if (tabIndex < 0) {
            return; // Do nothing as the tab no longer exists for some reason.
        }
        auto* theDetailView = qobject_cast<PersonDetailView*>(getTabWidget()->widget(tabIndex));
        getTabWidget()->setTabText(tabIndex, theDetailView->getDisplayName());
    });
}

void MainWindow::closeTab(int tabIndex) const {
    getTabWidget()->removeTab(tabIndex);
}

int MainWindow::findTabFor(IntegerPrimaryKey personId) const {
    for (int i = 0; i < getTabWidget()->count(); i++) {
        auto* tab = getTabWidget()->widget(i);
        // The tab widget should be a "PersonDetailView"
        if (auto* details = qobject_cast<PersonDetailView*>(tab)) {
            if (details->hasId(personId)) {
                return i;
            }
        }
    }
    return -1;
}

void MainWindow::openNameOriginManager() const {
    auto* window = new NameOriginsManagementWindow;
    window->show();
}

void MainWindow::openEventRolesManager() const {
    auto* window = new EventRolesManagementWindow;
    window->show();
}

void MainWindow::openEventTypesManager() const {
    auto* window = new EventTypesManagementWindow;
    window->show();
}

bool MainWindow::queryClose() {
    const KSharedConfigPtr config = KSharedConfig::openConfig();
    recentFilesAction_->saveEntries(config->group(QStringLiteral("RecentFiles")));
    if (!currentFile.isEmpty()) {
        auto general = config->group(QStringLiteral("General"));
        general.writeEntry("currentFile", currentFile);
    }
    QApplication::closeAllWindows();
    return KXmlGuiWindow::queryClose();
}
