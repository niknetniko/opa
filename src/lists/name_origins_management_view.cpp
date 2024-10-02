//
// Created by niko on 2/10/24.
//

#include <KLocalizedString>
#include <QToolBar>
#include <QSortFilterProxyModel>
#include <QTableView>
#include <QVBoxLayout>
#include "name_origins_management_view.h"
#include "data/data_manager.h"

NameOriginsManagementWindow::NameOriginsManagementWindow(QWidget *parent): QWidget(parent, Qt::Window) {
    this->setWindowTitle(i18n("Beheer naamoorsprongen"));

    // Create a toolbar.
    auto *nameToolbar = new QToolBar(this);

    auto addAction = new QAction(nameToolbar);
    addAction->setText(i18n("Nieuwe oorsprong toevoegen"));
    addAction->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
    nameToolbar->addAction(addAction);

    auto removeAction = new QAction(nameToolbar);
    removeAction->setText(i18n("Oorsprong verwijderen"));
    removeAction->setIcon(QIcon::fromTheme(QStringLiteral("edit-delete")));
    removeAction->setEnabled(false);
    nameToolbar->addAction(removeAction);

    auto* originsModel = DataManager::getInstance(this)->nameOriginsModel();
    // We want to filter and sort.
    auto *filterProxyModel = new QSortFilterProxyModel(this);
    filterProxyModel->setSourceModel(originsModel);

    auto* tableView = new QTableView(this);
    tableView->setModel(filterProxyModel);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(nameToolbar);
    layout->addWidget(tableView);

    this->setLayout(layout);
}
