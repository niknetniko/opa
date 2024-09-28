//
// Created by niko on 8/02/24.
//

#ifndef OPA_NAME_EDITOR_H
#define OPA_NAME_EDITOR_H

#include <QDataWidgetMapper>
#include <QDialog>
#include <QStyledItemDelegate>
#include "names_table_view.h"

/**
 * Editor for names.
 */
class NamesEditor : public QDialog {
Q_OBJECT
public:
    explicit NamesEditor(QAbstractProxyModel *model, bool newRow, QWidget *parent);

    void accept() override;
    void reject() override;

private:
    QAbstractProxyModel *model;
    QDataWidgetMapper *mapper;
    bool newRow;
};

#endif //OPA_NAME_EDITOR_H
