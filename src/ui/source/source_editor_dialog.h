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
    explicit SourceEditorDialog(std::optional<IntegerPrimaryKey> sourceId, QWidget* parent);

    static QVariant showDialogForNewSource(QWidget* parent);
    static void showDialogForExistingSource(IntegerPrimaryKey sourceId, QWidget* parent);

public Q_SLOTS:
    void accept() override;
    void editNoteWithEditor();
    void addNewSourceAsParent();
    void selectExistingSourceAsParent();

private:
    Ui::SourceEditorForm* form;
    std::optional<IntegerPrimaryKey> sourceId;
    std::optional<IntegerPrimaryKey> parentId;
    QAbstractItemModel* confidenceModel;

    void updateParentDisplay() const;
};
