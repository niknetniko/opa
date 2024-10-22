#include "builtin_text_translating_delegate.h"

BuiltinTextTranslatingDelegate::BuiltinTextTranslatingDelegate(QObject *parent): QStyledItemDelegate(parent) {
}

void BuiltinTextTranslatingDelegate::setTranslator(const std::function<QString(QString)> &translator) {
    this->translator = translator;
}

QString BuiltinTextTranslatingDelegate::displayText(const QVariant &value, const QLocale &locale) const {
    return this->translator(QStyledItemDelegate::displayText(value, locale));
}
