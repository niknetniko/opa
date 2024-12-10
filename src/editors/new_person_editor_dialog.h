/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

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
    explicit NewPersonEditorDialog(QWidget* parent);

public Q_SLOTS:
    void revert() override;

private:
    static constexpr int NAME_MAPPER = 0;
    static constexpr int PERSON_MAPPER = 1;

    Ui::NewPersonEditorForm* form;
    IntegerPrimaryKey newPersonId = -1;
    IntegerPrimaryKey newNameId = -1;
};
