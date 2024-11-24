/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include <qabstractproxymodel.h>

#include <KLazyLocalizedString>
#include <QMetaEnum>

/**
 * Find the first source model of a specific type.
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

template<typename E>
bool isValidEnum(const QString& value) {
    auto result = QMetaEnum::fromType<E>().keyToValue(value.toLatin1().constData());
    return result >= 0;
}

template<typename E>
E enumFromString(const QString& value) {
    auto result = QMetaEnum::fromType<E>().keyToValue(value.toLatin1().constData());
    assert(result >= 0);
    return static_cast<E>(result);
}

template<typename E>
QString genericToDisplayString(const QString& databaseValue, QHash<E, KLazyLocalizedString> mapping) {
    auto result = QMetaEnum::fromType<E>().keyToValue(databaseValue.toLatin1().constData());
    if (result == -1) {
        // This is not a built-in type, so do nothing with it.
        return databaseValue;
    }
    auto enumValue = static_cast<E>(result);
    return mapping[enumValue].toString();
}
