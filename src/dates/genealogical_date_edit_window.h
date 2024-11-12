/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include "genealogical_date.h"

#include <QDialog>

namespace Ui {
    class GenealogicalDateEditWindow;
}

class GenealogicalDate;
/**
 * Window showing an editor for a GenealogicalDate (or asking for a new one).
 */
class GenealogicalDateEditWindow : public QDialog {
    Q_OBJECT

public:
    explicit GenealogicalDateEditWindow(const GenealogicalDate& existingDate, QWidget* parent);
    ~GenealogicalDateEditWindow() override;

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
    Ui::GenealogicalDateEditWindow* ui;
    QString defaultLineEditStyle;
};
