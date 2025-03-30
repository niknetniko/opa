/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "person_event_tab.h"

#include "data/data_manager.h"
#include "data/event.h"
#include "editors/event_editor_dialog.h"
#include "link_existing/choose_existing_event_window.h"
#include "utils/formatted_identifier_delegate.h"

#include <KLocalizedString>
#include <QAbstractButton>
#include <QHeaderView>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QToolBar>
#include <QTreeView>
#include <QVBoxLayout>

PersonEventTab::PersonEventTab(IntegerPrimaryKey person, QWidget* parent) : QWidget(parent) {
    this->person = person;
    this->baseModel = DataManager::get().treeEventsModelForPerson(this, person);

    this->treeView = new QTreeView(this);
    treeView->setModel(baseModel);
    treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    treeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    treeView->setUniformRowHeights(true);
    treeView->hideColumn(PersonEventsTreeModel::ROLE_ID);
    treeView->hideColumn(PersonEventsTreeModel::ID);

    // The ID should be formatted properly.
    treeView->setItemDelegateForColumn(
        PersonEventsTreeModel::ID_AND_ROLE,
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
    // TODO: prevent something here?
    this->editAction->setEnabled(!selected.isEmpty());
    this->removeAction->setEnabled(!selected.isEmpty());
    this->unlinkAction->setEnabled(!selected.isEmpty());
}

void PersonEventTab::onAddNewEvent() {
    // TODO: Choose the default new event type intelligently.
    // Issue URL: https://github.com/niknetniko/opa/issues/60
    auto newEvent = addEventToPerson(EventTypes::Birth, person);

    auto* singleEventModel = DataManager::get().singleEventModel(this, newEvent.eventId);
    auto* singleRelationModel =
        DataManager::get().singleEventRelationModel(this, newEvent.eventId, newEvent.roleId, person);

    EventEditorDialog::showDialogForNewEvent(singleRelationModel, singleEventModel, this);
}

void PersonEventTab::onEditSelectedEvent() {
    // Get the currently selected name.
    auto* selection = this->treeView->selectionModel();
    if (!selection->hasSelection()) {
        return;
    }

    Q_ASSERT(selection->selectedRows().size() == 1);
    auto selectedIndex = selection->selectedRows().first();
    auto* model = selectedIndex.model();

    auto eventId = model->index(selectedIndex.row(), PersonEventsModel::ID, selectedIndex.parent()).data();
    auto eventRoleId = model->index(selectedIndex.row(), PersonEventsModel::ROLE_ID, selectedIndex.parent()).data();

    auto* eventModel = DataManager::get().singleEventModel(this, eventId);
    auto* relationsModel =
        DataManager::get().singleEventRelationModel(this, eventId, eventRoleId, QVariant::fromValue(person));

    EventEditorDialog::showDialogForExistingEvent(relationsModel, eventModel, this);
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
    auto eventId = treeView->model()->index(selectRow.row(), PersonEventsModel::ID, selectRow.parent()).data();

    // Look up where it is linked.
    auto* relationModel = DataManager::get().eventRelationsModel();
    auto usedCount = relationModel
                         ->match(relationModel->index(0, EventRelationsModel::EVENT_ID), Qt::DisplayRole, eventId, -1)
                         .size();

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

    qDebug() << "Will delete event with ID " << eventId.toLongLong();

    // Find the row in the original model.
    auto* eventsModel = DataManager::get().eventsModel();
    auto result = eventsModel->match(eventsModel->index(0, EventsModel::ID), Qt::DisplayRole, eventId).constFirst();
    if (!eventsModel->removeRow(result.row())) {
        qWarning() << "Could not delete event" << eventId.toLongLong();
    }
    eventsModel->select();
}

void PersonEventTab::onUnlinkSelectedEvent() {
    auto* selection = this->treeView->selectionModel();
    if (!selection->hasSelection()) {
        qDebug() << "There is no selection, so not deleting anything.";
        return;
    }
    Q_ASSERT(selection->selectedRows().size() == 1);

    auto selectRow = selection->selectedIndexes().first();
    auto eventId = treeView->model()->index(selectRow.row(), PersonEventsModel::ID, selectRow.parent()).data();
    auto roleId = treeView->model()->index(selectRow.row(), PersonEventsModel::ROLE_ID, selectRow.parent()).data();

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

    auto* relationModel = DataManager::get().eventRelationsModel();
    auto* matchedModel = DataManager::get().singleEventRelationModel(this, eventId, roleId, person);
    auto originalRow = mapToSourceModel(matchedModel->index(0, 0));

    if (!relationModel->removeRow(originalRow.row())) {
        qWarning() << "Could not unlink event" << eventId.toLongLong();
    }
    relationModel->select();
}

void PersonEventTab::onLinkExistingEvent() {
    // TODO: allow setting the role of this link!
    const auto selectedEvent = ChooseExistingEventWindow::selectEventAndRole(this);
    if (!selectedEvent.isValid()) {
        return;
    }

    auto* eventRelationModel = DataManager::get().eventRelationsModel();
    auto eventRelationRecord = eventRelationModel->record();
    eventRelationRecord.setValue(EventRelationsModel::EVENT_ID, selectedEvent.eventId);
    eventRelationRecord.setValue(EventRelationsModel::PERSON_ID, person);
    eventRelationRecord.setValue(EventRelationsModel::ROLE_ID, selectedEvent.roleId);

    if (!eventRelationModel->insertRecord(-1, eventRelationRecord)) {
        QMessageBox::warning(
            this, tr("Could not event relation"), tr("Problem inserting new event relation into database.")
        );
        qDebug() << "Could not insert event relation for some reason:";
        qDebug() << eventRelationModel->lastError();
    }
}
