/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "./name_editor_dialog.h"

#include "../../domain/name/name_repository.h"
#include "../../domain/name/names.h"
#include "../../editors/note_editor_dialog.h"
#include "ui_name_editor_dialog.h"
#include "domain/name/name_origins_model.h"
#include "utils/formatted_identifier_delegate.h"

#include <KLocalizedString>
#include <QCompleter>
#include <QString>
#include <QStringListModel>

NamesEditorDialog::NamesEditorDialog(IntegerPrimaryKey nameId, bool newRow, QWidget* parent) :
    AbstractEditorDialog(newRow, parent),
    form(new Ui::NameEditorForm()),
    nameId(nameId) {
    form->setupUi(this);

    NameRepository nameRepo;
    const auto entity = nameRepo.findById(nameId);

    if (newRow) {
        this->setWindowTitle(i18n("Nieuwe naam toevoegen"));
    } else {
        auto nameIdFormatted = format_id(FormattedIdentifierDelegate::NAME, nameId);
        this->setWindowTitle(i18n("%1 bewerken", nameIdFormatted));
    }

    // Populate origin combo box from the name origins model.
    auto* nameOriginsModel = new NameOriginsModel(this);
    form->origin->setModel(nameOriginsModel);
    form->origin->setModelColumn(0);
    form->origin->setEditable(true);
    form->origin->setCurrentText(entity->origin);

    // Set up autocomplete on surname and given names using all names in the DB.
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

    form->noteEdit->enableRichTextMode();
    connect(form->noteEditButton, &QPushButton::clicked, this, &NamesEditorDialog::editNoteWithEditor);
}

void NamesEditorDialog::accept() {
    NameRepository nameRepo;
    const bool ok = nameRepo.updateName(
        nameId,
        form->titles->text(),
        form->givenNames->text(),
        form->prefix->text(),
        form->surname->text(),
        form->noteEdit->textOrHtml(),
        form->origin->currentText()
    );

    if (!ok) {
        qWarning() << "NamesEditorDialog: could not save name" << nameId;
        return;
    }

    AbstractEditorDialog::accept();
}

void NamesEditorDialog::revert() {
    if (isNewItem) {
        NameRepository nameRepo;
        if (const bool ok = nameRepo.deleteName(nameId); !ok) {
            qWarning() << "NamesEditorDialog: could not revert name" << nameId;
            return;
        }
        nameId = -1;
    }
}

void NamesEditorDialog::editNoteWithEditor() {
    const auto currentText = form->noteEdit->textOrHtml();

    if (const auto note = NoteEditorDialog::editText(currentText, i18n("Edit note"), this); !note.isEmpty()) {
        form->noteEdit->setTextOrHtml(note);
    }
}

NamesEditorDialog::~NamesEditorDialog() {
    delete form;
}

void NamesEditorDialog::showDialogForNewName(IntegerPrimaryKey nameId, QWidget* parent) {
    auto* editorWindow = new NamesEditorDialog(nameId, true, parent);
    editorWindow->show();
}

void NamesEditorDialog::showDialogForExistingName(IntegerPrimaryKey nameId, QWidget* parent) {
    auto* editorWindow = new NamesEditorDialog(nameId, false, parent);
    editorWindow->show();
}
