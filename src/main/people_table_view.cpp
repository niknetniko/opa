//
// Created by niko on 2/09/2022.
//


#include <KSharedConfig>
#include <KConfigGroup>
#include <QDebug>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QLineEdit>
#include <KLocalizedString>

#include "people_table_view.h"
#include "people_table_model.h"
#include "data/data_manager.h"
#include "data/person.h"
#include "utils/formatted_identifier_delegate.h"

PeopleTableView::PeopleTableView(QWidget *parent): QWidget(parent) {

    auto *baseModel = DataManager::getInstance(this)->primaryNamesModel(this);

    qDebug() << "Base model has count: " << baseModel->rowCount();

    // Create a searchable model.
    auto filtered = new QSortFilterProxyModel(this);
    filtered->setSourceModel(baseModel);
    filtered->setFilterKeyColumn(DisplayNameModel::NAME);
    filtered->setFilterCaseSensitivity(Qt::CaseInsensitive);

    auto* searchBox = new QLineEdit(this);
    searchBox->setPlaceholderText(i18n("Zoeken..."));

    // Allow searching...
    connect(searchBox, &QLineEdit::textEdited, filtered, &QSortFilterProxyModel::setFilterFixedString);

    tableView = new QTableView(this);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->verticalHeader()->setVisible(false);
    tableView->sortByColumn(DisplayNameModel::NAME, Qt::SortOrder::AscendingOrder);
    tableView->horizontalHeader()->resizeSections(QHeaderView::Stretch);
    tableView->setSortingEnabled(true);
    // We are done setting up, attach the model.
    tableView->setModel(filtered);
    tableView->horizontalHeader()->setSectionResizeMode(DisplayNameModel::ID, QHeaderView::ResizeToContents);
    tableView->horizontalHeader()->setSectionResizeMode(DisplayNameModel::NAME, QHeaderView::Stretch);
    tableView->horizontalHeader()->setSectionResizeMode(DisplayNameModel::ROOT, QHeaderView::ResizeToContents);
    tableView->setItemDelegateForColumn(DisplayNameModel::ID, new FormattedIdentifierDelegate(FormattedIdentifierDelegate::PERSON));

    // Wrap in a VBOX for layout reasons.
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(searchBox);
    layout->addWidget(tableView);

//    auto config = KSharedConfig::openStateConfig();
//    KConfigGroup selectionGroup(config, i18n("selection"));
//    if (selectionGroup.hasKey("current")) {
//        auto row = selectionGroup.readEntry("current").toInt();
//        handleSelectedPersonChanged(row);
//    }

    connect(tableView->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this,
            &PeopleTableView::handleSelectedNewRow);
}

void PeopleTableView::handleSelectedPersonChanged(IntegerPrimaryKey personId) {
    int rowNumber = findRowIndex(personId);
    auto index = tableView->model()->index(rowNumber, 0);
    tableView->scrollTo(index);
    tableView->selectRow(rowNumber);
}

void PeopleTableView::handleSelectedNewRow(const QItemSelection &selected) {
    if (selected.empty()) {
        return;
    }

    // Get the ID of the person we want.
    auto personId = this->tableView->model()->index(selected.indexes().first().row(), DisplayNameModel::ID).data().toLongLong();

//    auto config = KSharedConfig::openStateConfig();
//    KConfigGroup selectionGroup(config, "selection");
//    selectionGroup.writeEntry("current", personId);
    Q_EMIT handlePersonSelected(personId);
}

int PeopleTableView::findRowIndex(IntegerPrimaryKey personId) {
    for (int i = 0; i < tableView->model()->rowCount(); ++i) {
        auto rowIndex = tableView->model()->index(i, DisplayNameModel::ID);
        auto data = tableView->model()->data(rowIndex);
        if (data.toLongLong() == personId) {
            return i;
        }
    }

    return -1;
}
