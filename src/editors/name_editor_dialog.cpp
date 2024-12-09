/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "name_editor_dialog.h"

#include "data/data_manager.h"
#include "data/names.h"
#include "note_editor_dialog.h"
#include "ui_name_editor_dialog.h"
#include "utils/formatted_identifier_delegate.h"
#include "utils/model_utils.h"

#include <KLocalizedString>
#include <QCompleter>
#include <QDataWidgetMapper>
#include <QDialog>
#include <QSqlError>
#include <QSqlQuery>
#include <QString>

NamesEditorDialog::NamesEditorDialog(QAbstractProxyModel* model, bool newRow, QWidget* parent) :
    QDialog(parent),
    model(model),
    newRow(newRow),
    form(new Ui::NameEditorForm()) {
    form->setupUi(this);

    // Connect the buttons.
    connect(form->dialogButtons, &QDialogButtonBox::accepted, this, &QDialog::accept); // NOLINT(*-unused-return-value)
    connect(form->dialogButtons, &QDialogButtonBox::rejected, this, &QDialog::reject); // NOLINT(*-unused-return-value)

    // Set up the name origin combobox.
    connectComboBox(model, NamesTableModel::ORIGIN, form->origin);

    // Get the ID of the current name, if not new.
    if (newRow) {
        this->setWindowTitle(i18n("Nieuwe naam toevoegen"));
    } else {
        auto nameId = format_id(FormattedIdentifierDelegate::NAME, model->index(0, 0).data());
        this->setWindowTitle(i18n("%1 bewerken", nameId));
    }

    auto* baseModel = DataManager::get().namesModel();

    // Set up autocomplete on the last name.
    // We want to sort this, so we need to create a new model.
    // Additionally, we want to use all last names, not just the once from the current person.
    auto* surnameCompleter = new QCompleter(baseModel);
    surnameCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    surnameCompleter->setCompletionColumn(NamesTableModel::SURNAME);
    surnameCompleter->setCompletionMode(QCompleter::PopupCompletion);
    form->surname->setCompleter(surnameCompleter);

    auto* givenNameCompleter = new QCompleter(baseModel);
    givenNameCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    givenNameCompleter->setCompletionColumn(NamesTableModel::GIVEN_NAMES);
    givenNameCompleter->setCompletionMode(QCompleter::PopupCompletion);
    form->givenNames->setCompleter(surnameCompleter);

    form->noteEdit->enableRichTextMode();
    connect(form->noteEditButton, &QPushButton::clicked, this, &NamesEditorDialog::editNoteWithEditor);

    // TODO: investigate why this does not work with manual submit.
    //  Possibly since the model uses auto-submit?
    //  This means cancel does not work.
    this->mapper = new QDataWidgetMapper(this);
    mapper->setModel(this->model);
    mapper->addMapping(form->titles, NamesTableModel::TITLES);
    mapper->addMapping(form->givenNames, NamesTableModel::GIVEN_NAMES);
    mapper->addMapping(form->prefix, NamesTableModel::PREFIX);
    mapper->addMapping(form->surname, NamesTableModel::SURNAME);
    mapper->addMapping(form->origin, NamesTableModel::ORIGIN);
    mapper->addMapping(form->noteEdit, NamesTableModel::NOTE);
    mapper->setItemDelegate(new CustomSqlRelationalDelegate(this));
    mapper->toFirst();
}

void NamesEditorDialog::accept() {
    // Attempt to submit the mapper changes.
    if (mapper->submit()) {
        // We are done.
        QDialog::accept();
    } else {
        // Find the original model.
        auto* sqlModel = findSourceModelOfType<QSqlQueryModel>(model);
        assert(sqlModel != nullptr);
        auto lastError = sqlModel->lastError();
        qWarning() << "Error was:" << lastError.text();
        qDebug() << "Native error code is " << lastError.nativeErrorCode();
        qDebug() << "Last query is " << sqlModel->query().lastQuery();
    }
}

void NamesEditorDialog::reject() {
    this->model->revert();
    if (this->newRow) {
        qDebug() << "Removing cancelled addition...";
        if (!this->model->removeRow(this->model->rowCount() - 1)) {
            qWarning() << "Could not revert name...";
        }
    }
    QDialog::reject();
}

void NamesEditorDialog::editNoteWithEditor() {
    const auto currentText = form->noteEdit->textOrHtml();

    if (const auto note = NoteEditorDialog::editText(currentText, i18n("Edit note"), this); !note.isEmpty()) {
        form->noteEdit->setTextOrHtml(note);
    }
}

NamesEditorDialog::~NamesEditorDialog() {
    delete this->form;
}

void NamesEditorDialog::showDialogForNewName(QAbstractProxyModel* model, QWidget* parent) {
    auto* editorWindow = new NamesEditorDialog(model, true, parent);
    editorWindow->show();
}

void NamesEditorDialog::showDialogForExistingName(QAbstractProxyModel* model, QWidget* parent) {
    auto* editorWindow = new NamesEditorDialog(model, false, parent);
    editorWindow->show();
}
