/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "location_types_management_window.h"

#include "domain/location/location_repository.h"
#include "domain/location/location_type_translation_repository.h"
#include "domain/location/location_types.h"
#include "domain/location/location_types_list_model.h"
#include "editors/type_translations_dialog.h"
#include "utils/model_utils.h"

#include <KLocalizedString>
#include <QMessageBox>
#include <QToolBar>

LocationTypesManagementWindow::LocationTypesManagementWindow() {
    setWindowTitle(i18n("Manage location types"));

    auto* listModel = new LocationTypesListModel(this);
    setModel(listModel);
    setColumns(LocationTypesListModel::ID, LocationTypesListModel::TYPE, LocationTypesListModel::BUILTIN);
    setTranslator(LocationTypes::toDisplayString);

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
            const auto isBuiltin = model->index(idx.row(), LocationTypesListModel::BUILTIN).data().toBool();
            translationsAction->setEnabled(!isBuiltin);
        }
    );

    connect(translationsAction, &QAction::triggered, this, [this]() {
        auto* selection = tableView->selectionModel();
        if (!selection->hasSelection()) {
            return;
        }
        const auto idx = mapToSourceModel(selection->selection().constFirst().indexes().constFirst());
        const auto typeId = model->index(idx.row(), LocationTypesListModel::ID).data().toLongLong();
        const auto typeName = model->index(idx.row(), LocationTypesListModel::TYPE).data().toString();

        auto* dialog = new TypeTranslationsDialog(
            typeName,
            [typeId]() -> QList<TypeTranslationsDialog::TranslationEntry> {
                LocationTypeTranslationRepository repo;
                QList<TypeTranslationsDialog::TranslationEntry> result;
                for (const auto& e : repo.findAllForType(typeId)) {
                    result.append({.id = e.id, .locale = e.locale, .name = e.name});
                }
                return result;
            },
            [typeId](const QString& locale, const QString& name) -> std::optional<IntegerPrimaryKey> {
                LocationTypeTranslationRepository repo;
                return repo.insert(typeId, locale, name);
            },
            [](IntegerPrimaryKey id) -> bool {
                LocationTypeTranslationRepository repo;
                return repo.remove(id);
            },
            this
        );
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        dialog->show();
    });
}

bool LocationTypesManagementWindow::repairConfirmation() {
    return false;
}

void LocationTypesManagementWindow::repairItems() {
    // Not implemented for location types.
}

void LocationTypesManagementWindow::removeMarkedReferences(
    [[maybe_unused]] const QHash<QString, QVector<IntegerPrimaryKey>>& valueToIds,
    [[maybe_unused]] const QHash<IntegerPrimaryKey, QString>& idToValue
) {
    // Not used: location types management does not support repair.
}

bool LocationTypesManagementWindow::isUsed(const QVariant& id) {
    LocationRepository repo;
    return repo.isLocationTypeUsed(id.toLongLong());
}

QVariant LocationTypesManagementWindow::doAddItem() const {
    LocationRepository repo;
    const auto newId = repo.insertLocationType(i18n("New type"));
    return newId ? QVariant(*newId) : QVariant{};
}

bool LocationTypesManagementWindow::doRemoveItem(const QVariant& id) const {
    LocationRepository repo;
    return repo.deleteLocationType(id.toLongLong());
}

QString LocationTypesManagementWindow::translatedItemCount(int itemCount) const {
    return i18np("%1 location type", "%1 location types", itemCount);
}

QString LocationTypesManagementWindow::translatedItemDescription(const QString& item, bool isBuiltIn) const {
    if (isBuiltIn) {
        return i18n("Built-in location type '%1'", item);
    }
    return i18n("Location type '%1'", item);
}
