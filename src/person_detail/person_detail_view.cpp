/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "person_detail_view.h"

#include "data/data_manager.h"
#include "data/event.h"
#include "data/person.h"
#include "database/schema.h"
#include "person_event_tab.h"
#include "person_family_tab.h"
#include "person_name_tab.h"
#include "ui_person_detail_view.h"
#include "utils/formatted_identifier_delegate.h"

#include <KLocalizedString>
#include <QDate>
#include <QSqlQuery>

PersonDetailView::PersonDetailView(IntegerPrimaryKey id, QWidget* parent) : QFrame(parent) {
    this->ui = new Ui::PersonDetailView();
    ui->setupUi(this);

    this->model = DataManager::get().personDetailsModel(this, id);
    this->populate();

    // Create tab for events
    auto* eventTab = new PersonEventTab(id, ui->tabWidget);
    ui->tabWidget->addTab(eventTab, i18n("Events"));

    // Create a tab for family.
    auto* familyTab = new PersonFamilyTab(id, ui->tabWidget);
    ui->tabWidget->addTab(familyTab, i18n("Family"));

    // Create tab for names
    auto* nameTab = new PersonNameTab(id, ui->tabWidget);
    ui->tabWidget->addTab(nameTab, i18n("Names"));

    // Connect the model to this view, so we update when the data is changed.
    connect(this->model, &QAbstractProxyModel::dataChanged, this, &PersonDetailView::populate);
}

void PersonDetailView::populate() {
    assert(model->rowCount() == 1);

    auto rawPersonId = model->index(0, PersonDetailModel::ID).data();
    auto personId = format_id(FormattedIdentifierDelegate::PERSON, rawPersonId);
    ui->id->setText(personId);
    ui->displayName->setText(this->getDisplayName());

    auto sex = model->index(0, PersonDetailModel::SEX).data().toString();
    auto sexSymbol = Sex::toIcon(sex);
    auto sexDescription = Sex::toDisplayString(sex);
    ui->sexIcon->setText(sexSymbol);
    ui->sexIcon->setToolTip(sexDescription);

    Q_EMIT this->dataChanged(rawPersonId.toLongLong());
}

PersonDetailView::~PersonDetailView() {
    delete ui;
}

bool PersonDetailView::hasId(IntegerPrimaryKey id) const {
    return model->index(0, PersonDetailModel::ID).data() == id;
}

QString PersonDetailView::getDisplayName() const {
    return model->index(0, PersonDetailModel::DISPLAY_NAME).data().toString();
}
