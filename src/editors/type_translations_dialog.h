/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "database/schema.h"

#include <KLanguageButton>
#include <QDialog>
#include <functional>
#include <optional>

namespace Ui {
class TypeTranslationsDialog;
}

class QStandardItem;
class QStandardItemModel;

/**
 * Dialog for managing per-locale name translations for a single user-defined type.
 *
 * Parameterised via callbacks so it works for both event types and location types.
 */
class TypeTranslationsDialog : public QDialog {
    Q_OBJECT

public:
    struct TranslationEntry {
        IntegerPrimaryKey id;
        QString locale;
        QString name;
    };

    TypeTranslationsDialog(
        const QString& typeName,
        std::function<QList<TranslationEntry>()> loadFn,
        std::function<std::optional<IntegerPrimaryKey>(const QString& locale, const QString& name)> insertFn,
        std::function<bool(IntegerPrimaryKey id)> removeFn,
        QWidget* parent = nullptr
    );

    ~TypeTranslationsDialog() override;

private Q_SLOTS:
    void addTranslation();
    void removeTranslation();
    void onItemChanged(QStandardItem* item);

private:
    void reload();
    [[nodiscard]] static QString formatLocale(const QString& localeCode);

    Ui::TypeTranslationsDialog* ui;
    QStandardItemModel* tableModel;
    KLanguageButton* languageButton;

    std::function<QList<TranslationEntry>()> loadFn;
    std::function<std::optional<IntegerPrimaryKey>(const QString& locale, const QString& name)> insertFn;
    std::function<bool(IntegerPrimaryKey id)> removeFn;
};
