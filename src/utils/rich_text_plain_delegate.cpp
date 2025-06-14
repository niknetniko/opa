/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "rich_text_plain_delegate.h"

#include <QTextDocumentFragment>

RichTextPlainDelegate::RichTextPlainDelegate(QObject* parent) : QStyledItemDelegate(parent) {
}

QString RichTextPlainDelegate::displayText(const QVariant& value, const QLocale& locale) const {
    QVariant finalValue = value;

    if (!finalValue.isValid()) {
        return QStyledItemDelegate::displayText(value, locale);
    }

    return QTextDocumentFragment::fromHtml(finalValue.toString()).toPlainText().section(QLatin1Char('\n'), 0, 0);
}
