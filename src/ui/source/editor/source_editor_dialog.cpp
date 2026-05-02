/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "source_editor_dialog.h"

#include "ai/ai_service.h"
#include "domain/media/media_repository.h"
#include "domain/source/source.h"
#include "domain/source/source_repository.h"
#include "domain/source/source_type_translation_repository.h"
#include "domain/source/source_types.h"
#include "domain/source/source_types_list_model.h"
#include "editors/note_editor_dialog.h"
#include "link_existing/choose_existing_source_window.h"
#include "opaSettings.h"
#include "ui/media/media_list_widget.h"
#include "ui_source_editor_dialog.h"
#include "utils/formatted_identifier_delegate.h"
#include "utils/model_utils.h"
#include "utils/translating_proxy_model.h"

#include <KCollapsibleGroupBox>
#include <KLocalizedString>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVBoxLayout>

SourceEditorDialog::SourceEditorDialog(std::optional<IntegerPrimaryKey> sourceId, QWidget* parent) :
    QDialog(parent),
    form(new Ui::SourceEditorForm),
    sourceId(sourceId),
    parentId(std::nullopt) {
    form->setupUi(this);

    connect(form->noteEditButton, &QPushButton::clicked, this, &SourceEditorDialog::editNoteWithEditor);
    connect(form->sourceParentAdd, &QPushButton::clicked, this, &SourceEditorDialog::addNewSourceAsParent);
    connect(
        form->sourceParentPickButton,
        &QPushButton::clicked,
        this,
        &SourceEditorDialog::selectExistingSourceAsParent
    );

    // Populate the type combo box from the source types lookup table.
    sourceTypesModel = new SourceTypesListModel(this);
    auto* typesProxy = new TranslatingProxyModel(
        TypeTranslationResolver(
            [](IntegerPrimaryKey typeId, const QString& locale) {
                return SourceTypeTranslationRepository().findByTypeIdAndLocale(typeId, locale);
            },
            SourceTypes::toDisplayString
        ),
        this
    );
    typesProxy->setSourceModel(sourceTypesModel);
    form->sourceTypeComboBox->setModel(typesProxy);
    form->sourceTypeComboBox->setModelColumn(SourceTypesListModel::TYPE);
    form->sourceTypeComboBox->setCurrentIndex(-1);

    // Populate the confidence combo box from the enum.
    confidenceModel = getEnumModel<Confidence::Values>(this, Confidence::toDisplayString);
    form->sourceConfidenceCombobox->setModel(confidenceModel);

    form->noteEdit->enableRichTextMode();

    // Load and pre-fill the entity data.
    SourceRepository repo;
    if (sourceId.has_value()) {
        if (const auto entity = repo.findById(*sourceId)) {
            form->sourcetTitleEdit->setText(entity->title);
            form->sourceAuthorEdit->setText(entity->author);
            form->sourcePublicationEdit->setText(entity->publication);
            form->noteEdit->setTextOrHtml(entity->note);

            if (entity->typeId.has_value()) {
                for (int i = 0; i < sourceTypesModel->rowCount(); ++i) {
                    const auto idx = sourceTypesModel->index(i, SourceTypesListModel::ID);
                    if (sourceTypesModel->data(idx).toLongLong() == *entity->typeId) {
                        form->sourceTypeComboBox->setCurrentIndex(i);
                        break;
                    }
                }
            }

            const auto storedValue = qToUnderlying(enumFromString<Confidence::Values>(entity->confidence));
            const auto idx = form->sourceConfidenceCombobox->findData(storedValue);
            if (idx >= 0) {
                form->sourceConfidenceCombobox->setCurrentIndex(idx);
            }

            parentId = entity->parentId;
            updateParentDisplay();

            auto nameId = format_id(FormattedIdentifierDelegate::SOURCE, *sourceId);
            this->setWindowTitle(i18n("Editing %1", nameId));
        }
    } else {
        const auto idx = form->sourceConfidenceCombobox->findData(qToUnderlying(Confidence::Values::Normal));
        form->sourceConfidenceCombobox->setCurrentIndex(idx);
        this->setWindowTitle(i18n("Add new source"));
    }

    if (sourceId.has_value()) {
        const auto id = *sourceId;
        auto* mediaGroup = new KCollapsibleGroupBox(this);
        mediaGroup->setTitle(i18n("Attached files"));
        auto* mediaLayout = new QVBoxLayout(mediaGroup);
        auto* mediaWidget = new MediaListWidget(
            [id] { return MediaRepository().findForSource(id); },
            [id](IntegerPrimaryKey mediaId) { return MediaRepository().attachToSource(id, mediaId); },
            [id](IntegerPrimaryKey mediaId) { return MediaRepository().detachFromSource(id, mediaId); },
            mediaGroup
        );
        mediaLayout->addWidget(mediaWidget);
        qobject_cast<QVBoxLayout*>(this->layout())->addWidget(mediaGroup);
    }

    connect(form->extractButton, &QPushButton::clicked, this, &SourceEditorDialog::onExtractClicked);
    form->parentSuggestionIconLabel->setPixmap(QIcon::fromTheme(u"tools-wizard"_s).pixmap(16, 16));

    connect(form->parentSuggestionUseButton, &QPushButton::clicked, this, &SourceEditorDialog::onUseSuggestedParent);
    connect(form->parentSuggestionPickButton, &QPushButton::clicked, this, &SourceEditorDialog::onPickDifferentParent);
    connect(
        form->parentSuggestionCreateButton,
        &QPushButton::clicked,
        this,
        &SourceEditorDialog::onCreateSuggestedParent
    );
}

void SourceEditorDialog::accept() {
    SourceRepository repo;

    auto title = form->sourcetTitleEdit->text();
    const auto typeRow = form->sourceTypeComboBox->currentIndex();
    const auto typeId = typeRow >= 0
                            ? std::optional<IntegerPrimaryKey>(
                                  sourceTypesModel->index(typeRow, SourceTypesListModel::ID).data().toLongLong()
                              )
                            : std::nullopt;
    auto author = form->sourceAuthorEdit->text();
    auto publication = form->sourcePublicationEdit->text();
    auto note = form->noteEdit->textOrHtml();

    auto confidenceInt = form->sourceConfidenceCombobox->currentData(Qt::UserRole).toInt();
    auto confidence = enumToString(static_cast<Confidence::Values>(confidenceInt));

    if (!sourceId.has_value()) {
        const auto newId = repo.insert(title, typeId, author, publication, confidence, note, parentId);
        if (!newId) {
            qWarning() << "Could not insert new source";
            return;
        }
        sourceId = newId;
    } else if (!repo.update(*sourceId, title, typeId, author, publication, confidence, note, parentId)) {
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

const QJsonObject extractionSchema = QJsonObject{
    {u"$schema"_s, u"https://json-schema.org/draft/2020-12/schema"_s},
    {u"type"_s, u"object"_s},
    {u"properties"_s,
     QJsonObject{
         {u"title"_s, QJsonObject{{u"type"_s, u"string"_s}}},
         {u"type"_s, QJsonObject{{u"type"_s, u"string"_s}}},
         {u"author"_s, QJsonObject{{u"type"_s, u"string"_s}}},
         {u"publication"_s, QJsonObject{{u"type"_s, u"string"_s}}},
         {u"note"_s, QJsonObject{{u"type"_s, u"string"_s}}},
         {u"suggested_parent"_s,
          QJsonObject{
              {u"type"_s, u"string"_s},
              {u"description"_s,
               u"Name of the parent source this item belongs to (e.g. newspaper name, book series, archive)"_s}
          }},
         {u"message"_s,
          QJsonObject{
              {u"type"_s, u"string"_s},
              {u"description"_s, u"Optional note about ambiguous or missing fields"_s}
          }},
     }},
    {u"required"_s, QJsonArray{u"title"_s, u"type"_s, u"confidence"_s}},
};

void SourceEditorDialog::onExtractClicked() {
    auto* svc = createAiService(this);
    if (!svc) {
        form->aiStatusLabel->setText(i18n("No AI provider configured. Set one in Preferences → AI."));
        return;
    }
    form->extractButton->setEnabled(false);
    form->aiStatusLabel->setText(i18n("Extracting…"));

    connect(svc, &AiService::responseReady, this, &SourceEditorDialog::onAiResponse);
    connect(svc, &AiService::requestFailed, this, &SourceEditorDialog::onAiFailed);
    svc->complete(opaSettings::sourceExtractionPrompt(), form->rawTextEdit->toPlainText(), extractionSchema);
}

void SourceEditorDialog::onAiResponse(const QString& response) {
    form->extractButton->setEnabled(true);

    const QJsonObject obj = QJsonDocument::fromJson(response.toUtf8()).object();
    if (obj.isEmpty()) {
        form->aiStatusLabel->setText(i18n("Could not parse AI response as JSON."));
        return;
    }

    clearFieldSuggestions();

    const auto wizardIcon = QIcon::fromTheme(u"tools-wizard"_s);
    auto setOrSuggest = [&](const QString& key, QLineEdit* edit) {
        if (const auto v = obj[key].toString(); !v.isEmpty()) {
            if (edit->text().isEmpty()) {
                edit->setText(v);
                auto* action = edit->addAction(wizardIcon, QLineEdit::TrailingPosition);
                fieldIndicatorActions.append(action);
            } else if (edit->text() != v) {
                showFieldSuggestion(edit, v);
            }
        }
    };
    setOrSuggest(u"title"_s, form->sourcetTitleEdit);
    setOrSuggest(u"author"_s, form->sourceAuthorEdit);
    setOrSuggest(u"publication"_s, form->sourcePublicationEdit);

    if (const auto t = obj[u"type"_s].toString(); !t.isEmpty()) {
        for (int i = 0; i < sourceTypesModel->rowCount(); ++i) {
            const auto typeIdx = sourceTypesModel->index(i, SourceTypesListModel::TYPE);
            if (sourceTypesModel->data(typeIdx).toString().compare(t, Qt::CaseInsensitive) == 0) {
                form->sourceTypeComboBox->setCurrentIndex(i);
                break;
            }
        }
    }

    if (const auto n = obj[u"note"_s].toString(); !n.isEmpty()) {
        form->noteEdit->setTextOrHtml(n);
    }

    if (const auto sp = obj[u"suggested_parent"_s].toString(); !sp.isEmpty()) {
        suggestedParentName = sp;
        showParentSuggestion();
    }

    const auto msg = obj[u"message"_s].toString();
    if (msg.isEmpty()) {
        form->aiStatusLabel->setText(i18n("Fields extracted successfully."));
    } else {
        form->aiStatusLabel->setText(msg);
    }
}

void SourceEditorDialog::onAiFailed(const QString& error) {
    form->extractButton->setEnabled(true);
    form->aiStatusLabel->setText(error);
}

void SourceEditorDialog::showParentSuggestion() {
    SourceRepository repo;
    parentSuggestionMatches = repo.findByTitleContaining(suggestedParentName);

    // Exclude self from matches.
    if (sourceId.has_value()) {
        parentSuggestionMatches.removeIf([this](const SourceEntity& e) { return e.id == *sourceId; });
    }

    if (!parentSuggestionMatches.isEmpty()) {
        const auto& best = parentSuggestionMatches.first();
        auto displayId = format_id(FormattedIdentifierDelegate::SOURCE, best.id);
        form->parentSuggestionLabel->setText(i18n("%1 (%2)", best.title, displayId));
        form->parentSuggestionUseButton->setVisible(true);
        form->parentSuggestionPickButton->setVisible(true);
        form->parentSuggestionCreateButton->setVisible(false);
    } else {
        form->parentSuggestionLabel->setText(i18n("Create: %1", suggestedParentName));
        form->parentSuggestionUseButton->setVisible(false);
        form->parentSuggestionPickButton->setVisible(false);
        form->parentSuggestionCreateButton->setVisible(true);
    }

    form->parentSuggestionFrame->setVisible(true);
}

void SourceEditorDialog::hideParentSuggestion() {
    form->parentSuggestionFrame->setVisible(false);
}

void SourceEditorDialog::onUseSuggestedParent() {
    if (!parentSuggestionMatches.isEmpty()) {
        parentId = parentSuggestionMatches.first().id;
        updateParentDisplay();
    }
    hideParentSuggestion();
}

void SourceEditorDialog::onPickDifferentParent() {
    hideParentSuggestion();
    selectExistingSourceAsParent();
}

void SourceEditorDialog::onCreateSuggestedParent() {
    hideParentSuggestion();
    auto* dialog = new SourceEditorDialog(std::nullopt, this);
    dialog->form->sourcetTitleEdit->setText(suggestedParentName);
    if (dialog->exec() == Accepted && dialog->sourceId.has_value()) {
        parentId = dialog->sourceId;
        updateParentDisplay();
    }
}

void SourceEditorDialog::clearFieldSuggestions() {
    for (auto* frame: fieldSuggestionFrames) {
        frame->deleteLater();
    }
    fieldSuggestionFrames.clear();

    for (auto* action: fieldIndicatorActions) {
        delete action;
    }
    fieldIndicatorActions.clear();
}

void SourceEditorDialog::showFieldSuggestion(QLineEdit* field, const QString& suggestedValue) {
    auto* formLayout = form->formLayout_2;

    int row = -1;
    QFormLayout::ItemRole role;
    formLayout->getWidgetPosition(field, &row, &role);
    if (row < 0) {
        return;
    }

    auto* frame = new QFrame(this);
    frame->setFrameShape(QFrame::StyledPanel);
    auto* layout = new QHBoxLayout(frame);
    layout->setContentsMargins(4, 2, 4, 2);

    auto* iconLabel = new QLabel(frame);
    iconLabel->setPixmap(QIcon::fromTheme(u"tools-wizard"_s).pixmap(16, 16));
    layout->addWidget(iconLabel);

    auto* textLabel = new QLabel(suggestedValue, frame);
    textLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    layout->addWidget(textLabel);

    auto* acceptBtn = new QPushButton(frame);
    acceptBtn->setIcon(QIcon::fromTheme(u"dialog-ok-apply"_s));
    acceptBtn->setFlat(true);
    connect(acceptBtn, &QPushButton::clicked, this, [this, field, suggestedValue, frame]() {
        field->setText(suggestedValue);
        fieldSuggestionFrames.removeOne(frame);
        frame->deleteLater();
    });
    layout->addWidget(acceptBtn);

    auto* dismissBtn = new QPushButton(frame);
    dismissBtn->setIcon(QIcon::fromTheme(u"dialog-cancel"_s));
    dismissBtn->setFlat(true);
    connect(dismissBtn, &QPushButton::clicked, this, [this, frame]() {
        fieldSuggestionFrames.removeOne(frame);
        frame->deleteLater();
    });
    layout->addWidget(dismissBtn);

    formLayout->insertRow(row + 1, static_cast<QWidget*>(nullptr), frame);
    fieldSuggestionFrames.append(frame);
}
