/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "name_origins_management_window.h"

#include "../core/data_event_broker.h"
#include "../domain/name/name_repository.h"
#include "../domain/name/names.h"
#include "database/schema.h"
#include "utils/model_utils.h"

#include <KLocalizedString>
#include <QMessageBox>
#include <QProgressDialog>
#include <QSqlError>
#include <QSqlQuery>

NameOriginsManagementWindow::NameOriginsManagementWindow() {
    setWindowTitle(i18n("Manage name origins"));

    originsModel = new NameOriginsModel(this);
    setModel(originsModel);
    setColumns(NameOriginsModel::ID, NameOriginsModel::ORIGIN, NameOriginsModel::BUILTIN);
    setTranslator(NameOrigins::toDisplayString);

    initializeLayout();
}

QVariant NameOriginsManagementWindow::doAddItem() const {
    const auto sql = QStringLiteral("INSERT INTO name_origins (origin, builtin) VALUES ('', 0)");
    QSqlQuery query;
    if (!query.exec(sql)) {
        qWarning() << "Could not insert new name origin:" << query.lastError().text();
        return {};
    }
    auto newId = query.lastInsertId();
    DataEventBroker::instance().notifyChanged<Schema::NameOrigins>(newId.toLongLong());
    return newId;
}

bool NameOriginsManagementWindow::doRemoveItem(const QVariant& id) const {
    const auto sql = QStringLiteral("DELETE FROM name_origins WHERE id = :id AND builtin = 0");
    QSqlQuery query;
    query.prepare(sql);
    query.bindValue(QStringLiteral(":id"), id);
    if (!query.exec()) {
        qWarning() << "Could not delete name origin:" << query.lastError().text();
        return false;
    }
    DataEventBroker::instance().notifyChanged<Schema::NameOrigins>(id.toLongLong());
    return true;
}

void NameOriginsManagementWindow::repairItems() {
    if (!repairConfirmation()) {
        return;
    }

    QProgressDialog progress(i18n("Opschonen..."), QString(), 0, 5, this);
    progress.setModal(true);
    progress.setValue(0);

    // Trim all values.
    const auto& allOrigins = originsModel->getItems();
    for (const auto& origin: allOrigins) {
        auto trimmed = origin.origin.simplified();
        auto lowered = trimmed.toLower();
        if (!lowered.isEmpty()) {
            lowered[0] = lowered[0].toTitleCase();
        }
        if (lowered != origin.origin) {
            QSqlQuery q;
            q.prepare(QStringLiteral("UPDATE name_origins SET origin = :origin WHERE id = :id"));
            q.bindValue(QStringLiteral(":origin"), lowered);
            q.bindValue(QStringLiteral(":id"), origin.id);
            q.exec();
        }
    }
    DataEventBroker::instance().notifyChanged<Schema::NameOrigins>(-1);
    progress.setValue(1);

    // Reload after trim.
    NameRepository repo;
    const auto updated = repo.findAllOrigins();

    // Determine duplicates.
    QHash<QString, QVector<IntegerPrimaryKey>> valueToIds;
    QHash<IntegerPrimaryKey, QString> idToValue;
    for (const auto& origin: updated) {
        valueToIds[origin.origin].append(origin.id);
        idToValue[origin.id] = origin.origin;
    }
    progress.setValue(2);

    this->removeMarkedReferences(valueToIds, idToValue);
    progress.setValue(3);

    // Determine the list of removals.
    QSet<IntegerPrimaryKey> toRemove;
    for (auto i = valueToIds.begin(); i != valueToIds.end(); ++i) {
        if (i.key() == QString()) {
            toRemove.unite(QSet(i.value().begin(), i.value().end()));
            continue;
        }
        if (i.value().length() == 1) {
            continue;
        }
        toRemove.unite(QSet(std::next(i.value().begin()), i.value().end()));
    }
    progress.setValue(4);

    for (const auto id: toRemove) {
        QSqlQuery q;
        q.prepare(QStringLiteral("DELETE FROM name_origins WHERE id = :id AND builtin = 0"));
        q.bindValue(QStringLiteral(":id"), id);
        if (!q.exec()) {
            qWarning() << "Could not remove row" << id << "!" << q.lastError().text();
        }
    }

    DataEventBroker::instance().notifyChanged<Schema::NameOrigins>(-1);
    progress.setValue(5);
}

bool NameOriginsManagementWindow::repairConfirmation() {
    QMessageBox messageBox(this);
    messageBox.setIcon(QMessageBox::Question);
    messageBox.setText(i18n("Clean up name origins?"));
    messageBox.setInformativeText(
        i18n("This will merge duplicate and remove empty name origins. Do you want to continue?")
    );
    messageBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    messageBox.setDefaultButton(QMessageBox::Ok);

    return messageBox.exec() == QMessageBox::Ok;
}

void NameOriginsManagementWindow::removeMarkedReferences(
    const QHash<QString, QVector<IntegerPrimaryKey>>& valueToIds, const QHash<IntegerPrimaryKey, QString>& idToValue
) {
    // Update names that reference a duplicate origin to point to the surviving (first) origin.
    const auto sql = QStringLiteral("SELECT id, origin_id FROM names WHERE origin_id IS NOT NULL");
    QSqlQuery selectQuery;
    if (!selectQuery.exec(sql)) {
        qWarning() << "Could not query names:" << selectQuery.lastError().text();
        return;
    }

    while (selectQuery.next()) {
        const auto nameId = selectQuery.value(0).toLongLong();
        const auto originId = selectQuery.value(1).toLongLong();

        if (!idToValue.contains(originId)) {
            continue;
        }

        const auto value = idToValue[originId];
        const auto idsForValue = valueToIds[value];

        if (idsForValue.length() <= 1) {
            continue;
        }

        // Point to the first (surviving) origin.
        QSqlQuery updateQuery;
        updateQuery.prepare(QStringLiteral("UPDATE names SET origin_id = :origin_id WHERE id = :id"));
        updateQuery.bindValue(QStringLiteral(":origin_id"), idsForValue.first());
        updateQuery.bindValue(QStringLiteral(":id"), nameId);
        if (!updateQuery.exec()) {
            qWarning() << "Could not update name origin reference:" << updateQuery.lastError().text();
        }
    }

    DataEventBroker::instance().notifyChanged<Schema::Names>(-1);
}

bool NameOriginsManagementWindow::isUsed(const QVariant& id) {
    QSqlQuery query;
    query.prepare(QStringLiteral("SELECT COUNT(*) FROM names WHERE origin_id = :id"));
    query.bindValue(QStringLiteral(":id"), id);
    if (query.exec() && query.next()) {
        return query.value(0).toInt() > 0;
    }
    return false;
}

QString NameOriginsManagementWindow::translatedItemCount(int itemCount) const {
    return i18np("%1 name origin", "%1 name origins", itemCount);
}

QString NameOriginsManagementWindow::translatedItemDescription(const QString& item, bool isBuiltIn) const {
    if (isBuiltIn) {
        return i18n("Built-in name origin '%1'", item);
    }
    return i18n("Name origin '%1'", item);
}
