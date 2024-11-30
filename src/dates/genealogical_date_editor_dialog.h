/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include "genealogical_date.h"

#include <QDialog>

namespace Ui {
    class GenealogicalDateEditorDialog;
}

class GenealogicalDate;
/**
 * Window showing an editor for a GenealogicalDate (or asking for a new one).
 */
class GenealogicalDateEditorDialog : public QDialog {
    Q_OBJECT

public:
    explicit GenealogicalDateEditorDialog(const GenealogicalDate& existingDate, QWidget* parent);
    ~GenealogicalDateEditorDialog() override;

    static GenealogicalDate editDate(const GenealogicalDate& existingDate = {}, QWidget* parent = nullptr);

public Q_SLOTS:
    void accept() override;
    void reject() override;

private Q_SLOTS:
    void updateFormsFromDate() const;
    void updateDateFromForms();
    void textualUpdated();

private:
    GenealogicalDate date;
    Ui::GenealogicalDateEditorDialog* ui;
    QString defaultLineEditStyle;
};
