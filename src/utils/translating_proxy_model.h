/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "type_translation_resolver.h"

#include <QIdentityProxyModel>

/**
 * Proxy model that replaces the display text in a type column with resolved translations.
 *
 * Wraps a source model with columns ID, TYPE, BUILTIN (indices 0, 1, 2).
 * For the TYPE column, Qt::DisplayRole is intercepted and resolved via TypeTranslationResolver.
 */
class TranslatingProxyModel : public QIdentityProxyModel {
    Q_OBJECT

public:
    static constexpr int ID_COLUMN = 0;
    static constexpr int TYPE_COLUMN = 1;
    static constexpr int BUILTIN_COLUMN = 2;

    explicit TranslatingProxyModel(TypeTranslationResolver resolver, QObject* parent = nullptr);

    [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

private:
    TypeTranslationResolver resolver;
};
