/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "translating_proxy_model.h"

#include <QLocale>

TranslatingProxyModel::TranslatingProxyModel(TypeTranslationResolver resolver, QObject* parent) :
    QIdentityProxyModel(parent),
    resolver(std::move(resolver)) {
}

QVariant TranslatingProxyModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.column() != TYPE_COLUMN || role != Qt::DisplayRole) {
        return QIdentityProxyModel::data(index, role);
    }

    const auto typeString = QIdentityProxyModel::data(index, Qt::DisplayRole).toString();
    const auto builtinIdx = QIdentityProxyModel::index(index.row(), BUILTIN_COLUMN, index.parent());
    const auto builtin = QIdentityProxyModel::data(builtinIdx, Qt::DisplayRole).toBool();
    const auto idIdx = QIdentityProxyModel::index(index.row(), ID_COLUMN, index.parent());
    const auto typeId = QIdentityProxyModel::data(idIdx, Qt::DisplayRole).toLongLong();

    const auto locale = QLocale::system().name();
    return resolver.resolve(typeString, builtin, typeId, locale);
}
