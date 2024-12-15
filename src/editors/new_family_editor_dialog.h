/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include "database/schema.h"
#include "editor_dialog.h"

class QDataWidgetMapper;

namespace Ui {
    class NewFamilyEditorForm;
}

struct FamilyData {
    IntegerPrimaryKey childId;
    bool hasNewBirthEvent;
    QVariant birthEventId;
    QVariant motherId;
    QVariant fatherId;
};

class NewFamilyEditorDialog : public QDialog {
    Q_OBJECT

public:
    explicit NewFamilyEditorDialog(IntegerPrimaryKey personId, QWidget* parent);
    ~NewFamilyEditorDialog() override;

public Q_SLOTS:
    void accept() override;
    void reject() override;

    void onSelectExistingMother();
    void onSelectExistingFather();
    void onSelectNewMother();
    void onSelectNewFather();

private:
    Ui::NewFamilyEditorForm* form;

    FamilyData data;

    QDataWidgetMapper* motherIdMapper;
    QDataWidgetMapper* motherBirthMapper;
    QDataWidgetMapper* motherNameMapper;
    QDataWidgetMapper* motherRelationMapper;
    QDataWidgetMapper* fatherIdMapper;
    QDataWidgetMapper* fatherBirthMapper;
    QDataWidgetMapper* fatherNameMapper;
    QDataWidgetMapper* fatherRelationMapper;
    QDataWidgetMapper* parentRelationMapper;

    void setMother(const QVariant& motherId);
    void setFather(const QVariant& fatherId);
    void setParentRelationIfPossible() const;
};
