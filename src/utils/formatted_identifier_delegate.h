//
// Created by niko on 25/09/24.
//

#ifndef OPA_FORMATTED_IDENTIFIER_DELEGATE_H
#define OPA_FORMATTED_IDENTIFIER_DELEGATE_H

#include <QStyledItemDelegate>

QString format_id(const QString &pattern, const QVariant &id);

class FormattedIdentifierDelegate : public QStyledItemDelegate {
Q_OBJECT

public:
    inline static const auto PERSON = QStringLiteral("P%1");
    inline static const auto NAME = QStringLiteral("N%1");
    inline static const auto EVENT = QStringLiteral("E%1");

    FormattedIdentifierDelegate(QObject *parent, const QString &pattern);

    QString displayText(const QVariant &value, const QLocale &locale) const override;

private:
    QString pattern;

};

#endif //OPA_FORMATTED_IDENTIFIER_DELEGATE_H
