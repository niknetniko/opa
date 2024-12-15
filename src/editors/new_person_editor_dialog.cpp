/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "new_person_editor_dialog.h"

#include "data/data_manager.h"
#include "data/names.h"
#include "data/person.h"
#include "ui_new_person_editor_dialog.h"

#include <QCompleter>
#include <QDataWidgetMapper>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>

NewPersonEditorDialog::NewPersonEditorDialog(QWidget* parent) :
    AbstractEditorDialog(true, parent),
    form(new Ui::NewPersonEditorForm) {
    form->setupUi(this);

    // Get a new person.
    auto* peopleModel = DataManager::get().peopleModel();
    auto newPersonRecord = peopleModel->record();
    newPersonRecord.setGenerated(PeopleTableModel::ID, false);
    newPersonRecord.setValue(PeopleTableModel::ROOT, false);
    if (!peopleModel->insertRecord(-1, newPersonRecord)) {
        qFatal() << "Could inserted new person for some reason";
        qWarning() << peopleModel->lastError();
        return;
    }

    addedPersonId = peopleModel->query().lastInsertId();
    if (!addedPersonId.isValid()) {
        qFatal() << "Could not get last inserted ID for some reason";
        qWarning() << peopleModel->lastError();
        return;
    }

    IntegerPrimaryKey newPersonId = addedPersonId.toLongLong();

    auto* namesModel = DataManager::get().namesModel();
    auto newNameRecord = namesModel->record();
    newNameRecord.setGenerated(NamesTableModel::ID, false);
    newNameRecord.setValue(NamesTableModel::PERSON_ID, newPersonId);
    newNameRecord.setValue(NamesTableModel::SORT, 1);
    if (!namesModel->insertRecord(-1, newNameRecord)) {
        qFatal() << "Could insert new name for some reason:";
        qWarning() << namesModel->lastError();
        return;
    }

    auto lastInsertedNameId = namesModel->query().lastInsertId();
    if (!lastInsertedNameId.isValid()) {
        qFatal() << "Could not get last inserted ID for some reason:";
        qWarning() << namesModel->lastError();
        return;
    }

    IntegerPrimaryKey newNameId = lastInsertedNameId.toLongLong();

    this->setWindowTitle(i18n("Add new person"));

    auto* singleNameModel = DataManager::get().singleNameModel(this, newNameId);
    auto* singlePersonModel = DataManager::get().singlePersonModel(this, newPersonId);

    // TODO: Reduce duplication between this and NameEditorDialog.
    // Issue URL: https://github.com/niknetniko/opa/issues/57
    auto* surnameCompleter = new QCompleter(namesModel, this);
    surnameCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    surnameCompleter->setCompletionColumn(NamesTableModel::SURNAME);
    surnameCompleter->setCompletionMode(QCompleter::PopupCompletion);
    form->surname->setCompleter(surnameCompleter);

    auto* givenNameCompleter = new QCompleter(namesModel, this);
    givenNameCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    givenNameCompleter->setCompletionColumn(NamesTableModel::GIVEN_NAMES);
    givenNameCompleter->setCompletionMode(QCompleter::PopupCompletion);
    form->givenNames->setCompleter(surnameCompleter);

    connectComboBox(singleNameModel, NamesTableModel::ORIGIN, form->origin);
    auto* nameMapper = new QDataWidgetMapper(this);
    nameMapper->setModel(singleNameModel);
    nameMapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
    nameMapper->addMapping(form->titles, NamesTableModel::TITLES);
    nameMapper->addMapping(form->givenNames, NamesTableModel::GIVEN_NAMES);
    nameMapper->addMapping(form->prefix, NamesTableModel::PREFIX);
    nameMapper->addMapping(form->surname, NamesTableModel::SURNAME);
    nameMapper->addMapping(form->origin, NamesTableModel::ORIGIN);
    nameMapper->setItemDelegate(new CustomSqlRelationalDelegate(this));
    nameMapper->toFirst();
    addMapper(nameMapper);

    auto* sexMapper = new QDataWidgetMapper(this);
    sexMapper->setModel(singlePersonModel);
    sexMapper->addMapping(form->sexComboBox, PeopleTableModel::SEX);
    sexMapper->toFirst();
    addMapper(sexMapper);

    // TODO: Support translatable dropdown values here
    // Issue URL: https://github.com/niknetniko/opa/issues/55
    auto* sexesModel = DataManager::get().sexesModel(this);
    form->sexComboBox->setModel(sexesModel);
}

NewPersonEditorDialog::~NewPersonEditorDialog() {
    delete form;
}

void NewPersonEditorDialog::reject() {
    addedPersonId = {};
    AbstractEditorDialog::reject();
}
