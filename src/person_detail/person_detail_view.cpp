/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "person_detail_view.h"

#include "../domain/event/event_types.h"
#include "../domain/name/names.h"
#include "../domain/person/person_sex.h"
#include "core/data_event_broker.h"
#include "database/schema.h"
#include "dates/genealogical_date.h"
#include "domain/event/person_birth_events_model.h"
#include "domain/event/person_death_events_model.h"
#include "domain/person/person_repository.h"
#include "person_event_tab.h"
#include "person_family_tab.h"
#include "person_name_tab.h"
#include "ui_person_detail_view.h"
#include "utils/formatted_identifier_delegate.h"

#include <KLocalizedString>
#include <QDate>

BirthInformation constructBirthText(const QAbstractItemModel* model) {
    if (model->rowCount() == 0) {
        return {};
    }

    auto dateString = model->index(0, PersonBirthEventsModel::DATE).data().toString();
    auto type = enumFromString<EventTypes::Values>(model->index(0, PersonBirthEventsModel::TYPE).data().toString());

    QString symbol = {};
    if (type == EventTypes::Values::Birth) {
        symbol = QStringLiteral("*");
    } else if (type == EventTypes::Values::Baptism) {
        symbol = QStringLiteral("~");
    } else {
        qFatal() << "Unknown birth event type encountered" << type;
        Q_UNREACHABLE();
    }

    return {.symbol = symbol, .date = dateString};
}

DeathInformation constructDeathText(const QAbstractItemModel* birthModel, const QAbstractItemModel* deathModel) {
    QString symbol;
    QString string;
    GenealogicalDate deathDate;

    if (deathModel->rowCount() != 0) {
        qDebug() << "There is a death event";
        auto index = deathModel->index(0, PersonDeathEventsModel::DATE);
        deathDate = GenealogicalDate::fromDatabaseRepresentation(
            deathModel->index(0, PersonDeathEventsModel::DATE_RAW).data().toString()
        );
        if (!deathDate.isValid()) {
            return {.symbol = QStringLiteral("✝︎"), .date = i18n("unknown date")};
        }
        string = index.data().toString();
        auto type =
            enumFromString<EventTypes::Values>(deathModel->index(0, PersonDeathEventsModel::TYPE).data().toString());

        if (type == EventTypes::Values::Death) {
            symbol = QStringLiteral("✝︎");
        } else if (type == EventTypes::Values::Funeral) {
            symbol = QStringLiteral("⚰︎");
        } else {
            qFatal() << "Unknown birth event type encountered" << type;
            Q_UNREACHABLE();
        }
    } else {
        deathDate = GenealogicalDate(
            GenealogicalDate::NONE, GenealogicalDate::ESTIMATED, QDate::currentDate(), true, true, true, {}
        );
    }

    QString ageText;
    if (birthModel->rowCount() != 0) {
        auto birthDate = GenealogicalDate::fromDatabaseRepresentation(
            birthModel->index(0, PersonBirthEventsModel::DATE_RAW).data().toString()
        );
        auto birthProleptic = birthDate.prolepticRepresentation();
        auto deathProleptic = deathDate.prolepticRepresentation();
        auto ageInDays = birthProleptic.daysTo(deathProleptic);

        if (string.isEmpty()) {
            symbol = QStringLiteral("✝︎");
            string = i18n("alive");
        }
        // We intentionally use 365 for age calculations, since that is how most people determine the age.
        // Leap years are typically not accounted for.
        auto ageInYears = ageInDays / 365;
        if (ageInYears != 0) {
            ageText = i18np("%1 year", "%1 years", ageInYears);
        } else {
            // See if we can calculate if there are a few months.
            // For simplicity, use 30 as the number of days in a month.
            auto ageInMonths = ageInDays / 30;
            if (ageInMonths != 0) {
                ageText = i18np("%1 month", "%1 months", ageInMonths);
            } else {
                auto ageInWeeks = ageInDays / 7;
                if (ageInWeeks != 0) {
                    ageText = i18np("%1 week", "1% weeks", ageInWeeks);
                } else {
                    // Display the number of days.
                    ageText = i18np("%1 day", "%1 days", ageInDays);
                }
            }
        }
    }

    if (!ageText.isEmpty()) {
        if (!string.isEmpty()) {
            string += QStringLiteral(" ");
        }
        string += QStringLiteral("(%1)").arg(ageText);
    }

    return {.symbol = symbol, .date = string};
}

PersonDetailView::PersonDetailView(IntegerPrimaryKey id, QWidget* parent) : QFrame(parent), id(id) {
    this->ui = new Ui::PersonDetailView();
    ui->setupUi(this);

    // Load person data from repository.
    PersonRepository repo;
    this->personData = repo.findDisplayById(id);

    this->birthModel = new PersonBirthEventsModel(id, this);
    this->deathModel = new PersonDeathEventsModel(id, this);
    this->populateName();
    this->populateDates();

    // Create a tab for family.
    auto* familyTab = new PersonFamilyTab(id, ui->tabWidget);
    ui->tabWidget->addTab(familyTab, i18n("Relationships"));

    // Create tab for events
    auto* eventTab = new PersonEventTab(id, ui->tabWidget);
    ui->tabWidget->addTab(eventTab, i18n("Events"));

    // Create tab for names
    auto* nameTab = new PersonNameTab(id, ui->tabWidget);
    ui->tabWidget->addTab(nameTab, i18n("Names"));

    // Listen for person/name changes to refresh display.
    connectToTable<Schema::People>(this, [this](std::optional<IntegerPrimaryKey> changedId) {
        if (!changedId || changedId == this->id) {
            PersonRepository r;
            this->personData = r.findDisplayById(this->id);
            populateName();
        }
    });
    connectToTable<Schema::Names>(this, [this](std::optional<IntegerPrimaryKey> changedId) {
        if (!changedId || changedId == this->id) {
            PersonRepository r;
            this->personData = r.findDisplayById(this->id);
            populateName();
        }
    });
    connect(birthModel, &QAbstractItemModel::dataChanged, this, &PersonDetailView::populateDates);
    connect(deathModel, &QAbstractItemModel::dataChanged, this, &PersonDetailView::populateDates);
    connect(birthModel, &QAbstractItemModel::modelReset, this, &PersonDetailView::populateDates);
    connect(deathModel, &QAbstractItemModel::modelReset, this, &PersonDetailView::populateDates);
}

void PersonDetailView::populateName() {
    auto personId = format_id(FormattedIdentifierDelegate::PERSON, id);
    ui->id->setText(personId);
    ui->displayName->setText(this->getDisplayName());

    QString sex;
    if (personData.has_value()) {
        sex = personData->sex;
    }
    auto sexSymbol = Sex::toIcon(sex);
    auto sexDescription = Sex::toDisplayString(sex);
    ui->sexIcon->setText(sexSymbol);
    ui->sexIcon->setToolTip(sexDescription);

    Q_EMIT this->nameChanged(id);
}

void PersonDetailView::populateDates() const {
    auto [birthSymbol, birthDate] = constructBirthText(birthModel);
    auto* layout = ui->formLayout;
    layout->setRowVisible(1, !birthSymbol.isEmpty() || !birthDate.isEmpty());
    ui->birthLabel->setText(birthSymbol);
    ui->birth->setText(birthDate);

    auto [deathSymbol, deathDate] = constructDeathText(birthModel, deathModel);
    layout->setRowVisible(2, !deathSymbol.isEmpty() || !deathDate.trimmed().isEmpty());
    ui->deathLabel->setText(deathSymbol);
    ui->death->setText(deathDate);
}

PersonDetailView::~PersonDetailView() {
    delete ui;
}

bool PersonDetailView::hasId(IntegerPrimaryKey id) const {
    return getId() == id;
}

QString PersonDetailView::getDisplayName() const {
    QString name;
    if (personData.has_value()) {
        name =
            construct_display_name(personData->titles, personData->givenNames, personData->prefix, personData->surname);
    }
    auto personId = format_id(FormattedIdentifierDelegate::PERSON, id);
    return QStringLiteral("%1 [%2]").arg(name, personId);
}

IntegerPrimaryKey PersonDetailView::getId() const {
    return id;
}
