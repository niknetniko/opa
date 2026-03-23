/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "person_event_tab.h"

#include "domain/event/event_repository.h"
#include "domain/event/person_events_model.h"
#include "editors/event_editor_dialog.h"
#include "link_existing/choose_existing_event_window.h"
#include "utils/formatted_identifier_delegate.h"
#include "utils/grouping_proxy_model.h"

#include <KLocalizedString>
#include <KRearrangeColumnsProxyModel>
#include <QAbstractButton>
#include <QHeaderView>
#include <QMessageBox>
#include <QToolBar>
#include <QTreeView>
#include <QVBoxLayout>

PersonEventTab::PersonEventTab(IntegerPrimaryKey person, QWidget* parent) : QWidget(parent) {
    this->person = person;
    this->baseModel = new PersonEventListModel(person, this);

    // Wrap with grouping proxy: groups events by role, makes them a tree.
    auto* groupProxy = createGroupingProxyModel(
        baseModel,
        PersonEventListModel::ROLE,
        PersonEventListModel::ID,
        this
    );

    // Rearrange columns: bring virtual ID_AND_ROLE column first, hiding the original ROLE column.
    auto* treeModel = new KRearrangeColumnsProxyModel(this);
    treeModel->setSourceModel(groupProxy);
    treeModel->setSourceColumns({
        baseModel->columnCount(), // virtual ID_AND_ROLE column added by VirtualParentsModel
        PersonEventListModel::TYPE,
        PersonEventListModel::DATE,
        PersonEventListModel::NAME,
        PersonEventListModel::ID,
        PersonEventListModel::ROLE_ID,
        PersonEventListModel::RELATION_ID,
    });

    this->treeView = new QTreeView(this);
    treeView->setModel(treeModel);
    treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    treeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    treeView->setUniformRowHeights(true);
    treeView->hideColumn(PersonEventListModel::ROLE_ID);
    treeView->hideColumn(PersonEventListModel::ID);
    treeView->hideColumn(PersonEventListModel::RELATION_ID);

    // The ID_AND_ROLE column shows role names for group rows and formatted event IDs for leaf rows.
    treeView->setItemDelegateForColumn(
        PersonEventListModel::ROLE,
        new FormattedIdentifierDelegate(treeView, FormattedIdentifierDelegate::EVENT)
    );
    // Change resizes.
    treeView->header()->setSortIndicatorClearable(false);
    treeView->expandAll();

    // Handle a naming being selected.
    connect(treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &PersonEventTab::onEventSelected);
    // Clear the selection when the model is reset.
    //  TODO: is this a bug in Qt?
    connect(treeView->model(), &QAbstractItemModel::modelReset, this, [this] { this->onEventSelected({}); });
    // Edit a name on double-click.
    connect(treeView, &QTreeView::doubleClicked, this, &PersonEventTab::onEditSelectedEvent);

    // Create a toolbar.
    auto* toolbar = new QToolBar(this);

    addAction = new QAction(toolbar);
    addAction->setText(i18n("Add new event"));
    addAction->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
    toolbar->addAction(addAction);

    editAction = new QAction(toolbar);
    editAction->setText(i18n("Edit event"));
    editAction->setIcon(QIcon::fromTheme(QStringLiteral("edit-entry")));
    editAction->setEnabled(false);
    toolbar->addAction(editAction);

    removeAction = new QAction(toolbar);
    removeAction->setText(i18n("Remove event"));
    removeAction->setIcon(QIcon::fromTheme(QStringLiteral("edit-delete")));
    removeAction->setEnabled(false);
    toolbar->addAction(removeAction);

    linkAction = new QAction(toolbar);
    linkAction->setText(i18n("Link event"));
    linkAction->setIcon(QIcon::fromTheme(QStringLiteral("insert-link")));
    toolbar->addAction(linkAction);

    unlinkAction = new QAction(toolbar);
    unlinkAction->setText(i18n("Unlink event"));
    unlinkAction->setIcon(QIcon::fromTheme(QStringLiteral("remove-link")));
    unlinkAction->setEnabled(false);
    toolbar->addAction(unlinkAction);

    // Add them together.
    auto* container = new QVBoxLayout(this);
    container->addWidget(toolbar);
    container->addWidget(treeView);

    // Connect the buttons and stuff.
    connect(addAction, &QAction::triggered, this, &PersonEventTab::onAddNewEvent);
    connect(editAction, &QAction::triggered, this, &PersonEventTab::onEditSelectedEvent);
    connect(removeAction, &QAction::triggered, this, &PersonEventTab::onRemoveSelectedEvent);
    connect(unlinkAction, &QAction::triggered, this, &PersonEventTab::onUnlinkSelectedEvent);
    connect(linkAction, &QAction::triggered, this, &PersonEventTab::onLinkExistingEvent);
}

void PersonEventTab::onEventSelected(const QItemSelection& selected) const {
    // Only enable event actions when a leaf row (actual event) is selected, not a role group header.
    const bool hasLeafSelection = !selected.isEmpty()
        && !selected.indexes().isEmpty()
        && selected.indexes().constFirst().parent().isValid();
    this->editAction->setEnabled(hasLeafSelection);
    this->removeAction->setEnabled(hasLeafSelection);
    this->unlinkAction->setEnabled(hasLeafSelection);
}

void PersonEventTab::onAddNewEvent() {
    EventEditorDialog::showDialogForNewEvent(person, this);
}

void PersonEventTab::onEditSelectedEvent() {
    // Get the currently selected name.
    auto* selection = this->treeView->selectionModel();
    if (!selection->hasSelection()) {
        return;
    }

    Q_ASSERT(selection->selectedRows().size() == 1);
    auto selectedIndex = selection->selectedRows().first();
    if (!selectedIndex.parent().isValid()) {
        return; // group (role header) row selected, nothing to edit
    }

    auto eventId = selectedIndex.siblingAtColumn(PersonEventListModel::ID).data();
    auto eventRoleId = selectedIndex.siblingAtColumn(PersonEventListModel::ROLE_ID).data();
    auto relationId = selectedIndex.siblingAtColumn(PersonEventListModel::RELATION_ID).data();

    EventEditorDialog::showDialogForExistingEvent(
        eventId.toLongLong(), eventRoleId.toLongLong(), relationId.toLongLong(), person, this
    );
}

void PersonEventTab::onRemoveSelectedEvent() const {
    auto* selection = this->treeView->selectionModel();
    if (!selection->hasSelection()) {
        qDebug() << "There is no selection, so not deleting anything.";
        return;
    }
    Q_ASSERT(selection->selectedRows().size() == 1);

    qDebug() << "Selection itself is " << selection;

    auto selectRow = selection->selectedIndexes().first();
    if (!selectRow.parent().isValid()) {
        return; // group (role header) row selected, nothing to remove
    }
    auto eventId = selectRow.siblingAtColumn(PersonEventListModel::ID).data().toLongLong();

    // Look up where it is linked.
    EventRepository repo;
    const auto usedCount = repo.findRelationsForEvent(eventId).size();

    QMessageBox confirmationBox;
    confirmationBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    confirmationBox.setText(i18n("Delete this event?"));
    confirmationBox.button(QMessageBox::Ok)->setText(i18n("Delete event"));
    confirmationBox.setIcon(QMessageBox::Question);
    if (usedCount == 1) {
        confirmationBox.setInformativeText(i18n("This event is not linked to anybody else."));
        confirmationBox.setDefaultButton(QMessageBox::Ok);
    } else {
        confirmationBox.setInformativeText(
            i18np("This event is linked to one other person", "This event is linked to %1 other people", usedCount)
        );
        confirmationBox.setDefaultButton(QMessageBox::Cancel);
    }

    if (confirmationBox.exec() != QMessageBox::Ok) {
        return;
    }

    qDebug() << "Will delete event with ID " << eventId;
    if (!repo.deleteEvent(eventId)) {
        qWarning() << "Could not delete event" << eventId;
    }
}

void PersonEventTab::onUnlinkSelectedEvent() {
    auto* selection = this->treeView->selectionModel();
    if (!selection->hasSelection()) {
        qDebug() << "There is no selection, so not deleting anything.";
        return;
    }
    Q_ASSERT(selection->selectedRows().size() == 1);

    auto selectRow = selection->selectedIndexes().first();
    if (!selectRow.parent().isValid()) {
        return; // group (role header) row selected, nothing to unlink
    }
    auto relationId = selectRow.siblingAtColumn(PersonEventListModel::RELATION_ID).data().toLongLong();

    QMessageBox confirmationBox;
    confirmationBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    confirmationBox.setText(i18n("Unlink this event?"));
    confirmationBox.setInformativeText(
        i18n("This will remove the event from this person, but the event itself will not be deleted.")
    );
    confirmationBox.button(QMessageBox::Ok)->setText(i18n("Unlink event"));
    confirmationBox.setIcon(QMessageBox::Question);
    confirmationBox.setDefaultButton(QMessageBox::Ok);

    if (confirmationBox.exec() != QMessageBox::Ok) {
        return;
    }

    EventRepository repo;
    if (!repo.deleteEventRelation(relationId)) {
        qWarning() << "Could not unlink event relation" << relationId;
    }
}

void PersonEventTab::onLinkExistingEvent() {
    // TODO: allow setting the role of this link!
    const auto selectedEvent = ChooseExistingEventWindow::selectEventAndRole(this);
    if (!selectedEvent.isValid()) {
        return;
    }

    EventRepository repo;
    if (!repo.insertEventRelation(selectedEvent.eventId.toLongLong(), person, selectedEvent.roleId.toLongLong()).has_value()) {
        QMessageBox::warning(
            this, tr("Could not link event"), tr("Problem inserting new event relation into database.")
        );
        qDebug() << "Could not insert event relation for some reason";
    }
}
