/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "database/schema.h"
#include "domain/source/source_entities.h"

#include <QDialog>
#include <optional>

namespace Ui {
    class SourceEditorForm;
}

class QAbstractItemModel;

class SourceEditorDialog : public QDialog {
    Q_OBJECT

public:
    explicit SourceEditorDialog(IntegerPrimaryKey sourceId, bool newSource, QWidget* parent);

    static QVariant showDialogForNewSource(QWidget* parent);
    static void showDialogForExistingSource(IntegerPrimaryKey sourceId, QWidget* parent);

public Q_SLOTS:
    void accept() override;
    void reject() override;
    void editNoteWithEditor();
    void addNewSourceAsParent();
    void selectExistingSourceAsParent();

private:
    Ui::SourceEditorForm* form;
    IntegerPrimaryKey sourceId;
    bool newSource;
    std::optional<IntegerPrimaryKey> m_parentId;
    QAbstractItemModel* confidenceModel;

    void updateParentDisplay();
};
