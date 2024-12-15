/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "new_family_editor_dialog.h"

#include "data/data_manager.h"
#include "data/event.h"
#include "data/person.h"
#include "link_existing/choose_existing_person_window.h"
#include "new_person_editor_dialog.h"
#include "ui_new_family_editor_dialog.h"
#include "utils/builtin_text_translating_delegate.h"
#include "utils/formatted_identifier_delegate.h"
#include "welcome/welcome_view.h"

#include <QCompleter>
#include <QDataWidgetMapper>
#include <QSqlError>

struct BirthEventDetails {
    QVariant eventId;
    bool isNew;
};

static BirthEventDetails getOrCreateBirthEvent(QObject* parent, IntegerPrimaryKey personId) {
    auto* birthEventModel = DataManager::get().birthEventModelForPerson(parent, personId);
    if (birthEventModel->rowCount() == 0) {
        auto newEvent = addEventToPerson(EventTypes::Birth, personId);
        return {newEvent.eventId, true};
    } else {
        QVariant birthEventId = birthEventModel->index(0, PersonEventsModel::ID).data();
        return {birthEventId, false};
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

    auto parentRolesModel = DataManager::get().parentEventRolesModel(this);

    form->motherRelation->setEnabled(false);
    motherIdMapper = new QDataWidgetMapper(this);
    motherBirthMapper = new QDataWidgetMapper(this);
    motherNameMapper = new QDataWidgetMapper(this);
    motherRelationMapper = new QDataWidgetMapper(this);

    // TODO: Support translatable enums in the QComboxBox.
    form->motherRelation->setModel(parentRolesModel);
    // TODO: intelligently select a default role.
    form->motherRelation->setModelColumn(EventRolesModel::ROLE);

    form->fatherRelation->setEnabled(false);
    fatherIdMapper = new QDataWidgetMapper(this);
    fatherBirthMapper = new QDataWidgetMapper(this);
    fatherNameMapper = new QDataWidgetMapper(this);
    fatherRelationMapper = new QDataWidgetMapper(this);

    form->fatherRelation->setModel(parentRolesModel);
    form->fatherRelation->setModelColumn(EventRolesModel::ROLE);

    parentRelationMapper = new QDataWidgetMapper(this);

    this->setAttribute(Qt::WA_DeleteOnClose);
}

NewFamilyEditorDialog::~NewFamilyEditorDialog() {
    delete form;
}

void NewFamilyEditorDialog::accept() {
    QDialog::accept();
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
}
