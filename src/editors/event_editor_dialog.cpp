/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "event_editor_dialog.h"

#include "data/data_manager.h"
#include "data/event.h"
#include "dates/genealogical_date.h"
#include "dates/genealogical_date_editor_dialog.h"
#include "note_editor_dialog.h"
#include "ui_event_editor_dialog.h"
#include "utils/formatted_identifier_delegate.h"
#include "utils/model_utils_find_source_model_of_type.h"
#include "utils/proxy_enabled_relational_delegate.h"

#include <QDataWidgetMapper>
#include <QSqlError>

EventEditorDialog::EventEditorDialog(
    QAbstractItemModel* eventRelationModel, QAbstractItemModel* eventModel, bool newEvent, QWidget* parent
) :
    QDialog(parent),
    eventRelationModel(eventRelationModel),
    eventModel(eventModel),
    newEvent(newEvent),
    form(new Ui::EventEditorForm) {
    form->setupUi(this);

    // Connect the buttons.
    connect(form->dialogButtons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(form->dialogButtons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(form->eventDateEditButton, &QPushButton::clicked, this, &EventEditorDialog::editDateWithEditor);
    connect(form->noteEditButton, &QPushButton::clicked, this, &EventEditorDialog::editNoteWithEditor);

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

    form->noteEdit->enableRichTextMode();

    // TODO: investigate why this does not work with manual submit.
    //  Possibly since the model uses auto-submit?
    eventMapper = new QDataWidgetMapper(this);
    eventMapper->setModel(this->eventModel);
    eventMapper->addMapping(form->eventDatePicker, EventsModel::DATE);
    eventMapper->addMapping(form->eventNameEdit, EventsModel::NAME);
    eventMapper->addMapping(form->eventTypeComboBox, EventsModel::TYPE);
    eventMapper->addMapping(form->noteEdit, EventsModel::NOTE);
    eventMapper->setItemDelegate(new SuperSqlRelationalDelegate(this));
    eventMapper->toFirst();
}

EventEditorDialog::~EventEditorDialog() {
    delete this->form;
}

void EventEditorDialog::showDialogForNewEvent(
    QAbstractItemModel* eventRelationModel, QAbstractItemModel* eventModel, QWidget* parent
) {
    auto* dialog = new EventEditorDialog(eventRelationModel, eventModel, true, parent);
    dialog->show();
}

void EventEditorDialog::showDialogForExistingEvent(
    QAbstractItemModel* eventRelationModel, QAbstractItemModel* eventModel, QWidget* parent
) {
    auto* dialog = new EventEditorDialog(eventRelationModel, eventModel, false, parent);
    dialog->show();
}

void EventEditorDialog::accept() {
    // Attempt to submit the mapper changes.
    bool eventSubmit = eventMapper->submit();
    bool relationSubmit = eventRelationMapper->submit();
    if (eventSubmit && relationSubmit) {
        QDialog::accept();
    } else {
        qDebug() << "Event submit" << eventSubmit << ", relation submit" << relationSubmit;
        // Find the original model.
        auto* eventSqlModel = findSourceModelOfType<QSqlQueryModel>(this->eventModel);
        assert(eventSqlModel != nullptr);
        auto* relationSqlModel = findSourceModelOfType<QSqlQueryModel>(this->eventRelationModel);
        assert(relationSqlModel != nullptr);
        auto eventError = eventSqlModel->lastError();
        auto relationError = relationSqlModel->lastError();
        qWarning() << "Event error was:" << eventError.text();
        qWarning() << "Event relation error was:" << relationError.text();
        qDebug() << "Raw event error: " << eventError.text();
        qDebug() << "Raw event relation error: " << relationError.text();
        // TODO: how to show this error to the user somehow?
    }
}

void EventEditorDialog::reject() {
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

void EventEditorDialog::editDateWithEditor() {
    auto currentText = form->eventDatePicker->text();
    auto startDate = GenealogicalDate::fromDisplayText(currentText);

    if (auto date = GenealogicalDateEditorDialog::editDate(startDate, this); date.isValid()) {
        form->eventDatePicker->setText(date.toDisplayText());
    }
}

void EventEditorDialog::editNoteWithEditor() {
    auto currentText = form->noteEdit->textOrHtml();

    if (auto note = NoteEditorDialog::editText(currentText, i18n("Edit note"), this); !note.isEmpty()) {
        form->noteEdit->setTextOrHtml(note);
    }
}
