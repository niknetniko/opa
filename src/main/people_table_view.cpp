//
// Created by niko on 2/09/2022.
//


#include <KSharedConfig>
#include <KConfigGroup>
#include <QDebug>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QLineEdit>

#include "people_table_view.h"
#include "people_table_model.h"

PeopleTableView::PeopleTableView(QWidget *parent): QWidget(parent) {

    auto *model = new PeopleTableModel(this);

    tableView = new QTableView(this);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->verticalHeader()->setVisible(false);
    tableView->sortByColumn(1, Qt::SortOrder::AscendingOrder);
    tableView->horizontalHeader()->resizeSections(QHeaderView::Stretch);
    tableView->setSortingEnabled(true);
    // We are done setting up, attach the model.
    tableView->setModel(model);
    qDebug() << model->columnCount();
    tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    tableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

    auto* searchBox = new QLineEdit(this);
    searchBox->setPlaceholderText(QString("Zoeken..."));

    connect(searchBox, &QLineEdit::textEdited, model, &PeopleTableModel::onSearchChanged);

    // Wrap in a VBOX for layout reasons.
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(searchBox);
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

void PeopleTableView::handleSelectedPersonChanged(unsigned long long personId) {
    int rowNumber = findRowIndex(personId);
    auto index = tableView->model()->index(rowNumber, 0);
    tableView->scrollTo(index);
    tableView->selectRow(rowNumber);
}

void PeopleTableView::handleSelectedNewRow(const QItemSelection &selected, const QItemSelection &_deselected) {
    // Hack
    if (selected.empty()) {
        return;
    }
    auto personId = selected.indexes().front().data(Qt::UserRole).toULongLong();

//    auto config = KSharedConfig::openStateConfig();
//    KConfigGroup selectionGroup(config, "selection");
//    selectionGroup.writeEntry("current", personId);
    emit handlePersonSelected(personId);
}

int PeopleTableView::findRowIndex(unsigned long long personId) {
    for (int i = 0; i < tableView->model()->rowCount(); ++i) {
        auto rowIndex = tableView->model()->index(i, 0);
        auto data = tableView->model()->data(rowIndex, Qt::UserRole);
        if (data.toULongLong() == personId) {
            return i;
        }
    }

    return -1;
}
