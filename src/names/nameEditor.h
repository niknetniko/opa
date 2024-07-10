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
    explicit NamesEditor(NamesTableModel *model, int selectedRow, QWidget *parent);

    void accept() override;
    void reject() override;

private:
    NamesTableModel *model;
    QDataWidgetMapper *mapper;
};

#endif //OPA_NAMEEDITOR_H
