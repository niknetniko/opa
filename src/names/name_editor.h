/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "names_overview_view.h"
#include "ui_name_editor.h"

#include <QAbstractProxyModel>
#include <QDataWidgetMapper>
#include <QDialog>

/**
 * Editor for names.
 */
class NamesEditor : public QDialog {
    Q_OBJECT

public:
    explicit NamesEditor(QAbstractProxyModel* model, bool newRow, QWidget* parent);

    ~NamesEditor() override;

public Q_SLOTS:
    void accept() override;

    void reject() override;

private:
    QAbstractProxyModel* model;
    QDataWidgetMapper* mapper;
    bool newRow;
    Ui::NameEditorForm* form;
};
