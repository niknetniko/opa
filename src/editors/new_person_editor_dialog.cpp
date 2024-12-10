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
    AbstractEditorDialog(parent),
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

    auto lastInsertedPeopleId = peopleModel->query().lastInsertId();
    if (!lastInsertedPeopleId.isValid()) {
        qFatal() << "Could not get last inserted ID for some reason";
        qWarning() << peopleModel->lastError();
        return;
    }

    newPersonId = lastInsertedPeopleId.toLongLong();

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

    newNameId = lastInsertedNameId.toLongLong();

    this->setWindowTitle(i18n("Add new person"));

    auto* singleNameModel = DataManager::get().singleNameModel(this, newNameId);
    auto* singlePersonModel = DataManager::get().singlePersonModel(this, newPersonId);

    // TODO: Reduce duplication between this and NameEditorDialog.
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
    mappers.append(nameMapper);

    auto* sexMapper = new QDataWidgetMapper(this);
    sexMapper->setModel(singlePersonModel);
    sexMapper->addMapping(form->sexComboBox, PeopleTableModel::SEX);
    sexMapper->toFirst();
    mappers.append(sexMapper);

    // TODO: Support translatable dropdown values here
    auto* sexesModel = DataManager::get().sexesModel(this);
    form->sexComboBox->setModel(sexesModel);
}

void NewPersonEditorDialog::revert() {
    if (newNameId != -1) {
        auto* nameMapper = mappers[NAME_MAPPER];
        if (!nameMapper->model()->removeRow(0)) {
            qWarning() << "Could not remove newly inserted name row?";
            if (auto* sourceModel = findSourceModelOfType<QSqlQueryModel>(nameMapper->model())) {
                qDebug() << sourceModel->lastError();
            }
        }
    }
    if (newPersonId != -1) {
        auto* personMapper = mappers[PERSON_MAPPER];
        if (!personMapper->model()->removeRow(0)) {
            qWarning() << "Could not remove newly inserted person row?";
            if (auto* sourceModel = findSourceModelOfType<QSqlQueryModel>(personMapper->model())) {
                qDebug() << sourceModel->lastError();
            }
        }
    }
}
