/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "database/schema.h"
#include "domain/source/source_entities.h"
#include "utils/pending_list_model.h"

#include <QDialog>
#include <optional>

class CitationListWidget;
class EventTypesListModel;
class EventRolesListModel;
class LocationPathsModel;

namespace Ui {
class EventEditorForm;
}

class EventEditorDialog : public QDialog {
    Q_OBJECT

public:
    EventEditorDialog(IntegerPrimaryKey personId, QWidget* parent);
    EventEditorDialog(
        IntegerPrimaryKey eventId,
        IntegerPrimaryKey roleId,
        IntegerPrimaryKey relationId,
        IntegerPrimaryKey personId,
        QWidget* parent
    );

    static void showDialogForNewEvent(IntegerPrimaryKey personId, QWidget* parent);
    static void showDialogForExistingEvent(
        IntegerPrimaryKey eventId,
        IntegerPrimaryKey roleId,
        IntegerPrimaryKey relationId,
        IntegerPrimaryKey personId,
        QWidget* parent
    );

public Q_SLOTS:
    void accept() override;
    void reject() override;

    void editDateWithEditor();
    void editNoteWithEditor();

private:
    void setupUi();

    Ui::EventEditorForm* form;
    std::optional<IntegerPrimaryKey> eventId;
    IntegerPrimaryKey personId;
    IntegerPrimaryKey roleId = 0;
    std::optional<IntegerPrimaryKey> relationId;

    EventTypesListModel* typesModel;
    EventRolesListModel* rolesModel;
    LocationPathsModel* locationsModel;

    CitationListWidget* eventRelationCitationsWidget;
    CitationListWidget* eventCitationsWidget;

    PendingListModel<SourceEntity> eventCitationsPending;
    PendingListModel<SourceEntity> relationCitationsPending;
};
