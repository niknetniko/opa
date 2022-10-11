//
// Created by niko on 2/09/2022.
//


#include <KConfigGroup>
#include <QDebug>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QLineEdit>

#include "event_table_view.h"
#include "event_table_model.h"

EventTableView::EventTableView(int personId, QWidget *parent): QWidget(parent) {

    auto *model = new EventTableModel(personId, this);
    // TODO: fix this and make it proper.
    while (model->canFetchMore()) { model->fetchMore(); }

    tableView = new QTableView(this);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->verticalHeader()->setVisible(false);
    tableView->sortByColumn(1, Qt::SortOrder::AscendingOrder);
    tableView->horizontalHeader()->resizeSections(QHeaderView::Stretch);
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    tableView->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    tableView->setSortingEnabled(true);
    // We are done setting up, attach the model.
    tableView->setModel(model);

    // Wrap in a VBOX for layout reasons.
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(tableView);

    connect(tableView->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this,
            &EventTableView::handleSelectedNewRow);
}

void EventTableView::handleSelectedNewRow(const QItemSelection &selected, const QItemSelection &_deselected) {
    // Hack
    if (selected.empty()) {
        return;
    }
    // TODO: open something?
}
