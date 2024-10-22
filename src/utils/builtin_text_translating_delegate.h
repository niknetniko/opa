//
// Created by niko on 22/10/24.
//

#ifndef BUILTIN_TEXT_TRANSLATING_DELEGATE_H
#define BUILTIN_TEXT_TRANSLATING_DELEGATE_H

#include <QStyledItemDelegate>

class BuiltinTextTranslatingDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    explicit BuiltinTextTranslatingDelegate(QObject *parent = nullptr);

    void setTranslator(const std::function<QString(QString)> &translator);

    QString displayText(const QVariant &value, const QLocale &locale) const override;

private:
    std::function<QString(QString)> translator;
};

#endif //BUILTIN_TEXT_TRANSLATING_DELEGATE_H
