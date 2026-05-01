/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "source_types_management_window.h"

#include "domain/source/source_repository.h"
#include "domain/source/source_type_translation_repository.h"
#include "domain/source/source_types.h"
#include "domain/source/source_types_list_model.h"
#include "editors/type_translations_dialog.h"
#include "utils/model_utils.h"

#include <KLocalizedString>
#include <QMessageBox>
#include <QToolBar>

SourceTypesManagementWindow::SourceTypesManagementWindow() {
    setWindowTitle(i18n("Manage source types"));

    auto* listModel = new SourceTypesListModel(this);
    setModel(listModel);
    setColumns(SourceTypesListModel::ID, SourceTypesListModel::TYPE, SourceTypesListModel::BUILTIN);
    setTranslator(SourceTypes::toDisplayString);

    initializeLayout();

    translationsAction = new QAction(i18n("Translations"), this);
    translationsAction->setIcon(QIcon::fromTheme(QStringLiteral("accessories-dictionary")));
    translationsAction->setEnabled(false);
    mainToolbar->addAction(translationsAction);

    connect(
        tableView->selectionModel(),
        &QItemSelectionModel::selectionChanged,
        this,
        [this](const QItemSelection& selected, const QItemSelection&) {
            if (selected.isEmpty()) {
                translationsAction->setEnabled(false);
                return;
            }
            const auto idx = mapToSourceModel(selected.indexes().constFirst());
            const auto isBuiltin = model->index(idx.row(), SourceTypesListModel::BUILTIN).data().toBool();
            translationsAction->setEnabled(!isBuiltin);
        }
    );

    connect(translationsAction, &QAction::triggered, this, [this]() {
        auto* selection = tableView->selectionModel();
        if (!selection->hasSelection()) {
            return;
        }
        const auto idx = mapToSourceModel(selection->selection().constFirst().indexes().constFirst());
        const auto typeId = model->index(idx.row(), SourceTypesListModel::ID).data().toLongLong();
        const auto typeName = model->index(idx.row(), SourceTypesListModel::TYPE).data().toString();

        auto* dialog = new TypeTranslationsDialog(
            typeName,
            [typeId]() -> QList<TypeTranslationsDialog::TranslationEntry> {
                SourceTypeTranslationRepository repo;
                QList<TypeTranslationsDialog::TranslationEntry> result;
                for (const auto& e : repo.findAllForType(typeId)) {
                    result.append({.id = e.id, .locale = e.locale, .name = e.name});
                }
                return result;
            },
            [typeId](const QString& locale, const QString& name) -> std::optional<IntegerPrimaryKey> {
                SourceTypeTranslationRepository repo;
                return repo.insert(typeId, locale, name);
            },
            [](IntegerPrimaryKey id) -> bool {
                SourceTypeTranslationRepository repo;
                return repo.remove(id);
            },
            this
        );
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        dialog->show();
    });
}

bool SourceTypesManagementWindow::repairConfirmation() {
    QMessageBox messageBox(this);
    messageBox.setIcon(QMessageBox::Question);
    messageBox.setText(i18n("Clean up source types?"));
    messageBox.setInformativeText(
        i18n("This will merge duplicate and remove empty source types. Do you want to continue?")
    );
    messageBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    messageBox.setDefaultButton(QMessageBox::Ok);
    return messageBox.exec() == QMessageBox::Ok;
}

void SourceTypesManagementWindow::repairItems() {
    // Not implemented for source types.
}

void SourceTypesManagementWindow::removeMarkedReferences(
    [[maybe_unused]] const QHash<QString, QVector<IntegerPrimaryKey>>& valueToIds,
    [[maybe_unused]] const QHash<IntegerPrimaryKey, QString>& idToValue
) {
    // Not used: source types management does not support repair.
}

bool SourceTypesManagementWindow::isUsed(const QVariant& id) {
    SourceRepository repo;
    return repo.isSourceTypeUsed(id.toLongLong());
}

QVariant SourceTypesManagementWindow::doAddItem() const {
    SourceRepository repo;
    const auto newId = repo.insertSourceType(QString());
    return newId ? QVariant(*newId) : QVariant{};
}

bool SourceTypesManagementWindow::doRemoveItem(const QVariant& id) const {
    SourceRepository repo;
    return repo.deleteSourceType(id.toLongLong());
}

QString SourceTypesManagementWindow::translatedItemCount(int itemCount) const {
    return i18np("%1 source type", "%1 source types", itemCount);
}

QString SourceTypesManagementWindow::translatedItemDescription(const QString& item, bool isBuiltIn) const {
    if (isBuiltIn) {
        return i18n("Built-in source type '%1'", item);
    }
    return i18n("Source type '%1'", item);
}
