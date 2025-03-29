/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "database/schema.h"

#include <KLazyLocalizedString>
#include <QAbstractProxyModel>
#include <QSqlTableModel>

/**
 * Execute a "transaction" on a model.
 *
 * Concretely, it will set the model to manual submit, submit all changes
 * and finally restore the original submit policy.
 */
template<typename T>
bool modelTransaction(QSqlTableModel* model, T&& transaction) {
    auto originalStrategy = model->editStrategy();
    Q_ASSERT(!model->isDirty());
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);

    // Do the transaction.
    transaction();

    bool wasSubmitted = model->submitAll();
    model->setEditStrategy(originalStrategy);
    return wasSubmitted;
}

/**
 * Find the first source model of a specific type.
 * This function supports nested proxy models.
 *
 * @tparam T The type to search for.
 * @param model The proxy model to find the source for.
 * @return The model pointer or nullptr if there is none.
 */
template<class T>
const T* findSourceModelOfType(const QAbstractItemModel* model) {
    // A special case: if this is the model we search, just return it.
    const T* secondCast = qobject_cast<const T*>(model);
    if (secondCast != nullptr) {
        return secondCast;
    }

    // Else, start going down the path.
    const QAbstractItemModel* potentialModel = model;
    const QAbstractProxyModel* proxyModel = nullptr;
    while ((proxyModel = qobject_cast<const QAbstractProxyModel*>(potentialModel)) != nullptr) {
        potentialModel = proxyModel->sourceModel();
        // If this is the type we search, stop now.
        secondCast = qobject_cast<const T*>(potentialModel);
        if (secondCast != nullptr) {
            return secondCast;
        }
    }

    // We have not found anything.
    return nullptr;
}

/**
 * Convert indices to source models until there are no more proxy models.
 *
 * This will use the model attached to the given index.
 *
 * @param index The index to convert to a source.
 *
 * @return The converted model index.
 */
inline QModelIndex mapToSourceModel(const QModelIndex& index) {
    const auto* model = qobject_cast<const QAbstractProxyModel*>(index.model());
    if (model == nullptr) {
        // There is no proxy model in the index.
        return index;
    }

    QModelIndex converted = model->mapToSource(index);
    while ((model = qobject_cast<const QAbstractProxyModel*>(model->sourceModel())) != nullptr) {
        converted = model->mapToSource(converted);
    }

    // We have not found anything.
    return converted;
}

/**
 * Check if a string is a valid enum value.
 *
 * @tparam E The enum to use.
 * @param value The value to check.
 * @return True if it is, false otherwise.
 */
template<typename E>
bool isValidEnum(const QString& value) {
    auto result = QMetaEnum::fromType<E>().keyToValue(value.toLatin1().constData());
    return result >= 0;
}

/**
 * Convert a string to an enum.
 *
 * @tparam E The enum to use.
 * @param value A valid enum value, see isValidEnum for checking.
 * @return The corresponding enum value.
 */
template<typename E>
E enumFromString(const QString& value) {
    auto result = QMetaEnum::fromType<E>().keyToValue(value.toLatin1().constData());
    Q_ASSERT(result >= 0);
    return static_cast<E>(result);
}

/**
 * Convert a database value to a display string associated with an enum.
 * If the provided string is not a valid enum, the string is returned as-is.
 *
 * @tparam E The enum to use.
 * @param databaseValue The string to attempt to convert into an enum.
 * @param mapping The mapping of enum values to string values.
 * @return The display string or original value if it was not a valid enum value.
 */
template<typename E>
QString genericToDisplayString(const QString& databaseValue, QHash<E, KLazyLocalizedString> mapping) {
    auto result = QMetaEnum::fromType<E>().keyToValue(databaseValue.toLatin1().constData());
    if (result == -1) {
        // This is not a built-in type, so do nothing with it.
        return databaseValue;
    }
    auto enumValue = static_cast<E>(result);
    Q_ASSERT(mapping.contains(enumValue));
    return mapping[enumValue].toString();
}

/**
 * Get the database ID of a built-in type value.
 *
 * @tparam E The type.
 * @param model The model for the types.
 * @param eventType The actual type.
 * @param enumMapping Mapping of the enum to database strings.
 * @param typeColumn The column containing the type.
 * @param idColumn The column containing the ID.
 * @return
 */
template<typename E>
IntegerPrimaryKey getTypeId(
    QAbstractItemModel* model, E eventType, QHash<E, KLazyLocalizedString> enumMapping, int typeColumn, int idColumn
) {
    auto databaseValue = QString::fromLatin1(enumMapping[eventType].untranslatedText());
    auto defaultEventRoleIndex = model->match(model->index(0, typeColumn), Qt::DisplayRole, databaseValue).constFirst();
    Q_ASSERT(defaultEventRoleIndex.isValid());

    auto idVariant = model->index(defaultEventRoleIndex.row(), idColumn).data();
    Q_ASSERT(idVariant.isValid());
    return idVariant.toLongLong();
}

void debugPrintModel(const QAbstractItemModel* model, const QModelIndex& parent = QModelIndex(), int level = 0);

/**
 * Return true if a variant is "practically" invalid, meaning it is null, invalid or the empty string.
 */
bool isInvalid(const QVariant& variant);
