/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include <QAbstractItemModel>
#include <QDataWidgetMapper>
#include <QDialog>

namespace Ui {
    class EventEditorForm;
}

class EventEditor : public QDialog {
    Q_OBJECT

public:
    explicit EventEditor(
        QAbstractItemModel* eventRelationModel, QAbstractItemModel* eventModel, bool newEvent, QWidget* parent
    );

    ~EventEditor() override;

public Q_SLOTS:
    void accept() override;
    void reject() override;

    void editDateWithEditor();
    void editNoteWithEditor();

private:
    QAbstractItemModel* eventRelationModel;
    QAbstractItemModel* eventModel;
    QDataWidgetMapper* eventMapper;
    QDataWidgetMapper* eventRelationMapper;
    bool newEvent;

    Ui::EventEditorForm* form;
};
