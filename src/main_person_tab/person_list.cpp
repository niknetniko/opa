#include <KConfigGroup>
#include <KLocalizedString>
#include <QDebug>
#include <QHeaderView>
#include <QLineEdit>
#include <QSortFilterProxyModel>
#include <QTreeView>
#include <QVBoxLayout>

#include "person_list.h"
#include "data/data_manager.h"
#include "data/person.h"
#include "utils/formatted_identifier_delegate.h"

PersonListWidget::PersonListWidget(QWidget *parent) : QWidget(parent) {
    auto *baseModel = DataManager::get().primaryNamesModel(this);

    // Create a searchable model.
    auto filtered = new QSortFilterProxyModel(this);
    filtered->setSourceModel(baseModel);
    filtered->setFilterKeyColumn(DisplayNameModel::NAME);
    filtered->setFilterCaseSensitivity(Qt::CaseInsensitive);

    auto *searchBox = new QLineEdit(this);
    searchBox->setPlaceholderText(i18n("Zoeken..."));
    searchBox->setClearButtonEnabled(true);

    // Allow searching...
    connect(searchBox, &QLineEdit::textEdited, filtered, &QSortFilterProxyModel::setFilterFixedString);

    tableView = new QTableView(this);
    tableView->setModel(filtered);
    tableView->setShowGrid(false);
    tableView->setSelectionBehavior(QTableView::SelectRows);
    tableView->setSelectionMode(QTableView::SelectionMode::SingleSelection);
    tableView->setSortingEnabled(true);
    tableView->verticalHeader()->hide();
    tableView->horizontalHeader()->resizeSections(QHeaderView::Stretch);
    tableView->horizontalHeader()->setSectionResizeMode(DisplayNameModel::ID, QHeaderView::ResizeToContents);
    tableView->horizontalHeader()->setSectionResizeMode(DisplayNameModel::NAME, QHeaderView::Stretch);
    tableView->horizontalHeader()->setSectionResizeMode(DisplayNameModel::ROOT, QHeaderView::ResizeToContents);
    tableView->horizontalHeader()->setHighlightSections(false);
    tableView->setItemDelegateForColumn(DisplayNameModel::ID,
                                        new FormattedIdentifierDelegate(
                                            tableView, FormattedIdentifierDelegate::PERSON));

    // Wrap in a VBOX for layout reasons.
    auto *layout = new QVBoxLayout(this);
    layout->addWidget(searchBox);
    layout->addWidget(tableView);

    connect(tableView->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this,
            &PersonListWidget::handleSelectedNewRow);
}

void PersonListWidget::handleSelectedNewRow(const QItemSelection &selected) {
    if (selected.empty()) {
        return;
    }

    // Get the ID of the person we want.
    auto personId = this->tableView->model()->index(selected.indexes().first().row(),
                                                    DisplayNameModel::ID).data().toLongLong();

    Q_EMIT handlePersonSelected(personId);
}
