/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "person_name_tab.h"

#include "../domain/name/name_repository.h"
#include "../domain/name/names.h"
#include "../domain/name/person_names_model.h"
#include "../ui/name/name_editor_dialog.h"
#include "utils/builtin_text_translating_delegate.h"
#include "utils/formatted_identifier_delegate.h"
#include "utils/model_utils.h"

#include <KLocalizedString>
#include <QHeaderView>
#include <QMessageBox>
#include <QToolBar>
#include <QTreeView>
#include <QVBoxLayout>
#include <numeric>

PersonNameTab::PersonNameTab(IntegerPrimaryKey person, QWidget* parent) : QWidget(parent) {
    this->person = person;
    this->baseModel = new PersonNamesModel(person, this);

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
        PersonNamesModel::ID,
        new FormattedIdentifierDelegate(treeView, FormattedIdentifierDelegate::NAME)
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
    NameRepository nameRepo;
    const int nextSort = treeView->model()->rowCount() + 1;
    const auto newNameId = nameRepo.insertName(this->person, nextSort);
    if (!newNameId.has_value()) {
        QMessageBox::warning(this, tr("Could not insert name"), tr("Problem inserting new name into database."));
        return;
    }

    NamesEditorDialog::showDialogForNewName(*newNameId, this);
}

void PersonNameTab::onEditSelectedName() {
    auto* selection = treeView->selectionModel();
    if (!selection->hasSelection()) {
        return;
    }

    Q_ASSERT(selection->selectedRows().size() == 1);
    auto selectedIndex = selection->selectedIndexes().first();
    auto* model = selectedIndex.model();

    auto idToEdit = model->index(selectedIndex.row(), PersonNamesModel::ID).data().toLongLong();

    NamesEditorDialog::showDialogForExistingName(idToEdit, this);
}

void PersonNameTab::onRemoveSelectedName() const {
    auto* selection = treeView->selectionModel();
    if (!selection->hasSelection()) {
        return;
    }

    Q_ASSERT(selection->selectedRows().size() == 1);

    auto selectedIndex = selection->selectedRows().first();
    auto nameId = treeView->model()->index(selectedIndex.row(), PersonNamesModel::ID).data().toLongLong();

    NameRepository nameRepo;
    if (!nameRepo.deleteName(nameId)) {
        qWarning() << "Could not remove name" << nameId;
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

    QVector<int> sortValues(model->rowCount());
    std::ranges::iota(sortValues, 1);
    std::swap(sortValues[sourceRow], sortValues[destinationRow]);

    NameRepository nameRepo;
    for (int row = 0; row < model->rowCount(); ++row) {
        auto nameId = model->index(row, PersonNamesModel::ID).data().toLongLong();
        if (!nameRepo.updateNameSort(nameId, sortValues[row])) {
            qWarning() << "Could not update sort for name" << nameId;
        }
    }

    treeView->selectionModel()->select(
        model->index(destinationRow, 0),
        QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows
    );
}
