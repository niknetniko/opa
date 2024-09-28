/*
    SPDX-FileCopyrightText: 2022 Niko Strijbol <strijbol.niko@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

// application headers
#include "main_window.h"

#include "opadebug.h"
#include "people_overview_view.h"
#include "person_detail/person_detail_view.h"

// KF headers
#include <KActionCollection>
#include <KConfigDialog>
#include <QDockWidget>


MainWindow::MainWindow() : KXmlGuiWindow() {
    // Attach the tab view.
    this->tabWidget = new QTabWidget();
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

    KActionCollection *actionCollection = this->actionCollection();
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
    m_settings.setupUi(generalSettingsPage);
    dialog->addPage(generalSettingsPage, i18nc("@title:tab", "General"), QStringLiteral("package_setting"));
//    connect(dialog, &KConfigDialog::settingsChanged, m_opaView, &opaView::handleSettingsChanged);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}


void MainWindow::openOrSelectPerson(IntegerPrimaryKey personId) {

    qDebug() << "Selecting person... " << personId;

    // First, check if we already have a tab for the person in question.
    for (int i = 0; i < tabWidget->count(); i++) {
        QWidget* tab = tabWidget->widget(i);
        // The tab widget should be a "PersonDetailView"
        if (auto* details = dynamic_cast<PersonDetailView*>(tab)) {
            if (details->id == personId) {
                tabWidget->setCurrentIndex(i);
                return;
            }
        }
    }

    qDebug() << "No existing tab found... " << personId;

    auto* detailView = new PersonDetailView(personId, this);
    connect(detailView, &PersonDetailView::personNameChanged, this, &MainWindow::updatePersonName);
    tabWidget->setCurrentIndex(tabWidget->addTab(detailView, tr("Laden...")));

    qDebug() << "Populating...";
    detailView->populate();
}

void MainWindow::updatePersonName(IntegerPrimaryKey personId, const QString& newName) {
    for (int i = 0; i < tabWidget->count(); i++) {
        QWidget* tab = tabWidget->widget(i);
        // The tab widget should be a "PersonDetailView"
        if (auto* details = dynamic_cast<PersonDetailView*>(tab)) {
            if (details->id == personId) {
                tabWidget->setTabText(i, newName);
                return;
            }
        }
    }
}

void MainWindow::closeTab(int tabIndex) {
    this->tabWidget->removeTab(tabIndex);
}
