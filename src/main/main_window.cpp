/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "main_window.h"

#include "data/data_manager.h"
#include "data/event.h"
#include "data/names.h"
#include "database/database.h"
#include "docks/person_list_dock.h"
#include "editors/new_person_editor_dialog.h"
#include "lists/event_roles_management_window.h"
#include "lists/event_types_management_window.h"
#include "lists/name_origins_management_window.h"
#include "person_detail/person_detail_view.h"
#include "person_placeholder_widget.h"
#include "ui_settings.h"
#include "utils/placeholder_widget.h"
#include "welcome/welcome_view.h"
#include <kddockwidgets/Config.h>
#include <kddockwidgets/DockWidget.h>
#include <kddockwidgets/MainWindow.h>

#include <KActionCollection>
#include <KConfigDialog>
#include <KLocalizedString>
#include <KMessageBox>
#include <QFileDialog>
#include <QSqlError>

PersonDock::PersonDock(IntegerPrimaryKey personId) :
    DockWidget(QStringLiteral("Person %1").arg(personId)),
    personId(personId) {
    auto* detailWidget = new PersonDetailView(personId, this);
    setWidget(detailWidget);
    setTitle(detailWidget->getDisplayName());
    connect(detailWidget, &PersonDetailView::nameChanged, this, [this, detailWidget] {
        setTitle(detailWidget->getDisplayName());
    });
}

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
    Q_ASSERT(mainWindow != nullptr);
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

    addNewPersonAction_ = new QAction(this);
    addNewPersonAction_->setText(i18n("Add new person"));
    addNewPersonAction_->setIcon(QIcon::fromTheme(QStringLiteral("list-add-user")));
    connect(addNewPersonAction_, &QAction::triggered, this, &MainWindow::onOpenNewPersonEditor);

    removePersonAction_ = new QAction(this);
    removePersonAction_->setText(i18n("Delete person"));
    removePersonAction_->setIcon(QIcon::fromTheme(QStringLiteral("list-remove-user")));
    connect(removePersonAction_, &QAction::triggered, this, &MainWindow::onDeleteCurrentPerson);

    auto* actionCollection = KXMLGUIClient::actionCollection();
    actionCollection->addAction(QStringLiteral("manage_name_origins"), manageNameOrigins_);
    actionCollection->addAction(QStringLiteral("manage_event_roles"), manageEventRoles_);
    actionCollection->addAction(QStringLiteral("manage_event_types"), manageEventTypes_);
    actionCollection->addAction(QStringLiteral("person_add_new"), addNewPersonAction_);
    actionCollection->addAction(QStringLiteral("person_delete_existing"), removePersonAction_);

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

    connect(this, &MainWindow::onPersonOpenedOrClosed, this, &MainWindow::syncRemoveAction);
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

    // Configure the docks with the proper options.
    KDDockWidgets::Config::self().setFlags(
        KDDockWidgets::Config::Flag_NativeTitleBar | KDDockWidgets::Config::Flag_HideTitleBarWhenTabsVisible |
        KDDockWidgets::Config::Flag_AllowReorderTabs | KDDockWidgets::Config::Flag_TabsHaveCloseButton |
        KDDockWidgets::Config::Flag_CloseOnlyCurrentTab | KDDockWidgets::Config::Flag_AlwaysShowTabs |
        KDDockWidgets::Config::Flag_ShowButtonsOnTabBarIfTitleBarHidden
    );

    auto* dockContainer = new KDDockWidgets::QtWidgets::MainWindow(QStringLiteral("MyMainWindow"), {}, this);
    auto* placeholderWidget = new KDDockPlaceholderWidget;
    placeholderWidget->setDockContainer(dockContainer);
    placeholderWidget->setPlaceholder(new PersonPlaceholderWidget);
    setCentralWidget(placeholderWidget);

    auto* personListDock = new PersonListDock();
    dockContainer->addDockWidget(personListDock, KDDockWidgets::Location_OnRight);
    connect(personListDock, &PersonListDock::handlePersonSelected, this, &MainWindow::openOrSelectPerson);
    syncActions();
}

void MainWindow::clearUi() {
    // Remove the main widget.
    auto* oldCentralWidget = takeCentralWidget();
    if (oldCentralWidget != nullptr) {
        oldCentralWidget->deleteLater();
    }
}

void MainWindow::syncActions() {
    const QList manageActions{manageEventRoles_, manageEventTypes_, manageNameOrigins_, addNewPersonAction_};
    for (auto* manageAction: manageActions) {
        manageAction->setEnabled(!currentFile.isEmpty());
    }
    closeAction_->setEnabled(!currentFile.isEmpty());

    if (currentFile.isEmpty()) {
        setWindowTitle(QStringLiteral("opa"));
    } else {
        setWindowTitle(QStringLiteral("%1 - opa").arg(currentFile));
    }
    syncRemoveAction();
}

KDDockWidgets::QtWidgets::MainWindow* MainWindow::getMainDockHost() const {
    auto* placeholder = qobject_cast<KDDockPlaceholderWidget*>(centralWidget());
    Q_ASSERT(placeholder != nullptr);
    return placeholder->getDockContainer();
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
        loadFile(config.readPathEntry("currentFile", QString()));
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
    auto* existingDock = findDockFor(personId);
    if (existingDock != nullptr) {
        if (existingDock->isFloating()) {
            existingDock->raise();
            existingDock->activateWindow();
        }
        existingDock->setAsCurrentTab();
        return;
    }

    auto* dock = new PersonDock(personId);
    auto* container = getMainDockHost();

    // Attempt to find a non-floating dock to add new person tab next to.
    PersonDock* nonFloating = nullptr;
    auto theChildren = findChildren<PersonDock*>();
    for (auto* personDock: std::as_const(theChildren)) {
        if (!personDock->isFloating()) {
            nonFloating = personDock;
            break;
        }
    }
    if (nonFloating == nullptr) {
        container->addDockWidget(dock, KDDockWidgets::Location_OnRight);
    } else {
        nonFloating->addDockWidgetAsTab(dock);
    }

    Q_EMIT onPersonOpenedOrClosed();
}

PersonDock* MainWindow::findDockFor(IntegerPrimaryKey personId) const {
    auto dockWidgets = findChildren<PersonDock*>();
    for (auto* personDock: std::as_const(dockWidgets)) {
        if (personDock->personId == personId) {
            return personDock;
        }
    }
    return nullptr;
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

void MainWindow::syncRemoveAction() const {
    // Check if we have an open person, or else disable the action.
    removePersonAction_->setEnabled(!currentFile.isEmpty() && findChildren<PersonDock*>().count() > 0);
}

void MainWindow::onOpenNewPersonEditor() {
    auto* dialog = new NewPersonEditorDialog(this);
    dialog->show();
}

void MainWindow::onDeleteCurrentPerson() {
    // TODO
    // void* currentTab = nullptr;
    // if (currentTab == nullptr) {
    //     qWarning() << "No person is currently open, so doing nothing";
    //     return;
    // }
    //
    // auto personId = currentTab->getId();
    //
    // KMessageBox::ButtonCode result = KMessageBox::warningContinueCancel(
    //     this,
    //     i18n("You are moments away from deleting person %1. This cannot be undone.").arg(personId),
    //     i18n("Confirm deletion"),
    //     KStandardGuiItem::del()
    // );
    //
    // if (result != KMessageBox::Continue) {
    //     return;
    // }
    //
    // closeTab(getTabWidget()->currentIndex());
    //
    // // First, delete all events where the person is linked.
    // auto* eventRelationModel = DataManager::get().eventRelationsModel();
    // for (int row = eventRelationModel->rowCount() - 1; row >= 0; --row) {
    //     if (eventRelationModel->index(row, EventRelationsModel::PERSON_ID).data() == personId) {
    //         if (!eventRelationModel->removeRow(row)) {
    //             qWarning() << "Could not remove event relation at row" << row;
    //             qDebug() << "Aborting deleting person";
    //             return;
    //         }
    //     }
    // }
    // if (!eventRelationModel->submit()) {
    //     qWarning() << "Could not save event relation changes.";
    //     qDebug() << eventRelationModel->lastError();
    //     return;
    // }
    //
    // // Delete all names associated with the person.
    // auto* namesModel = DataManager::get().namesModel();
    // for (int row = namesModel->rowCount() - 1; row >= 0; --row) {
    //     if (namesModel->index(row, NamesTableModel::PERSON_ID).data() == personId) {
    //         if (!namesModel->removeRow(row)) {
    //             qWarning() << "Could not remove name at row" << row;
    //             qDebug() << "Aborting deleting person";
    //             return;
    //         }
    //     }
    // }
    // if (!namesModel->submit()) {
    //     qWarning() << "Could not save name changes.";
    //     qDebug() << namesModel->lastError();
    //     return;
    // }
    //
    // // Delete the person itself.
    // auto* singlePersonModel = DataManager::get().singlePersonModel(this, personId);
    // if (!singlePersonModel->removeRow(0)) {
    //     qWarning() << "Could not remove person";
    //     qDebug() << "Aborting deleting person";
    //     return;
    // }
    // auto* personModel = DataManager::get().peopleModel();
    // if (!personModel->submit()) {
    //     qWarning() << "Could not save person changes.";
    //     qDebug() << personModel->lastError();
    // }
}
