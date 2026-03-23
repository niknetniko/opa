/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "../../domain/name/name_origins_model.h"
#include "../../editors/editor_dialog.h"
#include "database/schema.h"

#include <QDialog>

namespace Ui {
class NameEditorForm;
}

/**
 * Editor for names.
 *
 * The caller must insert a name row (via NameRepository::insertName) before opening this dialog.
 * On accept the dialog persists all form fields via NameRepository::updateName.
 * On reject when isNew is true, the dialog deletes the just-created name row.
 */
class NamesEditorDialog : public AbstractEditorDialog {
    Q_OBJECT

public:
    explicit NamesEditorDialog(IntegerPrimaryKey nameId, bool newRow, QWidget* parent);

    ~NamesEditorDialog() override;

    static void showDialogForNewName(IntegerPrimaryKey nameId, QWidget* parent);
    static void showDialogForExistingName(IntegerPrimaryKey nameId, QWidget* parent);

public Q_SLOTS:
    void accept() override;
    void editNoteWithEditor();

protected:
    void revert() override;

private:
    Ui::NameEditorForm* form;
    IntegerPrimaryKey nameId;
    NameOriginsModel* originsModel = nullptr;
};
