/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "editor_dialog.h"

namespace Ui {
    class SourceEditorForm;
}

class QAbstractItemModel;
class QDataWidgetMapper;

class SourceEditorDialog : public AbstractEditorDialog {
    Q_OBJECT

public:
    explicit SourceEditorDialog(QAbstractItemModel* sourceModel, bool newSource, QWidget* parent);

    static void showDialogForNewSource(QWidget* parent);
    static void showDialogForExistingSource(QAbstractItemModel* sourceModel, QWidget* parent);

public Q_SLOTS:
    void editNoteWithEditor();
    void addNewSourceAsParent();
    void selectExistingSourceAsParent();

private:
    Ui::SourceEditorForm* form;
    bool newSource;
};
