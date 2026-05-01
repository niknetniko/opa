/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "database/schema.h"
#include "domain/source/source_entities.h"

#include <QDialog>
#include <QList>
#include <optional>

namespace Ui {
class SourceEditorForm;
}

class QAbstractItemModel;
class QFrame;
class QLineEdit;
class SourceTypesListModel;

class SourceEditorDialog : public QDialog {
    Q_OBJECT

public:
    explicit SourceEditorDialog(std::optional<IntegerPrimaryKey> sourceId, QWidget* parent);

    static QVariant showDialogForNewSource(QWidget* parent);
    static void showDialogForExistingSource(IntegerPrimaryKey sourceId, QWidget* parent);

public Q_SLOTS:
    void accept() override;
    void editNoteWithEditor();
    void addNewSourceAsParent();
    void selectExistingSourceAsParent();

private Q_SLOTS:
    void onExtractClicked();
    void onAiResponse(const QString& json);
    void onAiFailed(const QString& error);
    void onUseSuggestedParent();
    void onPickDifferentParent();
    void onCreateSuggestedParent();

private:
    Ui::SourceEditorForm* form;
    std::optional<IntegerPrimaryKey> sourceId;
    std::optional<IntegerPrimaryKey> parentId;
    QAbstractItemModel* confidenceModel;
    SourceTypesListModel* sourceTypesModel = nullptr;

    QString suggestedParentName;
    QList<SourceEntity> parentSuggestionMatches;
    QList<QFrame*> fieldSuggestionFrames;
    QList<QAction*> fieldIndicatorActions;

    void updateParentDisplay() const;
    void showParentSuggestion();
    void hideParentSuggestion();
    void clearFieldSuggestions();
    void showFieldSuggestion(QLineEdit* field, const QString& suggestedValue);
};
