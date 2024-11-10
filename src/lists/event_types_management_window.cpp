/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "event_types_management_window.h"

#include "data/data_manager.h"
#include "data/event.h"

#include <KLocalizedString>
#include <QMessageBox>
#include <QProgressDialog>
#include <QSqlQuery>

EventTypesManagementWindow::EventTypesManagementWindow() {
    setWindowTitle(i18n("Manage event types"));

    setModel(DataManager::get().eventTypesModel());
    setColumns(EventTypesModel::ID, EventTypesModel::TYPE, EventTypesModel::BUILTIN);
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

void EventTypesManagementWindow::removeMarkedReferences(
    const QHash<QString, QVector<IntegerPrimaryKey>>& valueToIds, const QHash<IntegerPrimaryKey, QString>& idToValue
) {
    auto* eventsModel = DataManager::get().eventsModel();
    removeReferencesFromModel(valueToIds, idToValue, eventsModel, EventsModel::TYPE_ID);
}

bool EventTypesManagementWindow::isUsed(const QVariant& id) {
    auto* eventsModel = DataManager::get().eventsModel();
    auto usage = eventsModel->match(eventsModel->index(0, EventsModel::TYPE_ID), Qt::DisplayRole, id);
    return !usage.isEmpty();
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
