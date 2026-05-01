/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "./name_editor_dialog.h"

#include "../../domain/name/name_origin_translation_repository.h"
#include "../../domain/name/name_origins_model.h"
#include "../../domain/name/name_repository.h"
#include "../../domain/name/names.h"
#include "../../editors/note_editor_dialog.h"
#include "ui_name_editor_dialog.h"
#include "utils/formatted_identifier_delegate.h"
#include "utils/translating_proxy_model.h"

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
    auto entity = nameRepo.findById(nameId);

    if (newRow) {
        this->setWindowTitle(i18n("Nieuwe naam toevoegen"));
    } else {
        auto nameIdFormatted = format_id(FormattedIdentifierDelegate::NAME, nameId);
        this->setWindowTitle(i18n("%1 bewerken", nameIdFormatted));
    }

    // Populate origin combo box from the name origins model.
    originsModel = new NameOriginsModel(this);
    auto* originsProxy = new TranslatingProxyModel(
        TypeTranslationResolver(
            [](IntegerPrimaryKey originId, const QString& locale) {
                return NameOriginTranslationRepository().findByTypeIdAndLocale(originId, locale);
            },
            NameOrigins::toDisplayString
        ),
        this
    );
    originsProxy->setSourceModel(originsModel);
    form->origin->setModel(originsProxy);
    form->origin->setModelColumn(NameOriginsModel::ORIGIN);
    form->origin->setEditable(true);

    // Populate form fields from the entity (empty if not found / new).
    if (entity.has_value()) {
        form->titles->setText(entity->titles);
        form->givenNames->setText(entity->givenNames);
        form->prefix->setText(entity->prefix);
        form->surname->setText(entity->surname);
        form->noteEdit->setTextOrHtml(entity->note);

        // Select the correct origin in the combo box.
        if (entity->originId.has_value()) {
            for (int row = 0; row < originsModel->rowCount(); ++row) {
                auto pkIndex = originsModel->index(row, NameOriginsModel::ID);
                if (pkIndex.data().toLongLong() == *entity->originId) {
                    form->origin->setCurrentIndex(row);
                    break;
                }
            }
        } else {
            form->origin->setCurrentIndex(-1);
            form->origin->clearEditText();
        }
    }

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
    // Resolve the origin_id from the selected combo box entry.
    IntegerPrimaryKey originId = -1;
    const int selectedRow = form->origin->currentIndex();
    if (selectedRow >= 0) {
        auto pkIndex = originsModel->index(selectedRow, NameOriginsModel::ID);
        originId = pkIndex.data().toLongLong();
    }

    NameRepository nameRepo;
    const bool ok = nameRepo.updateName(
        nameId,
        form->titles->text(),
        form->givenNames->text(),
        form->prefix->text(),
        form->surname->text(),
        form->noteEdit->textOrHtml(),
        originId
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
        nameRepo.deleteName(nameId);
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
