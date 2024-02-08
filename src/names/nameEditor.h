//
// Created by niko on 8/02/24.
//

#ifndef OPA_NAMEEDITOR_H
#define OPA_NAMEEDITOR_H

#include <QDataWidgetMapper>
#include <QDialog>

class NamesEditor : public QDialog {
Q_OBJECT
public:
    explicit NamesEditor(long long personId, QWidget *parent);

    void accept() override;

private:
    QDataWidgetMapper *mapper;
};

#endif //OPA_NAMEEDITOR_H
