/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include <QStyledItemDelegate>

QString format_id(const QString& pattern, const QVariant& id);

class FormattedIdentifierDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    inline static const auto PERSON = QStringLiteral("P%1");
    inline static const auto NAME = QStringLiteral("N%1");
    inline static const auto EVENT = QStringLiteral("E%1");

    FormattedIdentifierDelegate(QObject* parent, QString pattern);

    [[nodiscard]] QString displayText(const QVariant& value, const QLocale& locale) const override;

private:
    QString pattern;
};
