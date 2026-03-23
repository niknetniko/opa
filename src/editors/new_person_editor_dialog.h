/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "../domain/name/name_origins_model.h"
#include "database/schema.h"
#include "editor_dialog.h"

namespace Ui {
class NewPersonEditorForm;
}

/**
 * Show an editor for a new person. This editor will also show an editor for the name of the person,
 * since we do not want people without names.
 */
class NewPersonEditorDialog : public AbstractEditorDialog {
    Q_OBJECT

public:
    QVariant addedPersonId;

    explicit NewPersonEditorDialog(QWidget* parent);
    ~NewPersonEditorDialog() override;

public Q_SLOTS:
    void accept() override;
    void reject() override;

protected:
    void revert() override;

private:
    Ui::NewPersonEditorForm* form;
    IntegerPrimaryKey newPersonId = -1;
    IntegerPrimaryKey newNameId = -1;
    NameOriginsModel* originsModel = nullptr;
};
