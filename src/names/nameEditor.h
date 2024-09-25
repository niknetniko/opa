//
// Created by niko on 8/02/24.
//

#ifndef OPA_NAMEEDITOR_H
#define OPA_NAMEEDITOR_H

#include <QDataWidgetMapper>
#include <QDialog>
#include <QStyledItemDelegate>
#include "names.h"

/**
 * Editor for names.
 */
class NamesEditor : public QDialog {
Q_OBJECT
public:
    explicit NamesEditor(QAbstractItemModel *model, bool newRow, QWidget *parent);

    void accept() override;
    void reject() override;

private:
    QAbstractItemModel *model;
    QDataWidgetMapper *mapper;
    bool newRow;
};

#endif //OPA_NAMEEDITOR_H
