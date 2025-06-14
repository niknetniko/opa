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
    if (!value.isValid()) {
        return QStyledItemDelegate::displayText(value, locale);
    }

    return QTextDocumentFragment::fromHtml(value.toString()).toPlainText().section(QLatin1Char('\n'), 0, 0);
}
