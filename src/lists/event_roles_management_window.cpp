/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "event_roles_management_window.h"

#include "../domain/event/event_roles.h"
#include "../domain/event/event_types.h"
#include "domain/event/event_repository.h"
#include "domain/event/event_roles_model.h"

#include <KLocalizedString>
#include <QMessageBox>
#include <QProgressDialog>

EventRolesManagementWindow::EventRolesManagementWindow() {
    setWindowTitle(i18n("Manage event roles"));

    auto* model = new EventRolesListModel(this);
    setModel(model);
    setColumns(EventRolesListModel::ID, EventRolesListModel::ROLE, EventRolesListModel::BUILTIN);
    setTranslator(EventRoles::toDisplayString);

    initializeLayout();
}

bool EventRolesManagementWindow::repairConfirmation() {
    QMessageBox messageBox(this);
    messageBox.setIcon(QMessageBox::Question);
    messageBox.setText(i18n("Clean up event roles?"));
    messageBox.setInformativeText(
        i18n("This will merge duplicate and remove empty event roles. Do you want to continue?")
    );
    messageBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    messageBox.setDefaultButton(QMessageBox::Ok);
    return messageBox.exec() == QMessageBox::Ok;
}

void EventRolesManagementWindow::repairItems() {
    if (!repairConfirmation()) {
        return;
    }

    QProgressDialog progress(i18n("Opschonen..."), QString(), 0, 5, this);
    progress.setModal(true);
    progress.setValue(0);

    EventRepository repo;

    // Normalize all values.
    const auto allRoles = repo.findAllEventRoles();
    for (const auto& entity: allRoles) {
        auto trimmed = entity.role.simplified();
        auto lowered = trimmed.toLower();
        if (!lowered.isEmpty()) {
            lowered[0] = lowered[0].toTitleCase();
        }
        if (lowered != entity.role) {
            repo.updateEventRole(entity.id, lowered);
        }
    }
    progress.setValue(1);

    // Determine duplicates.
    const auto updatedRoles = repo.findAllEventRoles();
    QHash<QString, QVector<IntegerPrimaryKey>> valueToIds;
    QHash<IntegerPrimaryKey, QString> idToValue;
    for (const auto& entity: updatedRoles) {
        valueToIds[entity.role].append(entity.id);
        idToValue[entity.id] = entity.role;
    }
    progress.setValue(2);

    // Reassign references from duplicate IDs to the canonical ID.
    for (auto i = valueToIds.begin(); i != valueToIds.end(); ++i) {
        if (i.value().length() <= 1)
            continue;
        const auto keepId = i.value().first();
        for (int j = 1; j < i.value().length(); ++j) {
            repo.reassignEventRoleId(i.value()[j], keepId);
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
        repo.deleteEventRole(id);
    }
    progress.setValue(5);
}

void EventRolesManagementWindow::removeMarkedReferences(
    [[maybe_unused]] const QHash<QString, QVector<IntegerPrimaryKey>>& valueToIds,
    [[maybe_unused]] const QHash<IntegerPrimaryKey, QString>& idToValue
) {
    // Not used: repairItems() is overridden to use the repository directly.
}

bool EventRolesManagementWindow::isUsed(const QVariant& id) {
    EventRepository repo;
    return repo.isEventRoleUsed(id.toLongLong());
}

QVariant EventRolesManagementWindow::doAddItem() const {
    EventRepository repo;
    const auto newId = repo.insertEventRole(QString());
    return newId ? QVariant(*newId) : QVariant{};
}

bool EventRolesManagementWindow::doRemoveItem(const QVariant& id) const {
    EventRepository repo;
    return repo.deleteEventRole(id.toLongLong());
}

QString EventRolesManagementWindow::translatedItemCount(int itemCount) const {
    return i18np("%1 event role", "%1 event roles", itemCount);
}

QString EventRolesManagementWindow::translatedItemDescription(const QString& item, bool isBuiltIn) const {
    if (isBuiltIn) {
        return i18n("Built-in event role '%1'", item);
    }
    return i18n("Event role '%1'", item);
}
