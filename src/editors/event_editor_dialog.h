/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "database/schema.h"

#include <QDialog>

namespace Ui {
    class EventEditorForm;
}

class EventEditorDialog : public QDialog {
    Q_OBJECT

public:
    explicit EventEditorDialog(
        IntegerPrimaryKey eventId,
        IntegerPrimaryKey roleId,
        IntegerPrimaryKey personId,
        bool newEvent,
        QWidget* parent
    );

    static void showDialogForNewEvent(
        IntegerPrimaryKey eventId, IntegerPrimaryKey roleId, IntegerPrimaryKey personId, QWidget* parent
    );
    static void showDialogForExistingEvent(
        IntegerPrimaryKey eventId, IntegerPrimaryKey roleId, IntegerPrimaryKey personId, QWidget* parent
    );

public Q_SLOTS:
    void accept() override;
    void reject() override;

    void editDateWithEditor();
    void editNoteWithEditor();

private:
    Ui::EventEditorForm* form;
    bool newEvent;
    IntegerPrimaryKey eventId;
    IntegerPrimaryKey roleId;
    IntegerPrimaryKey personId;

    // The role ID that was active when the dialog was opened (used to detect changes).
    IntegerPrimaryKey originalRoleId;
};
