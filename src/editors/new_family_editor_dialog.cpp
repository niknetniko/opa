/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "new_family_editor_dialog.h"

#include "data/data_manager.h"
#include "data/event.h"
#include "data/person.h"
#include "database/database.h"
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
        auto* birthEventModel = DataManager::get().birthEventModelForPerson(parent, personId);
        if (birthEventModel->rowCount() == 0) {
            auto newEvent = addEventToPerson(EventTypes::Birth, personId);
            return {newEvent.eventId, true};
        } else {
            QVariant birthEventId = birthEventModel->index(0, PersonEventsModel::ID).data();
            return {birthEventId, false};
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

    auto* parentRolesModel = DataManager::get().parentEventRolesModel(this);

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
    form->motherRelation->setModelColumn(EventRolesModel::ROLE);

    form->fatherRelation->setEnabled(false);
    fatherIdMapper = new QDataWidgetMapper(this);
    fatherBirthMapper = new QDataWidgetMapper(this);
    fatherNameMapper = new QDataWidgetMapper(this);
    fatherRelationMapper = new QDataWidgetMapper(this);

    form->fatherRelation->setModel(parentRolesModel);
    form->fatherRelation->setModelColumn(EventRolesModel::ROLE);

    parentRelationMapper = new QDataWidgetMapper(this);
    form->parentRelationRole->setEnabled(false);
    auto* relationshipEventTypes = DataManager::get().relationshipEventTypes(this);
    form->parentRelationRole->setModel(relationshipEventTypes);
    form->parentRelationRole->setModelColumn(EventTypesModel::TYPE);

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

    auto* eventRelationModel = DataManager::get().eventRelationsModel();

    auto chosenMotherId = data.motherId;
    auto chosenFatherId = data.fatherId;

    if (chosenMotherId.isValid() && chosenFatherId.isValid()) {
        auto* eventModel = DataManager::get().eventsModel();

        auto selectedTypeRow = form->parentRelationRole->currentIndex();
        auto chosenType = form->parentRelationRole->model()->index(selectedTypeRow, EventTypesModel::ID).data();
        auto record = eventModel->record();
        record.setGenerated(EventsModel::ID, false);
        record.setValue(EventsModel::TYPE_ID, chosenType);

        if (!eventModel->insertRecord(-1, record)) {
            qWarning() << "Could not insert relationship model:";
            qWarning() << eventModel->lastError();
            return false;
        }

        auto eventId = eventModel->query().lastInsertId();
        if (!eventId.isValid()) {
            qWarning() << "Could not get inserted relationship model:";
            qWarning() << eventModel->lastError();
            return false;
        }

        auto primaryId = EventRolesModel::getRoleId(EventRoles::Primary);
        auto motherRelationshipRecord = eventRelationModel->record();
        motherRelationshipRecord.setValue(EventRelationsModel::PERSON_ID, chosenMotherId);
        motherRelationshipRecord.setValue(EventRelationsModel::EVENT_ID, eventId);
        motherRelationshipRecord.setValue(EventRelationsModel::ROLE_ID, primaryId);

        if (!eventRelationModel->insertRecord(-1, motherRelationshipRecord)) {
            qWarning() << "Could not insert mother to father event relationship:";
            qWarning() << eventRelationModel->lastError();
            return false;
        }

        auto partnerId = EventRolesModel::getRoleId(EventRoles::Partner);
        auto fatherRelationshipRecord = eventRelationModel->record();
        fatherRelationshipRecord.setValue(EventRelationsModel::PERSON_ID, chosenFatherId);
        fatherRelationshipRecord.setValue(EventRelationsModel::EVENT_ID, eventId);
        fatherRelationshipRecord.setValue(EventRelationsModel::ROLE_ID, partnerId);

        if (!eventRelationModel->insertRecord(-1, fatherRelationshipRecord)) {
            qWarning() << "Could not insert father to mother event relationship:";
            qWarning() << eventRelationModel->lastError();
            return false;
        }
    }

    // Now, link the parents to the child.
    if (chosenMotherId.isValid()) {
        auto motherRole = EventRolesModel::getRoleId(EventRoles::Mother);
        auto newRecord = eventRelationModel->record();
        newRecord.setValue(EventRelationsModel::PERSON_ID, chosenMotherId);
        newRecord.setValue(EventRelationsModel::EVENT_ID, data.birthEventId);
        newRecord.setValue(EventRelationsModel::ROLE_ID, motherRole);

        if (!eventRelationModel->insertRecord(-1, newRecord)) {
            qWarning() << "Could not insert mother to birth event relationship:";
            qWarning() << eventRelationModel->lastError();
            return false;
        }
    }

    if (chosenFatherId.isValid()) {
        auto fatherRole = EventRolesModel::getRoleId(EventRoles::Father);
        auto newRecord = eventRelationModel->record();
        newRecord.setValue(EventRelationsModel::PERSON_ID, chosenFatherId);
        newRecord.setValue(EventRelationsModel::EVENT_ID, data.birthEventId);
        newRecord.setValue(EventRelationsModel::ROLE_ID, fatherRole);

        if (!eventRelationModel->insertRecord(-1, newRecord)) {
            qWarning() << "Could not insert father to birth event relationship:";
            qWarning() << eventRelationModel->lastError();
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
        // Delete the birth event relation.
        auto* erm = DataManager::get().eventRelationModelByPersonAndEvent(this, data.childId, data.birthEventId);
        qDebug() << "Found birth events:" << erm->rowCount();
        Q_ASSERT(erm->rowCount() == 1);
        Q_ASSERT(erm->index(0, EventRelationsModel::ROLE_ID).data() == EventRolesModel::getDefaultRole());
        if (!erm->removeRow(0)) {
            qWarning() << "Could not remove birth event relation";
            qWarning() << findSourceModelOfType<QSqlQueryModel>(erm)->lastError();
        }

        auto* em = DataManager::get().eventsModel();
        auto eventMatch = em->match(em->index(0, EventsModel::ID), Qt::DisplayRole, data.birthEventId);
        Q_ASSERT(eventMatch.size() == 1);
        if (!em->removeRow(eventMatch.constFirst().row())) {
            qWarning() << "Could not remove birth event";
            qWarning() << em->lastError();
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

    auto* motherDetailsModel = DataManager::get().personDetailsModel(this, motherId.toLongLong());
    motherIdMapper->setModel(motherDetailsModel);
    motherIdMapper->addMapping(form->motherId, PersonDetailModel::ID, "text");
    motherIdMapper->setItemDelegate(new FormattedIdentifierDelegate(this, FormattedIdentifierDelegate::PERSON));
    motherIdMapper->toFirst();

    motherNameMapper->setModel(motherDetailsModel);
    motherNameMapper->addMapping(form->motherName, PersonDetailModel::DISPLAY_NAME, "text");
    motherNameMapper->toFirst();

    auto* birthEvent = DataManager::get().birthEventModelForPerson(this, motherId.toLongLong());
    motherBirthMapper->setModel(birthEvent);
    motherBirthMapper->addMapping(form->motherBirth, PersonEventsModel::DATE, "text");
    motherBirthMapper->toFirst();

    setParentRelationIfPossible();
}

void NewFamilyEditorDialog::setFather(const QVariant& fatherId) {
    if (!fatherId.isValid()) {
        return;
    }
    form->fatherRelation->setEnabled(true);
    data.fatherId = fatherId;

    auto* fatherDetailsModel = DataManager::get().personDetailsModel(this, fatherId.toLongLong());
    fatherIdMapper->setModel(fatherDetailsModel);
    fatherIdMapper->addMapping(form->fatherId, PersonDetailModel::ID, "text");
    fatherIdMapper->setItemDelegate(new FormattedIdentifierDelegate(this, FormattedIdentifierDelegate::PERSON));
    fatherIdMapper->toFirst();

    fatherNameMapper->setModel(fatherDetailsModel);
    fatherNameMapper->addMapping(form->fatherName, PersonDetailModel::DISPLAY_NAME, "text");
    fatherNameMapper->toFirst();

    auto* birthEvent = DataManager::get().birthEventModelForPerson(this, fatherId.toLongLong());
    fatherBirthMapper->setModel(birthEvent);
    fatherBirthMapper->addMapping(form->fatherBirth, PersonEventsModel::DATE, "text");
    fatherBirthMapper->toFirst();

    setParentRelationIfPossible();
}

void NewFamilyEditorDialog::setParentRelationIfPossible() const {
    form->parentRelationRole->setEnabled(data.motherId.isValid() && data.fatherId.isValid());
}
