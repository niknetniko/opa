//
// Created by niko on 25/09/24.
//

#ifndef OPA_FORMATTED_IDENTIFIER_DELEGATE_H
#define OPA_FORMATTED_IDENTIFIER_DELEGATE_H

#include <QStyledItemDelegate>

class FormattedIdentifierDelegate: public QStyledItemDelegate {
    Q_OBJECT
public:
    inline static const QString PERSON = QStringLiteral("P%1");
    inline static const QString NAME = QStringLiteral("N%1");

    using QStyledItemDelegate::QStyledItemDelegate;

    FormattedIdentifierDelegate(QString pattern): pattern(pattern) {
    }

    QString displayText(const QVariant &value, const QLocale &locale) const override;

private:
    QString pattern;

};

#endif //OPA_FORMATTED_IDENTIFIER_DELEGATE_H
