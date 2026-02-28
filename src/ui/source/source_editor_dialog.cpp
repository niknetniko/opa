/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "source_editor_dialog.h"

#include "domain/source/source.h"
#include "domain/source/source_repository.h"
#include "editors/note_editor_dialog.h"
#include "link_existing/choose_existing_source_window.h"
#include "ui_source_editor_dialog.h"
#include "utils/formatted_identifier_delegate.h"
#include "utils/model_utils.h"

#include <KLocalizedString>
#include <QStringListModel>

SourceEditorDialog::SourceEditorDialog(std::optional<IntegerPrimaryKey> sourceId, QWidget* parent) :
    QDialog(parent),
    form(new Ui::SourceEditorForm),
    sourceId(sourceId),
    parentId(std::nullopt) {
    form->setupUi(this);

    connect(form->noteEditButton, &QPushButton::clicked, this, &SourceEditorDialog::editNoteWithEditor);
    connect(form->sourceParentAdd, &QPushButton::clicked, this, &SourceEditorDialog::addNewSourceAsParent);
    connect(
        form->sourceParentPickButton, &QPushButton::clicked, this, &SourceEditorDialog::selectExistingSourceAsParent
    );

    // Populate the type combo box from the repository.
    SourceRepository repo;
    auto* typeModel = new QStringListModel(repo.findAllTypes(), this);
    form->sourceTypeComboBox->setModel(typeModel);

    // Populate the confidence combo box from the enum.
    confidenceModel = getEnumModel<Confidence::Values>(this, Confidence::toDisplayString);
    form->sourceConfidenceCombobox->setModel(confidenceModel);

    form->noteEdit->enableRichTextMode();

    // Load and pre-fill the entity data.
    if (sourceId.has_value()) {
        if (const auto entity = repo.findById(*sourceId)) {
            form->sourcetTitleEdit->setText(entity->title);
            form->sourceAuthorEdit->setText(entity->author);
            form->sourcePublicationEdit->setText(entity->publication);
            form->noteEdit->setTextOrHtml(entity->note);
            form->sourceTypeComboBox->setCurrentText(entity->type);

            auto storedConfidence = enumFromString<Confidence::Values>(entity->confidence);
            const auto idx = form->sourceConfidenceCombobox->findData(qToUnderlying(storedConfidence), Qt::EditRole);
            if (idx >= 0) {
                form->sourceConfidenceCombobox->setCurrentIndex(idx);
            }

            parentId = entity->parentId;
            updateParentDisplay();
        }
    }

    if (!sourceId.has_value()) {
        this->setWindowTitle(i18n("Add new source"));
    } else {
        auto nameId = format_id(FormattedIdentifierDelegate::SOURCE, *sourceId);
        this->setWindowTitle(i18n("Editing %1", nameId));
    }
}

void SourceEditorDialog::accept() {
    SourceRepository repo;

    auto title = form->sourcetTitleEdit->text();
    auto type = form->sourceTypeComboBox->currentText();
    auto author = form->sourceAuthorEdit->text();
    auto publication = form->sourcePublicationEdit->text();
    auto note = form->noteEdit->textOrHtml();

    auto confidenceIdx = form->sourceConfidenceCombobox->currentIndex();
    auto confidenceInt = confidenceModel->data(confidenceModel->index(confidenceIdx, 0), Qt::EditRole).toInt();
    auto confidence = enumToString(static_cast<Confidence::Values>(confidenceInt));

    if (!sourceId.has_value()) {
        const auto newId = repo.insert(title, type, author, publication, confidence, note, parentId);
        if (!newId) {
            qWarning() << "Could not insert new source";
            return;
        }
        sourceId = newId;
    } else if (!repo.update(*sourceId, title, type, author, publication, confidence, note, parentId)) {
        qWarning() << "Could not save source" << *sourceId;
        return;
    }

    QDialog::accept();
}

QVariant SourceEditorDialog::showDialogForNewSource(QWidget* parent) {
    auto* dialog = new SourceEditorDialog(std::nullopt, parent);
    if (dialog->exec() != Accepted || !dialog->sourceId) {
        return {};
    }
    return QVariant::fromValue(*dialog->sourceId);
}

void SourceEditorDialog::showDialogForExistingSource(IntegerPrimaryKey sourceId, QWidget* parent) {
    auto* dialog = new SourceEditorDialog(sourceId, parent);
    dialog->show();
}

void SourceEditorDialog::editNoteWithEditor() {
    auto currentText = form->noteEdit->textOrHtml();
    if (auto note = NoteEditorDialog::editText(currentText, i18n("Edit note"), this); !note.isEmpty()) {
        form->noteEdit->setTextOrHtml(note);
    }
}

void SourceEditorDialog::addNewSourceAsParent() {
    qDebug() << "Adding new source as parent...";
    auto newSourceId = showDialogForNewSource(this);
    qDebug() << "New source ID:" << newSourceId;
    if (newSourceId.isValid()) {
        parentId = newSourceId.toLongLong();
        updateParentDisplay();
        qDebug() << "Selected new source ID:" << newSourceId;
    } else {
        qDebug() << "Aborting adding new source as parent.";
    }
}

void SourceEditorDialog::selectExistingSourceAsParent() {
    auto parentSource = ChooseExistingSourceWindow::selectSource(this);
    if (parentSource.isValid()) {
        parentId = parentSource.toLongLong();
        updateParentDisplay();
        qDebug() << "Selected source ID:" << parentSource;
    }
}

void SourceEditorDialog::updateParentDisplay() const {
    if (parentId.has_value()) {
        form->sourceParentDisplay->setText(format_id(FormattedIdentifierDelegate::SOURCE, parentId.value()));
    } else {
        form->sourceParentDisplay->setText(i18n("No parent selected"));
    }
}
