/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include <QStyledItemDelegate>

class KRichTextWidget;

/**
 * Displays "rich text" values in their plain text variant.
 */
class RichTextPlainDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    explicit RichTextPlainDelegate(QObject* parent = nullptr);

    [[nodiscard]] QString displayText(const QVariant& value, const QLocale& locale) const override;
};
