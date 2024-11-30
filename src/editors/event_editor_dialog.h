/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include <QDialog>

namespace Ui {
    class EventEditorForm;
}

class QAbstractItemModel;
class QDataWidgetMapper;


class EventEditorDialog : public QDialog {
    Q_OBJECT

public:
    explicit EventEditorDialog(
        QAbstractItemModel* eventRelationModel, QAbstractItemModel* eventModel, bool newEvent, QWidget* parent
    );

    ~EventEditorDialog() override;

    static void
    showDialogForNewEvent(QAbstractItemModel* eventRelationModel, QAbstractItemModel* eventModel, QWidget* parent);
    static void
    showDialogForExistingEvent(QAbstractItemModel* eventRelationModel, QAbstractItemModel* eventModel, QWidget* parent);

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
