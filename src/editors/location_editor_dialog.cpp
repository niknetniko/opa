/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "location_editor_dialog.h"

#include "dates/genealogical_date.h"
#include "dates/genealogical_date_editor_dialog.h"
#include "domain/location/location_repository.h"
#include "domain/location/location_type_translation_repository.h"
#include "domain/location/location_types.h"
#include "domain/location/location_types_list_model.h"
#include "link_existing/choose_existing_location_window.h"
#include "note_editor_dialog.h"
#include "ui_location_editor_dialog.h"
#include "utils/formatted_identifier_delegate.h"
#include "utils/translating_proxy_model.h"

#include <KLocalizedString>

using namespace Qt::StringLiterals;

// Sentinel value used by the spin box "specialValueText" to represent "not set".
static constexpr double LATITUDE_NOT_SET = -91.0;
static constexpr double LONGITUDE_NOT_SET = -181.0;

LocationEditorDialog::LocationEditorDialog(
    std::optional<IntegerPrimaryKey> locationId,
    std::optional<IntegerPrimaryKey> initialParentId,
    QWidget* parent
) :
    QDialog(parent),
    form(std::make_unique<Ui::LocationEditorForm>()),
    locationId(locationId),
    parentId(initialParentId) {
    form->setupUi(this);

    connect(form->noteEditButton, &QPushButton::clicked, this, &LocationEditorDialog::editNoteWithEditor);
    connect(form->parentAddButton, &QPushButton::clicked, this, &LocationEditorDialog::addNewLocationAsParent);
    connect(form->parentPickButton, &QPushButton::clicked, this, &LocationEditorDialog::selectExistingLocationAsParent);
    connect(form->dateStartEditButton, &QPushButton::clicked, this, &LocationEditorDialog::editDateStartWithEditor);
    connect(form->dateEndEditButton, &QPushButton::clicked, this, &LocationEditorDialog::editDateEndWithEditor);

    auto* typesModel = new LocationTypesListModel(this);
    auto* typesProxy = new TranslatingProxyModel(
        TypeTranslationResolver(
            [](IntegerPrimaryKey typeId, const QString& locale) {
                return LocationTypeTranslationRepository().findByTypeIdAndLocale(typeId, locale);
            },
            LocationTypes::toDisplayString
        ),
        this
    );
    typesProxy->setSourceModel(typesModel);
    form->typeComboBox->setModel(typesProxy);
    form->typeComboBox->setModelColumn(LocationTypesListModel::TYPE);
    // Start with no type selected (nullable)
    form->typeComboBox->setCurrentIndex(-1);

    form->noteEdit->enableRichTextMode();

    updateParentDisplay();

    if (locationId.has_value()) {
        LocationRepository repo;
        if (const auto entity = repo.findById(*locationId)) {
            form->nameEdit->setText(entity->name);
            form->noteEdit->setTextOrHtml(entity->note);

            if (entity->typeId.has_value()) {
                // Find the row in the model for this type id
                const auto* model = typesModel;
                for (int i = 0; i < model->rowCount(); ++i) {
                    const auto idx = model->index(i, LocationTypesListModel::ID);
                    if (model->data(idx).toLongLong() == *entity->typeId) {
                        form->typeComboBox->setCurrentIndex(i);
                        break;
                    }
                }
            }

            if (entity->coordinates.has_value()) {
                form->latitudeSpinBox->setValue(entity->coordinates->latitude);
                form->longitudeSpinBox->setValue(entity->coordinates->longitude);
            }

            if (!entity->dateStart.isEmpty()) {
                form->dateStartEdit->setText(
                    GenealogicalDate::fromDatabaseRepresentation(entity->dateStart).toDisplayText()
                );
            }
            if (!entity->dateEnd.isEmpty()) {
                form->dateEndEdit->setText(
                    GenealogicalDate::fromDatabaseRepresentation(entity->dateEnd).toDisplayText()
                );
            }

            parentId = entity->parentId;
            updateParentDisplay();

            this->setWindowTitle(i18n("Editing %1", format_id(FormattedIdentifierDelegate::LOCATION, *locationId)));
        }
    } else {
        this->setWindowTitle(i18n("Add new location"));
    }
}

LocationEditorDialog::~LocationEditorDialog() = default;

void LocationEditorDialog::accept() {
    LocationRepository repo;

    const auto name = form->nameEdit->text();
    if (name.isEmpty()) {
        return;
    }

    const auto note = form->noteEdit->textOrHtml();

    const auto typeIdx = form->typeComboBox->currentIndex();
    std::optional<IntegerPrimaryKey> typeId;
    if (typeIdx >= 0) {
        const auto idIdx = form->typeComboBox->model()->index(typeIdx, LocationTypesListModel::ID);
        typeId = form->typeComboBox->model()->data(idIdx).toLongLong();
    }

    std::optional<Coordinates> coordinates;
    const auto lat = form->latitudeSpinBox->value();
    const auto lon = form->longitudeSpinBox->value();
    if (lat != LATITUDE_NOT_SET && lon != LONGITUDE_NOT_SET) {
        coordinates = Coordinates{.latitude=lat, .longitude=lon};
    }

    const auto dateStartText = form->dateStartEdit->text();
    const auto dateStart = dateStartText.isEmpty()
                               ? QString{}
                               : GenealogicalDate::fromDisplayText(dateStartText).toDatabaseRepresentation();

    const auto dateEndText = form->dateEndEdit->text();
    const auto dateEnd = dateEndText.isEmpty()
                             ? QString{}
                             : GenealogicalDate::fromDisplayText(dateEndText).toDatabaseRepresentation();

    if (!locationId.has_value()) {
        const auto newId = repo.insert(name, typeId, parentId);
        if (!newId) {
            qWarning() << "Could not insert new location";
            return;
        }
        locationId = newId;
        if (!repo.update(*locationId, name, typeId, parentId, note, coordinates, dateStart, dateEnd)) {
            qWarning() << "Could not update newly inserted location" << *locationId;
            return;
        }
    } else if (!repo.update(*locationId, name, typeId, parentId, note, coordinates, dateStart, dateEnd)) {
        qWarning() << "Could not save location" << *locationId;
        return;
    }

    QDialog::accept();
}

QVariant LocationEditorDialog::showDialogForNewLocation(std::optional<IntegerPrimaryKey> parentId, QWidget* parent) {
    LocationEditorDialog dialog(std::nullopt, parentId, parent);
    if (dialog.exec() != Accepted || !dialog.locationId) {
        return {};
    }
    return QVariant::fromValue(*dialog.locationId);
}

void LocationEditorDialog::showDialogForExistingLocation(IntegerPrimaryKey locationId, QWidget* parent) {
    auto* dialog = new LocationEditorDialog(locationId, std::nullopt, parent);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

void LocationEditorDialog::editNoteWithEditor() {
    auto currentText = form->noteEdit->textOrHtml();
    if (auto note = NoteEditorDialog::editText(currentText, i18n("Edit note"), this); !note.isEmpty()) {
        form->noteEdit->setTextOrHtml(note);
    }
}

void LocationEditorDialog::addNewLocationAsParent() {
    auto newId = showDialogForNewLocation(std::nullopt, this);
    if (newId.isValid()) {
        parentId = newId.toLongLong();
        updateParentDisplay();
    }
}

void LocationEditorDialog::selectExistingLocationAsParent() {
    auto selected = ChooseExistingLocationWindow::selectLocation(this);
    if (selected.isValid()) {
        parentId = selected.toLongLong();
        updateParentDisplay();
    }
}

void LocationEditorDialog::editDateStartWithEditor() {
    auto currentText = form->dateStartEdit->text();
    auto startDate = GenealogicalDate::fromDisplayText(currentText);
    if (auto date = GenealogicalDateEditorDialog::editDate(startDate, this); date.isValid()) {
        form->dateStartEdit->setText(date.toDisplayText());
    }
}

void LocationEditorDialog::editDateEndWithEditor() {
    auto currentText = form->dateEndEdit->text();
    auto endDate = GenealogicalDate::fromDisplayText(currentText);
    if (auto date = GenealogicalDateEditorDialog::editDate(endDate, this); date.isValid()) {
        form->dateEndEdit->setText(date.toDisplayText());
    }
}

void LocationEditorDialog::updateParentDisplay() const {
    if (parentId.has_value()) {
        LocationRepository repo;
        if (const auto entity = repo.findById(*parentId)) {
            form->parentDisplay->setText(entity->name);
        } else {
            qWarning() << "Parent location" << *parentId << "not found in database";
            form->parentDisplay->setText(format_id(FormattedIdentifierDelegate::LOCATION, *parentId));
        }
    } else {
        form->parentDisplay->setText(i18n("No parent selected"));
    }
}
