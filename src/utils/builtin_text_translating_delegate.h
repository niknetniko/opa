/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include <QStyledItemDelegate>

class BuiltinTextTranslatingDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    explicit BuiltinTextTranslatingDelegate(QObject* parent = nullptr);

    void setTranslator(const std::function<QString(QString)>& translator);

    [[nodiscard]] QString displayText(const QVariant& value, const QLocale& locale) const override;

private:
    std::function<QString(QString)> translator;
};
