/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "person_name_tab.h"

#include "data/data_manager.h"
#include "data/names.h"
#include "editors/name_editor_dialog.h"
#include "utils/builtin_text_translating_delegate.h"
#include "utils/formatted_identifier_delegate.h"
#include "utils/model_utils.h"

#include <KLocalizedString>
#include <QHeaderView>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QToolBar>
#include <QTreeView>
#include <QVBoxLayout>

PersonNameTab::PersonNameTab(IntegerPrimaryKey person, QWidget* parent) : QWidget(parent) {
    this->person = person;
    this->baseModel = DataManager::get().namesModelForPerson(this, person);

    this->treeView = new QTreeView(this);
    treeView->setModel(baseModel);
    treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    treeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    treeView->setUniformRowHeights(true);
    treeView->setRootIsDecorated(false);
    treeView->setSortingEnabled(true);
    treeView->sortByColumn(PersonNamesModel::SORT, Qt::AscendingOrder);
    treeView->header()->setSortIndicatorClearable(false);

    // The ID should be formatted properly.
    treeView->setItemDelegateForColumn(
        PersonNamesModel::ID, new FormattedIdentifierDelegate(treeView, FormattedIdentifierDelegate::NAME)
    );
    // Origins must be translated.
    auto* originTranslator = new BuiltinTextTranslatingDelegate(treeView);
    originTranslator->setTranslator(NameOrigins::toDisplayString);
    treeView->setItemDelegateForColumn(PersonNamesModel::ORIGIN, originTranslator);

    // Handle a naming being selected.
    connect(treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &PersonNameTab::onNameSelected);
    // Clear the selection when the model is reset.
    //  TODO: is this a bug in Qt?
    connect(treeView->model(), &QAbstractItemModel::modelReset, this, [this] { this->onNameSelected({}); });
    // Edit a name on double-click.
    connect(treeView, &QTreeView::doubleClicked, this, &PersonNameTab::onEditSelectedName);
    // Preserve the selection when the sorting changes.
    connect(treeView->header(), &QHeaderView::sortIndicatorChanged, this, [this](int logicalIndex) {
        auto* model = treeView->selectionModel();
        onSortChanged(model->selection(), logicalIndex);
    });

    // Create a toolbar.
    auto* nameToolbar = new QToolBar(this);

    addAction = new QAction(nameToolbar);
    addAction->setText(i18n("Nieuwe naam toevoegen"));
    addAction->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
    nameToolbar->addAction(addAction);

    editAction = new QAction(nameToolbar);
    editAction->setText(i18n("Naam bewerken"));
    editAction->setIcon(QIcon::fromTheme(QStringLiteral("edit-entry")));
    editAction->setEnabled(false);
    nameToolbar->addAction(editAction);

    removeAction = new QAction(nameToolbar);
    removeAction->setText(i18n("Naam verwijderen"));
    removeAction->setIcon(QIcon::fromTheme(QStringLiteral("edit-delete")));
    removeAction->setEnabled(false);
    nameToolbar->addAction(removeAction);

    upAction = new QAction(nameToolbar);
    upAction->setText(i18n("Omhoog"));
    upAction->setIcon(QIcon::fromTheme(QStringLiteral("go-up")));
    upAction->setEnabled(false);
    nameToolbar->addAction(upAction);

    downAction = new QAction(nameToolbar);
    downAction->setText(i18n("Omlaag"));
    downAction->setIcon(QIcon::fromTheme(QStringLiteral("go-down")));
    downAction->setEnabled(false);
    nameToolbar->addAction(downAction);

    // Add them together.
    auto* container = new QVBoxLayout(this);
    container->addWidget(nameToolbar);
    container->addWidget(treeView);

    // Connect the buttons and stuff.
    connect(addAction, &QAction::triggered, this, &PersonNameTab::onAddNewName);
    connect(editAction, &QAction::triggered, this, &PersonNameTab::onEditSelectedName);
    connect(removeAction, &QAction::triggered, this, &PersonNameTab::onRemoveSelectedName);
    connect(upAction, &QAction::triggered, this, &PersonNameTab::onMoveSelectedNameUp);
    connect(downAction, &QAction::triggered, this, &PersonNameTab::onMoveSelectedNameDown);
}

void PersonNameTab::onNameSelected(const QItemSelection& selected) const {
    if (selected.isEmpty()) {
        this->editAction->setEnabled(false);
        this->removeAction->setEnabled(false);
        this->downAction->setEnabled(false);
        this->upAction->setEnabled(false);
        return;
    }

    this->editAction->setEnabled(true);

    auto selectedIndex = selected.indexes().first();
    auto* model = selectedIndex.model();

    int nameSort = model->index(selectedIndex.row(), PersonNamesModel::SORT).data().toInt();
    int rowCount = model->rowCount();

    // Do not allow removal of the main name.
    this->removeAction->setEnabled(nameSort != 1);
    // Do not allow moving the first name up.
    this->upAction->setEnabled(nameSort > 1 && rowCount > 1);
    // Do not allow moving the last name down.
    this->downAction->setEnabled(nameSort < model->rowCount() && rowCount > 1);
}

void PersonNameTab::onSortChanged(const QItemSelection& selected, const int logicalIndex) const {
    // Check if there is currently a selected item.
    if (selected.isEmpty()) {
        // Leave everything alone as it was.
        // The relevant actions should already be disabled.
        return;
    }

    bool allowUpAndDown = logicalIndex < 0 || logicalIndex == 1;
    // Only allow up and down if the view is not sorted or sorted on the second column.
    this->upAction->setEnabled(this->upAction->isEnabled() && allowUpAndDown);
    this->downAction->setEnabled(this->downAction->isEnabled() && allowUpAndDown);
}

void PersonNameTab::onAddNewName() {
    auto* namesModel = DataManager::get().namesModel();

    // Add a new row to the table for editing.
    auto newRecord = namesModel->record();
    newRecord.setGenerated(NamesTableModel::ID, false);
    newRecord.setValue(NamesTableModel::PERSON_ID, this->person);
    newRecord.setValue(NamesTableModel::SORT, treeView->model()->rowCount() + 1);
    if (!namesModel->insertRecord(-1, newRecord)) {
        QMessageBox::warning(this, tr("Could not insert name"), tr("Problem inserting new name into database."));
        qDebug() << "Could not get last inserted ID for some reason:";
        qDebug() << namesModel->lastError();
        return;
    }

    auto lastInsertedId = namesModel->query().lastInsertId();
    if (!lastInsertedId.isValid()) {
        QMessageBox::warning(this, tr("Could not insert name"), tr("Problem inserting new name into database."));
        qDebug() << "Could not get last inserted ID for some reason:";
        qDebug() << namesModel->lastError();
        return;
    }

    auto theId = lastInsertedId.toLongLong();
    auto* singleModel = DataManager::get().singleNameModel(this, theId);

    NamesEditorDialog::showDialogForNewName(singleModel, this);
}

void PersonNameTab::onEditSelectedName() {
    auto* selection = treeView->selectionModel();
    if (!selection->hasSelection()) {
        return;
    }

    Q_ASSERT(selection->selectedRows().size() == 1);
    auto selectedIndex = selection->selectedIndexes().first();
    auto* model = selectedIndex.model();

    auto idToEdit = model->index(selectedIndex.row(), PersonNamesModel::ID).data();
    auto* singleModel = DataManager::get().singleNameModel(this, idToEdit);

    NamesEditorDialog::showDialogForExistingName(singleModel, this);
}

void PersonNameTab::onRemoveSelectedName() const {
    auto* selection = treeView->selectionModel();
    if (!selection->hasSelection()) {
        return;
    }

    Q_ASSERT(selection->selectedRows().size() == 1);

    if (!treeView->model()->removeRow(selection->selectedRows().first().row())) {
        qWarning() << "Could not remove row.";
    }
}

void PersonNameTab::onMoveSelectedNameDown() const {
    auto* selection = treeView->selectionModel();
    if (!selection->hasSelection()) {
        return;
    }

    Q_ASSERT(selection->selectedRows().size() == 1);
    auto selectRow = selection->selectedRows().first().row();
    auto newRow = selectRow + 1;
    moveSelectedNameToPosition(selectRow, newRow);
}

void PersonNameTab::onMoveSelectedNameUp() const {
    auto* selection = treeView->selectionModel();
    if (!selection->hasSelection()) {
        return;
    }

    Q_ASSERT(selection->selectedRows().size() == 1);
    auto selectRow = selection->selectedRows().first().row();
    auto newRow = selectRow - 1;
    moveSelectedNameToPosition(selectRow, newRow);
}

void PersonNameTab::moveSelectedNameToPosition(int sourceRow, int destinationRow) const {
    auto* model = treeView->model();

    Q_ASSERT(sourceRow >= 0 && sourceRow < model->rowCount());
    Q_ASSERT(destinationRow >= 0 && destinationRow < model->rowCount());

    QVector<int> vector(model->rowCount());
    std::iota(std::begin(vector), std::end(vector), 1);
    std::swap(vector[sourceRow], vector[destinationRow]);

    // We want to update this in one go; so get the root model.
    auto* namesModel = DataManager::get().namesModel();

    auto saved = modelTransaction(namesModel, [&vector, &model] {
        for (int row = 0; row < vector.length(); ++row) {
            // Do this on the original model, so no need to map changes.
            model->setData(model->index(row, 1), vector[row]);
        }
    });

    // Commit the data and done.
    if (!saved) {
        qDebug() << "Could not submit for some reason";
        qDebug() << namesModel->lastError();
    }

    treeView->selectionModel()->select(
        model->index(destinationRow, 0), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows
    );
}
