/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "new_family_editor_dialog.h"

#include "../domain/person/person_sex.h"
#include "database/database.h"
#include "domain/event/event_repository.h"
#include "domain/family/family_repository.h"
#include "domain/person/person_detail_model.h"
#include "link_existing/choose_existing_person_window.h"
#include "new_person_editor_dialog.h"
#include "ui_new_family_editor_dialog.h"
#include "utils/formatted_identifier_delegate.h"
#include "welcome/welcome_view.h"

#include <QCompleter>
#include <QDataWidgetMapper>

namespace {
struct BirthEventDetails {
    QVariant eventId;
    bool isNew;
};

BirthEventDetails getOrCreateBirthEvent(IntegerPrimaryKey personId) {
    const EventRepository repo;
    const auto birthEvents = repo.findBirthEventsForPerson(personId);

    if (birthEvents.count() == 0) {
        const auto newEventId =
            repo.insertEventWithRelation(QStringLiteral("Birth"), personId, QStringLiteral("Primary"));
        return {.eventId = newEventId ? QVariant(*newEventId) : QVariant {}, .isNew = true};
    }

    const IntegerPrimaryKey birthEventId = birthEvents.at(0).id;
    return {.eventId = birthEventId, .isNew = false};
}
}

NewFamilyEditorDialog::NewFamilyEditorDialog(IntegerPrimaryKey personId, QWidget* parent) :
    QDialog(parent),
    form(new Ui::NewFamilyEditorForm) {
    form->setupUi(this);

    data.childId = personId;

    auto [birthEventId, hasNewBirthEvent] = getOrCreateBirthEvent(personId);
    data.hasNewBirthEvent = hasNewBirthEvent;
    Q_ASSERT(birthEventId.isValid());
    data.birthEventId = birthEventId;

    connect(form->motherExistingPerson, &QPushButton::clicked, this, &NewFamilyEditorDialog::onSelectExistingMother);
    connect(form->fatherExistingPerson, &QPushButton::clicked, this, &NewFamilyEditorDialog::onSelectExistingFather);
    connect(form->motherNewPerson, &QPushButton::clicked, this, &NewFamilyEditorDialog::onSelectNewMother);
    connect(form->fatherNewPerson, &QPushButton::clicked, this, &NewFamilyEditorDialog::onSelectNewFather);

    // TODO: populate
    auto* parentRolesModel = new QStringListModel(this);

    form->motherRelation->setEnabled(false);
    motherIdMapper = new QDataWidgetMapper(this);
    motherBirthMapper = new QDataWidgetMapper(this);
    motherNameMapper = new QDataWidgetMapper(this);
    motherRelationMapper = new QDataWidgetMapper(this);

    // TODO: Support translatable enums in the QComboxBox.
    // Issue URL: https://github.com/niknetniko/opa/issues/59
    form->motherRelation->setModel(parentRolesModel);

    form->fatherRelation->setEnabled(false);
    fatherIdMapper = new QDataWidgetMapper(this);
    fatherBirthMapper = new QDataWidgetMapper(this);
    fatherNameMapper = new QDataWidgetMapper(this);
    fatherRelationMapper = new QDataWidgetMapper(this);

    form->fatherRelation->setModel(parentRolesModel);

    parentRelationMapper = new QDataWidgetMapper(this);
    form->parentRelationRole->setEnabled(false);
    // TODO: populate
    auto* relationshipEventTypes = new QStringListModel(this);
    form->parentRelationRole->setModel(relationshipEventTypes);

    this->setAttribute(Qt::WA_DeleteOnClose);
}

NewFamilyEditorDialog::~NewFamilyEditorDialog() {
    delete form;
}

bool NewFamilyEditorDialog::saveNewFamily() const {
    // TODO: allow linking parents without adding a new marriage.
    // Issue URL: https://github.com/niknetniko/opa/issues/63
    Q_ASSERT(data.birthEventId.isValid());

    FamilyRepository familyRepo;
    auto familyId = familyRepo.createFamily();
    if (!familyId.has_value()) {
        qWarning() << "Could not create family record";
        return false;
    }

    const EventRepository repo;

    auto chosenMotherId = data.motherId;
    auto chosenFatherId = data.fatherId;

    if (chosenMotherId.isValid() && chosenFatherId.isValid()) {
        auto selectedTypeRow = form->parentRelationRole->currentIndex();
        auto chosenTypeId = form->parentRelationRole->model()->index(selectedTypeRow, 0).data().toString();

        auto eventId = repo.insertEvent(chosenTypeId);
        if (!eventId.has_value()) {
            qWarning() << "Could not insert relationship event";
            return false;
        }

        if (!repo.insertEventRelation(*eventId, chosenMotherId.toLongLong(), QStringLiteral("Primary")).has_value()) {
            qWarning() << "Could not insert mother to relationship event";
            return false;
        }

        if (!repo.insertEventRelation(*eventId, chosenFatherId.toLongLong(), QStringLiteral("Partner")).has_value()) {
            qWarning() << "Could not insert father to relationship event";
            return false;
        }

        if (!familyRepo.linkEventToFamily(*eventId, *familyId)) {
            qWarning() << "Could not link relationship event to family";
            return false;
        }
    }

    // Now, link the parents to the child.
    if (chosenMotherId.isValid()) {
        if (!repo.insertEventRelation(
                     data.birthEventId.toLongLong(),
                     chosenMotherId.toLongLong(),
                     QStringLiteral("Mother")
            )
                 .has_value()) {
            qWarning() << "Could not insert mother to birth event relationship";
            return false;
        }
    }

    if (chosenFatherId.isValid()) {
        if (!repo.insertEventRelation(
                     data.birthEventId.toLongLong(),
                     chosenFatherId.toLongLong(),
                     QStringLiteral("Father")
            )
                 .has_value()) {
            qWarning() << "Could not insert father to birth event relationship";
            return false;
        }
    }

    if (!familyRepo.linkEventToFamily(data.birthEventId.toLongLong(), *familyId)) {
        qWarning() << "Could not link birth event to family";
        return false;
    }

    return true;
}

void NewFamilyEditorDialog::accept() {
    auto result = executeInTransaction([&]() -> std::optional<bool> {
        if (!saveNewFamily()) {
            return std::nullopt;
        }
        return true;
    });

    if (result) {
        QDialog::accept();
    } else {
        qWarning() << "Something went wrong while saving a family.";
    }
}

void NewFamilyEditorDialog::reject() {
    if (data.hasNewBirthEvent) {
        Q_ASSERT(data.birthEventId.isValid());
        const auto birthEventId = data.birthEventId.toLongLong();

        const EventRepository repo;
        // Delete all relations for this birth event, then delete the event itself.
        const auto relations = repo.findRelationsForEvent(birthEventId);
        for (const auto& relation: relations) {
            if (!repo.deleteEventRelation(relation.id)) {
                qWarning() << "Could not remove birth event relation" << relation.id;
            }
        }

        if (!repo.deleteEvent(birthEventId)) {
            qWarning() << "Could not remove birth event";
        }
    }

    QDialog::reject();
}

void NewFamilyEditorDialog::onSelectExistingMother() {
    auto motherId = ChooseExistingPersonWindow::selectPerson(this);
    if (motherId.isValid()) {
        setMother(motherId.toLongLong());
    }
}

void NewFamilyEditorDialog::onSelectExistingFather() {
    auto fatherId = ChooseExistingPersonWindow::selectPerson(this);
    if (fatherId.isValid()) {
        setFather(fatherId.toLongLong());
    }
}

void NewFamilyEditorDialog::onSelectNewMother() {
    NewPersonEditorDialog addDialog(this);
    addDialog.exec();

    if (addDialog.addedPersonId.isValid()) {
        setMother(addDialog.addedPersonId.toLongLong());
    }
}
void NewFamilyEditorDialog::onSelectNewFather() {
    NewPersonEditorDialog addDialog(this);
    addDialog.exec();

    if (addDialog.addedPersonId.isValid()) {
        setFather(addDialog.addedPersonId.toLongLong());
    }
}

void NewFamilyEditorDialog::setMother(const QVariant& motherId) {
    if (!motherId.isValid()) {
        return;
    }
    form->motherRelation->setEnabled(true);
    data.motherId = motherId;

    auto* motherDetailsModel = new PersonDetailModel(motherId.toLongLong(), this);
    motherIdMapper->setModel(motherDetailsModel);
    motherIdMapper->addMapping(form->motherId, PersonDetailModel::ID, "text");
    motherIdMapper->setItemDelegate(new FormattedIdentifierDelegate(this, FormattedIdentifierDelegate::PERSON));
    motherIdMapper->toFirst();

    motherNameMapper->setModel(motherDetailsModel);
    motherNameMapper->addMapping(form->motherName, PersonDetailModel::DISPLAY_NAME, "text");
    motherNameMapper->toFirst();

    // TODO: replace by just querying
    // auto* birthEvent = new PersonBirthEventsModel(motherId.toLongLong(), this);
    // motherBirthMapper->setModel(birthEvent);
    // motherBirthMapper->addMapping(form->motherBirth, PersonBirthEventsModel::DATE, "text");
    // motherBirthMapper->toFirst();

    setParentRelationIfPossible();
}

void NewFamilyEditorDialog::setFather(const QVariant& fatherId) {
    if (!fatherId.isValid()) {
        return;
    }
    form->fatherRelation->setEnabled(true);
    data.fatherId = fatherId;

    auto* fatherDetailsModel = new PersonDetailModel(fatherId.toLongLong(), this);
    fatherIdMapper->setModel(fatherDetailsModel);
    fatherIdMapper->addMapping(form->fatherId, PersonDetailModel::ID, "text");
    fatherIdMapper->setItemDelegate(new FormattedIdentifierDelegate(this, FormattedIdentifierDelegate::PERSON));
    fatherIdMapper->toFirst();

    fatherNameMapper->setModel(fatherDetailsModel);
    fatherNameMapper->addMapping(form->fatherName, PersonDetailModel::DISPLAY_NAME, "text");
    fatherNameMapper->toFirst();

    // TODO: replace by just querying
    // auto* birthEvent = new PersonBirthEventsModel(fatherId.toLongLong(), this);
    // fatherBirthMapper->setModel(birthEvent);
    // fatherBirthMapper->addMapping(form->fatherBirth, PersonBirthEventsModel::DATE, "text");
    // fatherBirthMapper->toFirst();

    setParentRelationIfPossible();
}

void NewFamilyEditorDialog::setParentRelationIfPossible() const {
    form->parentRelationRole->setEnabled(data.motherId.isValid() && data.fatherId.isValid());
}
