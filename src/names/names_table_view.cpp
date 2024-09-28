//
// Created by niko on 8/02/24.
//

#include <QSqlQuery>
#include <QDebug>
#include <QSqlError>
#include <stdexcept>
#include <QMetaEnum>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QMessageBox>

#include "names_table_view.h"
#include "database/schema.h"
#include "names/name_editor.h"
#include "utils/single_row_model.h"
#include "data/names.h"
#include "data/data_manager.h"
#include "utils/formatted_identifier_delegate.h"

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

NamesTableView::NamesTableView(IntegerPrimaryKey personId, QWidget *parent) : QWidget(parent) {
    this->personId = personId;
    qDebug() << "Initializing NamesTableView for person with ID " << personId;
    this->baseModel = DataManager::getInstance(this)->namesModelForPerson(this, personId);

    // We only show certain columns here.
    auto *selectedColumnsModel = new KRearrangeColumnsProxyModel(this);
    selectedColumnsModel->setSourceModel(baseModel);
    selectedColumnsModel->setSourceColumns({
                                                   NamesTableModel::ID,
                                                   NamesTableModel::MAIN,
                                                   NamesTableModel::TITLES,
                                                   NamesTableModel::GIVEN_NAMES,
                                                   NamesTableModel::PREFIX,
                                                   NamesTableModel::SURNAME,
                                                   NamesTableModel::ORIGIN_ID
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
    // We are done setting up, attach the model.
    treeView->setModel(filterProxyModel);
    treeView->setItemDelegateForColumn(NamesTableModel::ID, new FormattedIdentifierDelegate(FormattedIdentifierDelegate::NAME));

    // Wrap in a VBOX for layout reasons.
    auto *layout = new QVBoxLayout(this);
    layout->addWidget(treeView);

    connect(treeView->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this,
            &NamesTableView::handleSelectedNewRow);

    connect(treeView, &QTableView::doubleClicked, this, &NamesTableView::handleDoubleClick);
}

void NamesTableView::handleSelectedNewRow(const QItemSelection &selected, const QItemSelection & /*deselected*/) {
    Q_EMIT this->selectedName(this->treeView->selectionModel()->model(), selected);
}

void NamesTableView::handleNewName() {
    auto *nameModel = DataManager::getInstance(this)->namesModel();
    auto newRecord = nameModel->record();
    newRecord.setGenerated(NamesTableModel::ID, true);
    newRecord.setValue(NamesTableModel::PERSON_ID, this->personId);
    newRecord.setValue(NamesTableModel::MAIN, false);
    if (!nameModel->insertRecord(-1, newRecord)) {
        QMessageBox::warning(this, tr("Could not insert name"), tr("Problem inserting new name into database."));
        qWarning() << "Error was: " << nameModel->lastError();
        qDebug() << "Query was: " << nameModel->query().lastQuery();
        return;
    }

    auto index = this->baseModel->index(this->baseModel->rowCount() - 1, NamesTableModel::ID);
    auto theId = this->baseModel->data(index).toLongLong();
    auto *theModel = DataManager::getInstance(this)->singleNameModel(this, theId);

    // Show the dialog for the other data.
    auto *editorWindow = new NamesEditor(theModel, true, this);
    editorWindow->show();
    editorWindow->adjustSize();
}

void NamesTableView::editSelectedName() {
    // Get the currently selected name.
    auto selection = this->treeView->selectionModel();
    if (!selection->hasSelection()) {
        return;
    }

    auto selectRow = selection->selectedRows().first();

    // The first column contains the primary key.
    auto index = this->treeView->model()->index(selectRow.row(), 0);
    auto theId = this->treeView->model()->data(index, Qt::EditRole).toLongLong();
    auto *theModel = DataManager::getInstance(this)->singleNameModel(this, theId);
    auto *editorWindow = new NamesEditor(theModel, false, this);
    editorWindow->show();
    editorWindow->adjustSize();
}

void NamesTableView::removeSelectedName() {
    // Get the currently selected name.
    auto selection = this->treeView->selectionModel();
    if (!selection->hasSelection()) {
        return;
    }

    auto selectRow = selection->selectedRows().first();
    this->treeView->model()->removeRow(selectRow.row());
}

void NamesTableView::handleDoubleClick(const QModelIndex &clicked) {
    this->editSelectedName();
}
