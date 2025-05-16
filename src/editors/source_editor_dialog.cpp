/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "source_editor_dialog.h"

#include "data/data_manager.h"
#include "data/person.h"
#include "data/source.h"
#include "link_existing/choose_existing_source_window.h"
#include "note_editor_dialog.h"
#include "ui_source_editor_dialog.h"
#include "utils/formatted_identifier_delegate.h"

#include <KLocalizedString>
#include <QDataWidgetMapper>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>

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

    // connectComboBox(sourceModel, SourcesTableModel::TYPE, form->sourceTypeComboBox);
    // connectComboBox(sourceModel, SourcesTableModel::CONFIDENCE, form->sourceConfidenceCombobox);

    auto* confidenceModel = getEnumModel<Confidence::Values>(this, Confidence::toDisplayString);
    form->sourceConfidenceCombobox->setModel(confidenceModel);

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
    // TODO: make this show the parent somehow after it has been selected.
    sourceMapper->addMapping(form->sourceParentDisplay, SourcesTableModel::PARENT_ID);
    sourceMapper->toFirst();
    addMapper(sourceMapper);

    // TODO: add auto complete for confidences.
}

void SourceEditorDialog::showDialogForNewSource(QWidget* parent) {
    auto* sourcesModel = DataManager::get().sourcesModel();
    auto newSourceRecord = sourcesModel->record();
    newSourceRecord.setGenerated(SourcesTableModel::ID, false);
    auto insertedConfidence = enumToString(Confidence::Values::Normal);
    newSourceRecord.setValue(SourcesTableModel::CONFIDENCE, insertedConfidence);

    if (!sourcesModel->insertRecord(-1, newSourceRecord)) {
        qFatal() << "Could insert new name for some reason:";
        qWarning() << sourcesModel->lastError();
        return;
    }

    auto lastInsertedSource = sourcesModel->query().lastInsertId();
    if (!lastInsertedSource.isValid()) {
        qFatal() << "Could not get last inserted ID for some reason:";
        qWarning() << sourcesModel->lastError();
        return;
    }

    auto* singleSourceModel = DataManager::get().singleSourceModel(parent, lastInsertedSource);
    auto* dialog = new SourceEditorDialog(singleSourceModel, true, parent);
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
    auto sourceId = ChooseExistingSourceWindow::selectSource(this);
    if (sourceId.isValid()) {
        auto model = mappers.first()->model();
        auto index = model->index(0, SourcesTableModel::PARENT_ID);
        model->setData(index, sourceId);
        qDebug() << "Selected source ID:" << sourceId;
    }
}
