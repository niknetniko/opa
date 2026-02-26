/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "new_family_editor_dialog.h"

#include "data/person.h"
#include "database/database.h"
#include "domain/event/event_repository.h"
#include "domain/event/parent_event_roles_list_model.h"
#include "domain/event/person_birth_events_model.h"
#include "domain/person/person_detail_model.h"
#include "link_existing/choose_existing_person_window.h"
#include "new_person_editor_dialog.h"
#include "ui_new_family_editor_dialog.h"
#include "utils/formatted_identifier_delegate.h"
#include "welcome/welcome_view.h"

#include <QCompleter>
#include <QDataWidgetMapper>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>

namespace {
    struct BirthEventDetails {
        QVariant eventId;
        bool isNew;
    };

    BirthEventDetails getOrCreateBirthEvent(QObject* parent, IntegerPrimaryKey personId) {
        auto* birthEventModel = new PersonBirthEventsModel(personId, parent);
        if (birthEventModel->rowCount() == 0) {
            EventRepository repo;
            const auto primaryRoleId = repo.findEventRoleIdByName(QStringLiteral("Primary"));
            const auto birthTypeId = repo.findEventTypeIdByName(QStringLiteral("Birth"));
            if (!primaryRoleId || !birthTypeId) {
                qWarning() << "Could not find Primary role or Birth event type";
                return {};
            }
            const auto newEventId = repo.insertEventWithRelation(*birthTypeId, personId, *primaryRoleId);
            return {.eventId = newEventId ? QVariant(*newEventId) : QVariant{}, .isNew = true};
        } else {
            QVariant birthEventId = birthEventModel->index(0, PersonBirthEventsModel::ID).data();
            return {.eventId = birthEventId, .isNew = false};
        }
    }
}

NewFamilyEditorDialog::NewFamilyEditorDialog(IntegerPrimaryKey personId, QWidget* parent) :
    QDialog(parent),
    form(new Ui::NewFamilyEditorForm) {
    form->setupUi(this);

    data.childId = personId;

    auto [birthEventId, hasNewBirthEvent] = getOrCreateBirthEvent(parent, personId);
    data.hasNewBirthEvent = hasNewBirthEvent;
    Q_ASSERT(birthEventId.isValid());
    data.birthEventId = birthEventId;

    connect(form->motherExistingPerson, &QPushButton::clicked, this, &NewFamilyEditorDialog::onSelectExistingMother);
    connect(form->fatherExistingPerson, &QPushButton::clicked, this, &NewFamilyEditorDialog::onSelectExistingFather);
    connect(form->motherNewPerson, &QPushButton::clicked, this, &NewFamilyEditorDialog::onSelectNewMother);
    connect(form->fatherNewPerson, &QPushButton::clicked, this, &NewFamilyEditorDialog::onSelectNewFather);

    auto* parentRolesModel = new ParentEventRolesListModel(this);

    form->motherRelation->setEnabled(false);
    motherIdMapper = new QDataWidgetMapper(this);
    motherBirthMapper = new QDataWidgetMapper(this);
    motherNameMapper = new QDataWidgetMapper(this);
    motherRelationMapper = new QDataWidgetMapper(this);

    // TODO: Support translatable enums in the QComboxBox.
    // Issue URL: https://github.com/niknetniko/opa/issues/59
    form->motherRelation->setModel(parentRolesModel);
    // TODO: intelligently select a default role.
    // Issue URL: https://github.com/niknetniko/opa/issues/58
    form->motherRelation->setModelColumn(ParentEventRolesListModel::ROLE);

    form->fatherRelation->setEnabled(false);
    fatherIdMapper = new QDataWidgetMapper(this);
    fatherBirthMapper = new QDataWidgetMapper(this);
    fatherNameMapper = new QDataWidgetMapper(this);
    fatherRelationMapper = new QDataWidgetMapper(this);

    form->fatherRelation->setModel(parentRolesModel);
    form->fatherRelation->setModelColumn(ParentEventRolesListModel::ROLE);

    parentRelationMapper = new QDataWidgetMapper(this);
    form->parentRelationRole->setEnabled(false);
    auto* relationshipEventTypes = new RelationshipEventTypesListModel(this);
    form->parentRelationRole->setModel(relationshipEventTypes);
    form->parentRelationRole->setModelColumn(RelationshipEventTypesListModel::TYPE);

    this->setAttribute(Qt::WA_DeleteOnClose);
}

NewFamilyEditorDialog::~NewFamilyEditorDialog() {
    delete form;
}

bool NewFamilyEditorDialog::saveNewFamily() const {
    // TODO: allow linking parents without adding a new marriage.
    // Issue URL: https://github.com/niknetniko/opa/issues/63
    Q_ASSERT(hasActiveTransaction());
    Q_ASSERT(data.birthEventId.isValid());

    EventRepository repo;

    auto chosenMotherId = data.motherId;
    auto chosenFatherId = data.fatherId;

    if (chosenMotherId.isValid() && chosenFatherId.isValid()) {
        auto selectedTypeRow = form->parentRelationRole->currentIndex();
        auto chosenTypeId = form->parentRelationRole->model()->index(selectedTypeRow, RelationshipEventTypesListModel::ID).data().toLongLong();

        auto eventId = repo.insertEvent(chosenTypeId);
        if (!eventId.has_value()) {
            qWarning() << "Could not insert relationship event";
            return false;
        }

        auto primaryId = repo.findEventRoleIdByName(QStringLiteral("Primary"));
        if (!primaryId.has_value() || !repo.insertEventRelation(*eventId, chosenMotherId.toLongLong(), *primaryId)) {
            qWarning() << "Could not insert mother to relationship event";
            return false;
        }

        auto partnerId = repo.findEventRoleIdByName(QStringLiteral("Partner"));
        if (!partnerId.has_value() || !repo.insertEventRelation(*eventId, chosenFatherId.toLongLong(), *partnerId)) {
            qWarning() << "Could not insert father to relationship event";
            return false;
        }
    }

    // Now, link the parents to the child.
    if (chosenMotherId.isValid()) {
        auto motherRole = repo.findEventRoleIdByName(QStringLiteral("Mother"));
        if (!motherRole.has_value() || !repo.insertEventRelation(data.birthEventId.toLongLong(), chosenMotherId.toLongLong(), *motherRole)) {
            qWarning() << "Could not insert mother to birth event relationship";
            return false;
        }
    }

    if (chosenFatherId.isValid()) {
        auto fatherRole = repo.findEventRoleIdByName(QStringLiteral("Father"));
        if (!fatherRole.has_value() || !repo.insertEventRelation(data.birthEventId.toLongLong(), chosenFatherId.toLongLong(), *fatherRole)) {
            qWarning() << "Could not insert father to birth event relationship";
            return false;
        }
    }

    return true;
}

void NewFamilyEditorDialog::accept() {
    if (!QSqlDatabase::database().transaction()) {
        qFatal() << "Could not get transaction on database:";
        qFatal() << QSqlDatabase::database().lastError();
        return;
    }

    if (this->saveNewFamily()) {
        if (!QSqlDatabase::database().commit()) {
            qFatal() << "Could not commit transaction on database:";
            qFatal() << QSqlDatabase::database().lastError();
            return;
        }
        QDialog::accept();
    } else {
        qWarning() << "Something went wrong while saving a family, aborting transaction.";
        if (!QSqlDatabase::database().rollback()) {
            qFatal() << "Could not rollback transaction on database:";
            qFatal() << QSqlDatabase::database().lastError();
        }
    }
}

void NewFamilyEditorDialog::reject() {
    if (data.hasNewBirthEvent) {
        Q_ASSERT(data.birthEventId.isValid());
        const auto birthEventId = data.birthEventId.toLongLong();

        EventRepository repo;
        // Find the default (Primary) role ID and delete that birth event relation.
        const auto primaryRoleId = repo.findEventRoleIdByName(QStringLiteral("Primary"));
        if (primaryRoleId.has_value()) {
            if (!repo.deleteEventRelation(birthEventId, data.childId, *primaryRoleId)) {
                qWarning() << "Could not remove birth event relation";
            }
        } else {
            qWarning() << "Could not find Primary role; birth event relation not removed";
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

    auto* birthEvent = new PersonBirthEventsModel(motherId.toLongLong(), this);
    motherBirthMapper->setModel(birthEvent);
    motherBirthMapper->addMapping(form->motherBirth, PersonBirthEventsModel::DATE, "text");
    motherBirthMapper->toFirst();

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

    auto* birthEvent = new PersonBirthEventsModel(fatherId.toLongLong(), this);
    fatherBirthMapper->setModel(birthEvent);
    fatherBirthMapper->addMapping(form->fatherBirth, PersonBirthEventsModel::DATE, "text");
    fatherBirthMapper->toFirst();

    setParentRelationIfPossible();
}

void NewFamilyEditorDialog::setParentRelationIfPossible() const {
    form->parentRelationRole->setEnabled(data.motherId.isValid() && data.fatherId.isValid());
}
