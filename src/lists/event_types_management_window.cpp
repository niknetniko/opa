/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "event_types_management_window.h"

#include "../domain/event/event_types.h"
#include "domain/event/event_repository.h"
#include "domain/event/event_types_model.h"

#include <KLocalizedString>
#include <QMessageBox>
#include <QProgressDialog>

EventTypesManagementWindow::EventTypesManagementWindow() {
    setWindowTitle(i18n("Manage event types"));

    auto* model = new EventTypesListModel(this);
    setModel(model);
    setColumns(EventTypesListModel::ID, EventTypesListModel::TYPE, EventTypesListModel::BUILTIN);
    setTranslator(EventTypes::toDisplayString);

    initializeLayout();
}

bool EventTypesManagementWindow::repairConfirmation() {
    QMessageBox messageBox(this);
    messageBox.setIcon(QMessageBox::Question);
    messageBox.setText(i18n("Clean up event types?"));
    messageBox.setInformativeText(
        i18n("This will merge duplicate and remove empty event types. Do you want to continue?")
    );
    messageBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    messageBox.setDefaultButton(QMessageBox::Ok);
    return messageBox.exec() == QMessageBox::Ok;
}

void EventTypesManagementWindow::repairItems() {
    if (!repairConfirmation()) {
        return;
    }

    QProgressDialog progress(i18n("Opschonen..."), QString(), 0, 5, this);
    progress.setModal(true);
    progress.setValue(0);

    EventRepository repo;

    // Normalize all values.
    const auto allTypes = repo.findAllEventTypes();
    for (const auto& entity: allTypes) {
        auto trimmed = entity.type.simplified();
        auto lowered = trimmed.toLower();
        if (!lowered.isEmpty()) {
            lowered[0] = lowered[0].toTitleCase();
        }
        if (lowered != entity.type) {
            repo.updateEventType(entity.id, lowered);
        }
    }
    progress.setValue(1);

    // Determine duplicates.
    const auto updatedTypes = repo.findAllEventTypes();
    QHash<QString, QVector<IntegerPrimaryKey>> valueToIds;
    QHash<IntegerPrimaryKey, QString> idToValue;
    for (const auto& entity: updatedTypes) {
        valueToIds[entity.type].append(entity.id);
        idToValue[entity.id] = entity.type;
    }
    progress.setValue(2);

    // Reassign references from duplicate IDs to the canonical ID.
    for (auto i = valueToIds.begin(); i != valueToIds.end(); ++i) {
        if (i.value().length() <= 1)
            continue;
        const auto keepId = i.value().first();
        for (int j = 1; j < i.value().length(); ++j) {
            repo.reassignEventTypeId(i.value()[j], keepId);
        }
    }
    progress.setValue(3);

    // Delete duplicates and empty entries.
    QSet<IntegerPrimaryKey> toRemove;
    for (auto i = valueToIds.begin(); i != valueToIds.end(); ++i) {
        if (i.key().isEmpty()) {
            toRemove.unite(QSet(i.value().begin(), i.value().end()));
            continue;
        }
        if (i.value().length() > 1) {
            toRemove.unite(QSet(std::next(i.value().begin()), i.value().end()));
        }
    }
    progress.setValue(4);

    for (const auto id: toRemove) {
        repo.deleteEventType(id);
    }
    progress.setValue(5);
}

void EventTypesManagementWindow::removeMarkedReferences(
    [[maybe_unused]] const QHash<QString, QVector<IntegerPrimaryKey>>& valueToIds,
    [[maybe_unused]] const QHash<IntegerPrimaryKey, QString>& idToValue
) {
    // Not used: repairItems() is overridden to use the repository directly.
}

bool EventTypesManagementWindow::isUsed(const QVariant& id) {
    EventRepository repo;
    return repo.isEventTypeUsed(id.toLongLong());
}

QVariant EventTypesManagementWindow::doAddItem() const {
    EventRepository repo;
    const auto newId = repo.insertEventType(QString());
    return newId ? QVariant(*newId) : QVariant{};
}

bool EventTypesManagementWindow::doRemoveItem(const QVariant& id) const {
    EventRepository repo;
    return repo.deleteEventType(id.toLongLong());
}

QString EventTypesManagementWindow::translatedItemCount(int itemCount) const {
    return i18np("%1 event type", "%1 event types", itemCount);
}

QString EventTypesManagementWindow::translatedItemDescription(const QString& item, bool isBuiltIn) const {
    if (isBuiltIn) {
        return i18n("Built-in event type '%1'", item);
    }
    return i18n("Event type '%1'", item);
}
