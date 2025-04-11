/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "source_editor_dialog.h"

#include "data/source.h"
#include "note_editor_dialog.h"
#include "ui_source_editor_dialog.h"
#include "utils/custom_sql_relational_model.h"
#include "utils/formatted_identifier_delegate.h"

#include <KLocalizedString>
#include <QDataWidgetMapper>

SourceEditorDialog::SourceEditorDialog(QAbstractItemModel* sourceModel, bool newSource, QWidget* parent) :
    AbstractEditorDialog(newSource, parent),
    form(new Ui::SourceEditorForm),
    newSource(newSource) {
    form->setupUi(this);

    connect(form->noteEditButton, &QPushButton::clicked, this, &SourceEditorDialog::editNoteWithEditor);
    connect(form->sourceParentAdd, &QPushButton::clicked, this, &SourceEditorDialog::addNewSourceAsParent);
    connect(
        form->sourceParentPickButton, &QPushButton::clicked, this, &SourceEditorDialog::selectExistingSourceAsParent
    );

    connectComboBox(sourceModel, SourcesTableModel::TYPE, form->sourceTypeComboBox);
    connectComboBox(sourceModel, SourcesTableModel::CONFIDENCE, form->sourceConfidenceCombobox);

    if (newSource) {
        this->setWindowTitle(i18n("Add new source"));
    } else {
        auto nameId =
            format_id(FormattedIdentifierDelegate::SOURCE, sourceModel->index(0, SourcesTableModel::ID).data());
        this->setWindowTitle(i18n("Editing %1", nameId));
    }

    form->noteEdit->enableRichTextMode();

    auto* sourceMapper = new QDataWidgetMapper(this);
    sourceMapper->setModel(sourceModel);
    sourceMapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
    sourceMapper->addMapping(form->sourcetTitleEdit, SourcesTableModel::TITLE);
    sourceMapper->addMapping(form->sourceAuthorEdit, SourcesTableModel::AUTHOR);
    sourceMapper->addMapping(form->sourcePublicationEdit, SourcesTableModel::PUBLICATION);
    sourceMapper->addMapping(form->sourceTypeComboBox, SourcesTableModel::TYPE);
    sourceMapper->addMapping(form->sourceConfidenceCombobox, SourcesTableModel::CONFIDENCE);
    sourceMapper->addMapping(form->noteEdit, SourcesTableModel::NOTE);
    sourceMapper->toFirst();
    addMapper(sourceMapper);
}

void SourceEditorDialog::showDialogForNewSource(QAbstractItemModel* sourceModel, QWidget* parent) {
    auto* dialog = new SourceEditorDialog(sourceModel, true, parent);
    dialog->show();
}

void SourceEditorDialog::showDialogForExistingSource(QAbstractItemModel* sourceModel, QWidget* parent) {
    auto* dialog = new SourceEditorDialog(sourceModel, false, parent);
    dialog->show();
}

void SourceEditorDialog::editNoteWithEditor() {
    auto currentText = form->noteEdit->textOrHtml();
    if (auto note = NoteEditorDialog::editText(currentText, i18n("Edit note"), this); !note.isEmpty()) {
        form->noteEdit->setTextOrHtml(note);
    }
}

void SourceEditorDialog::addNewSourceAsParent() {
}
void SourceEditorDialog::selectExistingSourceAsParent() {
}
