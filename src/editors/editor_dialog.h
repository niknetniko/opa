/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <QDialog>


class QDialogButtonBox;
class QDataWidgetMapper;


/**
 * Dialog for editing stuff with some common behaviour.
 */
class AbstractEditorDialog : public QDialog {
    Q_OBJECT

public:
    explicit AbstractEditorDialog(QWidget* parent = nullptr);
    ~AbstractEditorDialog() override = default;

public Q_SLOTS:
    void accept() override;
    void reject() override;

protected:
    QList<QDataWidgetMapper*> mappers;

    /**
     * Called when the user rejects the made changes.
     * The changes in the mappers are reverted by this model, but this method should revert any data added to the
     * database for this editor.
     */
    virtual void revert() = 0;
};
