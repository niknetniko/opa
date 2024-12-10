/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "editor_dialog.h"

namespace Ui {
    class EventEditorForm;
}

class QAbstractItemModel;
class QDataWidgetMapper;

class EventEditorDialog : public AbstractEditorDialog {
    Q_OBJECT

public:
    explicit EventEditorDialog(
        QAbstractItemModel* eventRelationModel, QAbstractItemModel* eventModel, bool newEvent, QWidget* parent
    );

    static void
    showDialogForNewEvent(QAbstractItemModel* eventRelationModel, QAbstractItemModel* eventModel, QWidget* parent);
    static void
    showDialogForExistingEvent(QAbstractItemModel* eventRelationModel, QAbstractItemModel* eventModel, QWidget* parent);

public Q_SLOTS:
    void editDateWithEditor();
    void editNoteWithEditor();

private:
    Ui::EventEditorForm* form;
    bool newEvent;
};
