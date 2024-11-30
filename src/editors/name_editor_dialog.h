/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include <QDialog>

namespace Ui {
    class NameEditorForm;
}

class QAbstractProxyModel;
class QDataWidgetMapper;

/**
 * Editor for names.
 */
class NamesEditorDialog : public QDialog {
    Q_OBJECT

public:
    explicit NamesEditorDialog(QAbstractProxyModel* model, bool newRow, QWidget* parent);

    ~NamesEditorDialog() override;

    static void showDialogForNewName(QAbstractProxyModel* model, QWidget* parent);
    static void showDialogForExistingName(QAbstractProxyModel* model, QWidget* parent);

public Q_SLOTS:
    void accept() override;
    void reject() override;

    void editNoteWithEditor();

private:
    QAbstractProxyModel* model;
    QDataWidgetMapper* mapper;

    bool newRow;

    Ui::NameEditorForm* form;
};
