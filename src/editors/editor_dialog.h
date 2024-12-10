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
 * Common editor behaviour with special support for reversing changes if rejected.
 *
 * Notable, the class requires each mapper to have a model with a single row.
 */
class AbstractEditorDialog : public QDialog {
    Q_OBJECT

public:
    explicit AbstractEditorDialog(bool isNewItem, QWidget* parent = nullptr);
    ~AbstractEditorDialog() override = default;

public Q_SLOTS:
    void accept() override;
    void reject() override;

protected:
    QList<QDataWidgetMapper*> mappers;
    bool isNewItem;

    /**
     * Called when the user rejects the made changes.
     * The changes in the mappers are reverted by this model, but this method should revert any data added to the
     * database for this editor.
     */
    virtual void revert();

    /**
     * Add a mapper to the editor.
     */
    void addMapper(QDataWidgetMapper* mapper);
};
