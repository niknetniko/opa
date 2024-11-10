/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "builtin_text_translating_delegate.h"

BuiltinTextTranslatingDelegate::BuiltinTextTranslatingDelegate(QObject* parent) : QStyledItemDelegate(parent) {
}

void BuiltinTextTranslatingDelegate::setTranslator(const std::function<QString(QString)>& translator) {
    this->translator = translator;
}

QString BuiltinTextTranslatingDelegate::displayText(const QVariant& value, const QLocale& locale) const {
    return this->translator(QStyledItemDelegate::displayText(value, locale));
}
