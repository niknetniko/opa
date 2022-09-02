//
// Created by niko on 2/09/2022.
//


#include <KSharedConfig>
#include <KConfigGroup>
#include <QDebug>
#include <QVBoxLayout>
#include <QHeaderView>

#include "people_table_view.h"
#include "people_list_model.h"

PeopleTableView::PeopleTableView(QWidget *parent): QWidget(parent) {

    // TODO: fix this and make it proper.
    auto *model = new PeopleListModel(this);
    while (model->canFetchMore()) { model->fetchMore(); }

    this->tableView = new QTableView(this);
    tableView->setModel(model);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->verticalHeader()->setVisible(false);

    // Wrap in a VBOX for layout reasons.
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(tableView);

    auto config = KSharedConfig::openStateConfig();
    KConfigGroup selectionGroup(config, "selection");
    if (selectionGroup.hasKey("current")) {
        auto row = selectionGroup.readEntry("current").toInt();
        handleSelectedPersonChanged(row);
    }

    connect(tableView->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this,
            &PeopleTableView::handleSelectedNewRow);
}

void PeopleTableView::handleSelectedPersonChanged(int personId) {
    // TODO: properly do all this stuff somehow.
    auto index = tableView->model()->index(personId - 1, 0);
    tableView->scrollTo(index);
    tableView->selectRow(personId - 1);
}

void PeopleTableView::handleSelectedNewRow(const QItemSelection &selected, const QItemSelection &_deselected) {
    // Hack
    if (selected.empty()) {
        return;
    }
    qDebug() << "Indices are is now " << selected;
    auto personId = selected.indexes().front().data(Qt::EditRole).toULongLong();
    qDebug() << "Person is now " << personId;

    emit handlePersonSelected(personId);
//    auto config = KSharedConfig::openStateConfig();
//    KConfigGroup selectionGroup(config, "selection");
//    selectionGroup.writeEntry("current", personId);
}


