//
// Created by niko on 2/10/24.
//

#include <KLocalizedString>
#include <QToolBar>
#include <QSortFilterProxyModel>
#include <QTableView>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QSqlRecord>
#include "name_origins_management_view.h"
#include "data/data_manager.h"
#include "data/names.h"

NameOriginsManagementWindow::NameOriginsManagementWindow(QWidget *parent): QWidget(parent, Qt::Window) {
    this->setWindowTitle(i18n("Beheer naamoorsprongen"));

    // Create a toolbar.
    auto *nameToolbar = new QToolBar(this);
    nameToolbar->setOrientation(Qt::Vertical);
    nameToolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    auto addAction = new QAction(nameToolbar);
    addAction->setText(i18n("Toevoegen"));
    addAction->setToolTip(i18n("Voeg een nieuwe naamoorsprong toe"));
    addAction->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
    nameToolbar->addAction(addAction);
    connect(addAction, &QAction::triggered, this, &NameOriginsManagementWindow::addOrigin);

    this->removeAction = new QAction(nameToolbar);
    removeAction->setText(i18n("Verwijderen"));
    removeAction->setToolTip(i18n("Verwijder deze naamoorsprong"));
    removeAction->setIcon(QIcon::fromTheme(QStringLiteral("edit-delete")));
    removeAction->setEnabled(false);
    nameToolbar->addAction(removeAction);

    auto repairAction = new QAction(nameToolbar);
    repairAction->setText(i18n("Opschonen"));
    repairAction->setToolTip(i18n("Verwijder lege en dubbele oorsprongen"));
    repairAction->setIcon(QIcon::fromTheme(QStringLiteral("tools-wizard")));
    repairAction->setEnabled(true);
    nameToolbar->addAction(repairAction);

    this->model = DataManager::getInstance(this)->nameOriginsModel();
    // We want to filter and sort.
    auto *filterProxyModel = new QSortFilterProxyModel(this);
    filterProxyModel->setSourceModel(model);

    auto* tableView = new QTableView(this);
    tableView->setModel(filterProxyModel);
    tableView->setSortingEnabled(true);
    tableView->sortByColumn(NameOriginTableModel::ID, Qt::AscendingOrder);
    tableView->setShowGrid(false);
    tableView->setSelectionMode(QTableView::SelectionMode::SingleSelection);
    tableView->verticalHeader()->hide();
    tableView->horizontalHeader()->resizeSections(QHeaderView::Stretch);
    tableView->horizontalHeader()->setSectionResizeMode(NameOriginTableModel::ID, QHeaderView::ResizeToContents);
    tableView->horizontalHeader()->setSectionResizeMode(NameOriginTableModel::ORIGIN, QHeaderView::Stretch);
    tableView->horizontalHeader()->setHighlightSections(false);

    connect(tableView->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this,
            &NameOriginsManagementWindow::onSelectionChanged);
    // Support models being reset.
    connect(tableView->model(), &QAbstractItemModel::modelReset, this, [this]() {
        this->onSelectionChanged(QItemSelection(), QItemSelection());
    });

    auto *layout = new QHBoxLayout(this);
    layout->setSpacing(0);
    layout->addWidget(tableView);
    layout->addWidget(nameToolbar);
}

void NameOriginsManagementWindow::addOrigin() {
    auto newRecord = this->model->record();
    newRecord.setGenerated(NameOriginTableModel::ID, false);
    this->model->insertRecord(-1, newRecord);
}

void NameOriginsManagementWindow::onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected) {
    if (selected.isEmpty()) {
        this->removeAction->setEnabled(false);
        return;
    }

    // TODO: change this to be able to use the ID instead of the actual name.
    // Get the ID of the current selected row.
    auto id = this->model->index(selected.indexes().first().row(), NameOriginTableModel::ORIGIN).data();

    qDebug() << "Checking name with id " << id << " for usage";

    // TODO: this is not very efficient
    auto* nameModel = DataManager::getInstance(this)->namesModel();
    bool isUsedByNames = false;
    for(int r = 0; r < nameModel->rowCount(); ++r) {
        auto usedOrigin = nameModel->index(r, NamesTableModel::ORIGIN_ID).data();
        qDebug() << "Name at row " << r << " uses origin " << usedOrigin;
        if (usedOrigin == id) {
            isUsedByNames = true;
            break;
        }
    }

    this->removeAction->setEnabled(!isUsedByNames);
}

void NameOriginsManagementWindow::removeOrigin() {


}
