/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "event_editor.h"

#include "data/data_manager.h"
#include "data/event.h"
#include "ui_event_editor.h"
#include "ui_temp.h"
#include "utils/formatted_identifier_delegate.h"
#include "utils/model_utils_find_source_model_of_type.h"
#include "utils/proxy_enabled_relational_delegate.h"

#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

EventEditor::EventEditor(
    QAbstractItemModel* eventRelationModel, QAbstractItemModel* eventModel, bool newEvent, QWidget* parent
) :
    QDialog(parent),
    eventRelationModel(eventRelationModel),
    eventModel(eventModel),
    newEvent(newEvent),
    form(new Ui::EventEditorForm()) {
    form->setupUi(this);

    // Connect the buttons.
    connect(form->dialogButtons, &QDialogButtonBox::accepted, this, &QDialog::accept); // NOLINT(*-unused-return-value)
    connect(form->dialogButtons, &QDialogButtonBox::rejected, this, &QDialog::reject); // NOLINT(*-unused-return-value)

    connectComboBox(eventRelationModel, EventRelationsModel::ROLE, form->eventRoleComboBox);
    connectComboBox(eventModel, EventsModel::TYPE, form->eventTypeComboBox);

    if (newEvent) {
        this->setWindowTitle(i18n("Nieuwe gebeurtenis toevoegen"));
    } else {
        auto nameId = format_id(FormattedIdentifierDelegate::EVENT, eventModel->index(0, 0).data());
        this->setWindowTitle(i18n("%1 bewerken", nameId));
    }

    eventRelationMapper = new QDataWidgetMapper(this);
    eventRelationMapper->setModel(this->eventRelationModel);
    eventRelationMapper->addMapping(form->eventRoleComboBox, EventRelationsModel::ROLE);
    eventRelationMapper->setItemDelegate(new SuperSqlRelationalDelegate(this));
    eventRelationMapper->toFirst();

    // TODO: investigate why this does not work with manual submit.
    //  Possibly since the model uses auto-submit?
    eventMapper = new QDataWidgetMapper(this);
    eventMapper->setModel(this->eventModel);
    eventMapper->addMapping(form->eventDatePicker, EventsModel::DATE);
    eventMapper->addMapping(form->eventNameEdit, EventsModel::NAME);
    eventMapper->addMapping(form->eventTypeComboBox, EventsModel::TYPE);
    eventMapper->setItemDelegate(new SuperSqlRelationalDelegate(this));
    eventMapper->toFirst();
}

EventEditor::~EventEditor() {
    delete this->form;
}

void EventEditor::accept() {
    // Attempt to submit the mapper changes.
    if (this->eventMapper->submit() && this->eventRelationMapper->submit()) {
        QDialog::accept();
    } else {
        // Find the original model.
        const auto* eventSqlModel = findSourceModelOfType<QSqlQueryModel>(this->eventModel);
        assert(eventSqlModel != nullptr);
        const auto* relationSqlModel = findSourceModelOfType<QSqlQueryModel>(this->eventRelationModel);
        assert(relationSqlModel != nullptr);
        auto eventError = eventSqlModel->lastError();
        auto relationError = relationSqlModel->lastError();
        qWarning() << "Event error was:" << eventError.text();
        qWarning() << "Event relation error was:" << relationError.text();
        qDebug() << "Raw event error: " << eventError.text();
        qDebug() << "Raw event relation error: " << relationError.text();
        // TODO: how to show this error to the user somehow?
        // QMessageBox::critical(
        //     this,
        //     i18n("Fout bij opslaan"),
        //     i18n("The changes could not be saved for some reason:\n") + eventError.text() + relationError.text()
        // );
    }
}

void EventEditor::reject() {
    this->eventRelationModel->revert();
    this->eventModel->revert();
    if (this->newEvent) {
        if (!this->eventRelationModel->removeRow(this->eventRelationModel->rowCount() - 1)) {
            qWarning() << "Could not revert event relation model?";
        }
        if (!this->eventModel->removeRow(this->eventModel->rowCount() - 1)) {
            qWarning() << "Could not revert event model?";
        }
    }
    QDialog::reject();
}
