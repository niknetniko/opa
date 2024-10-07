//
// Created by niko on 8/02/24.
//

#include <QSqlQuery>
#include <QDebug>
#include <QSqlError>
#include <QSqlTableModel>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QMessageBox>

#include "names_overview_view.h"
#include "database/schema.h"
#include "names/name_editor.h"
#include "utils/single_row_model.h"
#include "data/names.h"
#include "data/data_manager.h"
#include "utils/formatted_identifier_delegate.h"
#include "utils/model_utils.h"

QString Names::construct_display_name(const QString &titles, const QString &givenNames, const QString &prefix,
                                      const QString &surname) {
    QStringList nameParts;
    if (!titles.isEmpty()) {
        nameParts.append(titles);
    }
    if (!givenNames.isEmpty()) {
        nameParts.append(givenNames);
    }
    if (!prefix.isEmpty()) {
        nameParts.append(prefix);
    }
    if (!surname.isEmpty()) {
        nameParts.append(surname);
    }
    return nameParts.join(QStringLiteral(" "));
}

NamesOverviewView::NamesOverviewView(IntegerPrimaryKey personId, QWidget *parent) : QWidget(parent) {
    this->personId = personId;
    this->baseModel = DataManager::getInstance(this)->namesModelForPerson(this, personId);

    // We only show certain columns here.
    auto *selectedColumnsModel = new KRearrangeColumnsProxyModel(this);
    selectedColumnsModel->setSourceModel(baseModel);
    selectedColumnsModel->setSourceColumns({
                                                   NamesTableModel::ID,
                                                   NamesTableModel::SORT,
                                                   NamesTableModel::TITLES,
                                                   NamesTableModel::GIVEN_NAMES,
                                                   NamesTableModel::PREFIX,
                                                   NamesTableModel::SURNAME,
                                                   NamesTableModel::ORIGIN
                                           });

    // We want to filter and sort.
    auto *filterProxyModel = new QSortFilterProxyModel(this);
    filterProxyModel->setSourceModel(selectedColumnsModel);

    // TODO: fix this and make it proper.
    // while (model->canFetchMore()) { model->fetchMore(); }

    treeView = new QTreeView(this);
    treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    treeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    treeView->setUniformRowHeights(true);
    treeView->setRootIsDecorated(false);
    treeView->setSortingEnabled(true);
    // Sort on default column.
    treeView->sortByColumn(1, Qt::AscendingOrder);
    // We are done setting up, attach the model.
    treeView->setModel(filterProxyModel);
    treeView->setItemDelegateForColumn(NamesTableModel::ID, new FormattedIdentifierDelegate(FormattedIdentifierDelegate::NAME));
    treeView->header()->setSortIndicatorClearable(false);

    // Wrap in a VBOX for layout reasons.
    auto *layout = new QVBoxLayout(this);
    layout->addWidget(treeView);

    connect(treeView->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this,
            &NamesOverviewView::handleSelectedNewRow);
    // Support models being reset.
    connect(treeView->model(), &QAbstractItemModel::modelReset, this, [this]() {
        this->handleSelectedNewRow(QItemSelection(), QItemSelection());
    });

    connect(treeView, &QTableView::doubleClicked, this, &NamesOverviewView::handleDoubleClick);
    connect(treeView->header(), &QHeaderView::sortIndicatorChanged, this, [this](int logicalIndex) {
        auto* model = this->treeView->selectionModel();
        Q_EMIT this->sortChanged(model->model(), model->selection(), logicalIndex);
    });
}

void NamesOverviewView::handleSelectedNewRow(const QItemSelection &selected, const QItemSelection & /*deselected*/) {
    Q_EMIT this->selectedName(this->treeView->selectionModel()->model(), selected);
}

void NamesOverviewView::handleNewName() {
    auto *nameModel = const_cast<QSqlTableModel*>(find_source_model_of_type<QSqlTableModel>(this->baseModel));

    auto newRecord = nameModel->record();
    newRecord.setGenerated(NamesTableModel::ID, false);
    newRecord.setValue(NamesTableModel::PERSON_ID, this->personId);
    newRecord.setValue(NamesTableModel::SORT, treeView->model()->rowCount() + 1);
    if (!nameModel->insertRecord(-1, newRecord)) {
        QMessageBox::warning(this, tr("Could not insert name"), tr("Problem inserting new name into database."));
        qDebug() << "Could not get last inserted ID for some reason:";
        qDebug() << nameModel->lastError();
        return;
    }

    auto lastInsertedId = nameModel->query().lastInsertId();
    if (!lastInsertedId.isValid()) {
        QMessageBox::warning(this, tr("Could not insert name"), tr("Problem inserting new name into database."));
        qDebug() << "Could not get last inserted ID for some reason:";
        qDebug() << nameModel->lastError();
        return;
    }

    auto theId = lastInsertedId.toLongLong();
    auto *theModel = DataManager::getInstance(this)->singleNameModel(this, theId);

    // Show the dialog for the other data.
    auto *editorWindow = new NamesEditor(theModel, true, this);
    editorWindow->show();
    editorWindow->adjustSize();
}

void NamesOverviewView::editSelectedName() {
    // Get the currently selected name.
    auto selection = this->treeView->selectionModel();
    if (!selection->hasSelection()) {
        return;
    }

    auto selectRow = selection->selectedRows().first();

    // The first column contains the primary key.
    // TODO: do not hardcode this.
    auto index = this->treeView->model()->index(selectRow.row(), 0);
    auto theId = this->treeView->model()->data(index, Qt::EditRole).toLongLong();
    auto *theModel = DataManager::getInstance(this)->singleNameModel(this, theId);
    auto *editorWindow = new NamesEditor(theModel, false, this);
    editorWindow->show();
    editorWindow->adjustSize();
}

void NamesOverviewView::removeSelectedName() {
    // Get the currently selected name.
    auto selection = this->treeView->selectionModel();
    if (!selection->hasSelection()) {
        return;
    }

    auto selectRow = selection->selectedRows().first();
    this->treeView->model()->removeRow(selectRow.row());
}

void NamesOverviewView::handleDoubleClick(const QModelIndex &clicked) {
    this->editSelectedName();
}

void NamesOverviewView::moveSelectedNameUp() {
    auto selection = this->treeView->selectionModel();
    if (!selection->hasSelection()) {
        return;
    }

    auto selectRow = selection->selectedRows().first().row();
    auto newRow = selectRow - 1;
    this->moveSelectedNameToPosition(selectRow, newRow);
}

void NamesOverviewView::moveSelectedNameToPosition(int from, int to) {
    // Create an ordered list of the names except the current one.
    // We can assume the model is ordered, otherwise we would have not allowed modifying it.
    auto* model = this->treeView->model();

    qDebug() << "There are " << model->rowCount() << " rows in the model...";

    QVector<int> vector(model->rowCount());
    std::iota( std::begin(vector), std::end(vector), 1 );
    std::swap(vector[from], vector[to]);

    // We want to update this in one go; so get the root model.
    auto* rootModel = const_cast<QSqlTableModel*>(find_source_model_of_type<QSqlTableModel>(model));
    auto original = rootModel->editStrategy();
    rootModel->setEditStrategy(QSqlTableModel::OnManualSubmit);

    // Update all values.
    for (int row = 0; row < vector.length(); ++row) {
        model->setData(model->index(row, 1), vector[row]);
    }

    connect(rootModel, &QSqlTableModel::dataChanged, this, []() {
        qDebug() << "Data has been changed in the root model!";
    });
    connect(rootModel, &QSqlTableModel::modelReset, this, []() {
        qDebug() << "Reset in the the root model!";
    });

    // Commit the data and done.
    if (!rootModel->submitAll()) {
        qDebug() << "Could not submit for some reason";
        qDebug() << rootModel->lastError();
    } else {
        qDebug() << "Have submitted";
    }

    rootModel->setEditStrategy(original);

    // Fix the selection by choosing the new row.
    treeView->selectionModel()->select(model->index(to, 0), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
}

void NamesOverviewView::moveSelectedNameDown() {
    auto selection = this->treeView->selectionModel();
    if (!selection->hasSelection()) {
        return;
    }

    auto selectRow = selection->selectedRows().first().row();
    auto newRow = selectRow + 1;
    this->moveSelectedNameToPosition(selectRow, newRow);
}
