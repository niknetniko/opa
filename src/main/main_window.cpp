/*
    SPDX-FileCopyrightText: 2022 Niko Strijbol <strijbol.niko@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
// KF headers
#include <KActionCollection>
#include <KConfigDialog>
#include <QDockWidget>
#include <KLocalizedString>

// application headers
#include "main_window.h"

#include "opadebug.h"
#include "lists/name_origins_management_view.h"
#include "people_overview_view.h"
#include "person_detail/person_detail_view.h"
#include "ui_settings.h"

MainWindow::MainWindow() {
    // Attach the tab view.
    auto *tabWidget = new QTabWidget();
    tabWidget->setTabsClosable(true);
    tabWidget->setMovable(true);
    tabWidget->addTab(new QLabel(tr("Niets"), this), tr("Test"));
    setCentralWidget(tabWidget);
    connect(tabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::closeTab);

    // Initialise the dock by default.
    auto *dockWidget = new QDockWidget(tr("Personen"), this);
    dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    dockWidget->setFeatures(QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
    auto tableView = new PeopleOverviewView(this);
    dockWidget->setWidget(tableView);
    addDockWidget(Qt::LeftDockWidgetArea, dockWidget);

    // Connect stuff.
    connect(tableView, &PeopleOverviewView::handlePersonSelected, this, &MainWindow::openOrSelectPerson);

    auto *manageNameOrigins = new QAction(this);
    manageNameOrigins->setText(i18n("Naamoorsprongen beheren"));
    connect(manageNameOrigins, &QAction::triggered, this, &MainWindow::openNameOriginManager);

    KActionCollection *actionCollection = KXMLGUIClient::actionCollection();
    actionCollection->addAction(QStringLiteral("manage_name_origins"), manageNameOrigins);
    KStandardAction::openNew(this, &MainWindow::fileNew, actionCollection);
    KStandardAction::quit(QCoreApplication::instance(), &QApplication::closeAllWindows, actionCollection);
    KStandardAction::preferences(this, &MainWindow::settingsConfigure, actionCollection);

    setupGUI();
}

MainWindow::~MainWindow() {
}

void MainWindow::fileNew() {
    qDebug() << "MainWindow::fileNew()";
    (new MainWindow)->show();
}

void MainWindow::settingsConfigure() {
    qDebug() << "MainWindow::settingsConfigure()";
    // The preference dialog is derived from prefs_base.ui
    //
    // compare the names of the widgets in the .ui file
    // to the names of the variables in the .kcfg file
    //avoid to have 2 dialogs shown
    if (KConfigDialog::showDialog(QStringLiteral("settings"))) {
        return;
    }

    auto *dialog = new KConfigDialog(this, QStringLiteral("settings"), opaSettings::self());
    auto *generalSettingsPage = new QWidget;
    m_settings = new Ui::Settings();
    m_settings->setupUi(generalSettingsPage);
    dialog->addPage(generalSettingsPage, i18nc("@title:tab", "General"), QStringLiteral("package_setting"));
    //    connect(dialog, &KConfigDialog::settingsChanged, m_opaView, &opaView::handleSettingsChanged);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}


void MainWindow::openOrSelectPerson(IntegerPrimaryKey personId) {
    qDebug() << "Selecting person... " << personId;

    // First, check if we already have a tab for the person in question.
    auto existingTab = this->findTabFor(personId);
    if (existingTab >= 0) {
        qDebug() << "Existing tab found... " << personId;
        qobject_cast<QTabWidget *>(centralWidget())->setCurrentIndex(existingTab);
        return;
    }

    qDebug() << "No existing tab found... " << personId;

    auto *detailView = new PersonDetailView(personId, this);
    auto addedIndex = qobject_cast<QTabWidget *>(centralWidget())->addTab(detailView, detailView->getDisplayName());

    // Go to the tab.
    qobject_cast<QTabWidget *>(centralWidget())->setCurrentIndex(addedIndex);

    // Connect to the detail view to update the name of the person when needed.
    connect(detailView, &PersonDetailView::dataChanged, detailView, [this](IntegerPrimaryKey id) {
        int tabIndex = this->findTabFor(id);
        if (tabIndex < 0) {
            return; // Do nothing as the tab no longer exists for some reason.
        }
        auto *theDetailView = qobject_cast<PersonDetailView *>(
            qobject_cast<QTabWidget *>(centralWidget())->widget(tabIndex));
        qobject_cast<QTabWidget *>(centralWidget())->setTabText(tabIndex, theDetailView->getDisplayName());
    });
}

void MainWindow::closeTab(int tabIndex) {
    qobject_cast<QTabWidget *>(centralWidget())->removeTab(tabIndex);
}

int MainWindow::findTabFor(IntegerPrimaryKey personId) {
    for (int i = 0; i < qobject_cast<QTabWidget *>(centralWidget())->count(); i++) {
        QWidget *tab = qobject_cast<QTabWidget *>(centralWidget())->widget(i);
        // The tab widget should be a "PersonDetailView"
        if (auto *details = dynamic_cast<PersonDetailView *>(tab)) {
            if (details->hasId(personId)) {
                return i;
            }
        }
    }
    return -1;
}

void MainWindow::openNameOriginManager() {
    auto *window = new NameOriginsManagementWindow(this);
    window->show();
}
