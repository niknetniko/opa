/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "database/schema.h"

#include <QDialog>
#include <memory>
#include <optional>

namespace Ui {
class LocationEditorForm;
}

class LocationEditorDialog : public QDialog {
    Q_OBJECT

public:
    explicit LocationEditorDialog(std::optional<IntegerPrimaryKey> locationId, std::optional<IntegerPrimaryKey> initialParentId, QWidget* parent);
    ~LocationEditorDialog() override;

    static QVariant showDialogForNewLocation(std::optional<IntegerPrimaryKey> parentId, QWidget* parent);
    static void showDialogForExistingLocation(IntegerPrimaryKey locationId, QWidget* parent);

public Q_SLOTS:
    void accept() override;
    void editNoteWithEditor();
    void addNewLocationAsParent();
    void selectExistingLocationAsParent();
    void editDateStartWithEditor();
    void editDateEndWithEditor();

private:
    std::unique_ptr<Ui::LocationEditorForm> form;
    std::optional<IntegerPrimaryKey> locationId;
    std::optional<IntegerPrimaryKey> parentId;

    void updateParentDisplay() const;
};
