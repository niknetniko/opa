/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "new_person_editor_dialog.h"

#include "../domain/name/name_origins_model.h"
#include "../domain/name/name_repository.h"
#include "../domain/person/person_repository.h"
#include "ui_new_person_editor_dialog.h"

#include <KLocalizedString>
#include <QCompleter>
#include <QStringListModel>

using namespace Qt::StringLiterals;

NewPersonEditorDialog::NewPersonEditorDialog(QWidget* parent) :
    AbstractEditorDialog(true, parent),
    form(new Ui::NewPersonEditorForm) {
    form->setupUi(this);

    // Insert a new person via the repository.
    PersonRepository personRepo;
    const auto insertedPersonId = personRepo.insertPerson(QString(), false);
    if (!insertedPersonId.has_value()) {
        qFatal() << "Could not insert new person for some reason";
        return;
    }
    newPersonId = *insertedPersonId;

    addedPersonId = newPersonId;

    // Insert a new name via the repository.
    NameRepository nameRepo;
    const auto insertedNameId = nameRepo.insertName(newPersonId, 1);
    if (!insertedNameId.has_value()) {
        qFatal() << "Could not insert new name for some reason";
        return;
    }
    newNameId = *insertedNameId;

    this->setWindowTitle(i18n("Add new person"));

    // Set up autocomplete on surname and given names using all names in the DB.
    // TODO: Reduce duplication between this and NameEditorDialog.
    // Issue URL: https://github.com/niknetniko/opa/issues/57
    auto* surnameModel = new QStringListModel(nameRepo.findAllSurnames(), this);
    auto* surnameCompleter = new QCompleter(surnameModel, this);
    surnameCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    surnameCompleter->setCompletionMode(QCompleter::PopupCompletion);
    form->surname->setCompleter(surnameCompleter);

    auto* givenNamesModel = new QStringListModel(nameRepo.findAllGivenNames(), this);
    auto* givenNameCompleter = new QCompleter(givenNamesModel, this);
    givenNameCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    givenNameCompleter->setCompletionMode(QCompleter::PopupCompletion);
    form->givenNames->setCompleter(givenNameCompleter);

    // Set up origin combo box from the name origins model.
    originsModel = new NameOriginsModel(this);
    form->origin->setModel(originsModel);
    form->origin->setModelColumn(NameOriginsModel::ORIGIN);
    form->origin->setEditable(true);
    form->origin->setCurrentIndex(-1);
    form->origin->clearEditText();

    auto* sexesModel = new QStringListModel({u"Male"_s, u"Female"_s, u"Unknown"_s}, this);
    form->sexComboBox->setModel(sexesModel);
}

NewPersonEditorDialog::~NewPersonEditorDialog() {
    delete form;
}

void NewPersonEditorDialog::accept() {
    // Save the sex field via the repository.
    PersonRepository personRepo;
    personRepo.updatePerson(newPersonId, form->sexComboBox->currentText(), false);

    // Resolve the origin_id from the selected combo box entry.
    IntegerPrimaryKey originId = -1;
    const int selectedRow = form->origin->currentIndex();
    if (selectedRow >= 0) {
        auto pkIndex = originsModel->index(selectedRow, NameOriginsModel::ID);
        originId = pkIndex.data().toLongLong();
    }

    // Save the name fields via the repository.
    NameRepository nameRepo;
    if (!nameRepo.updateName(
            newNameId,
            form->titles->text(),
            form->givenNames->text(),
            form->prefix->text(),
            form->surname->text(),
            QString(),
            originId
        )) {
        qWarning() << "NewPersonEditorDialog: could not save name" << newNameId;
        return;
    }

    QDialog::accept();
}

void NewPersonEditorDialog::reject() {
    addedPersonId = {};
    AbstractEditorDialog::reject();
}

void NewPersonEditorDialog::revert() {
    // Delete the person (CASCADE removes the name too).
    PersonRepository personRepo;
    personRepo.deletePerson(newPersonId);
    newPersonId = -1;
    newNameId = -1;
}
