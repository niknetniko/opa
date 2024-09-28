#include <QSqlQuery>
#include <QDebug>
#include <QSqlError>
#include <QSqlField>
#include <QToolBar>
#include <QDate>
#include <KLocalizedString>

#include "person_detail_view.h"
#include "ui_person_detail_view.h"
#include "data/person.h"
#include "event/event_table_view.h"
#include "data/event.h"
#include "database/schema.h"
#include "names/names_table_view.h"
#include "person_name_tab.h"

PersonDetailView::PersonDetailView(long long id, QWidget *parent) :
        QFrame(parent),
        id(id),
        ui(new Ui::PersonDetailView) {
    ui->setupUi(this);
}

void PersonDetailView::populateBirth() {
//    QSqlQuery query;
//    query.prepare(QString::fromUtf8("SELECT * FROM event WHERE person = :id AND type = 'birth'"));
//    query.bindValue(QString::fromUtf8(":id"), id);
//    if (!query.exec()) {
//        qCritical() << "Could not get birth event..." << query.lastError();
//    }
//    if (query.first()) {
//        auto eventRecord = query.record();
//        auto birthdate = eventRecord.field(Data::Event::Table::DATE).value().toDate();
//        auto formatted = QLocale().toString(birthdate,QLocale::LongFormat);
//        ui->birth->setText(formatted);
//
//        // Calculate the age of the person.
//        auto days = birthdate.daysTo(QDate::currentDate());
//        auto years = days / 365;
//        ui->death->setText(QString::number(years) + QString::fromUtf8(" dagen"));
//    } else {
        ui->birth->setText(QString::fromUtf8("?"));
//    }
}

void PersonDetailView::populate() {
    ui->id->setText(QString::fromUtf8("P%1").arg(this->id, 4, 10, QLatin1Char('0')));

    // Get the person from the database.
    QSqlQuery query;
    query.prepare(QString::fromUtf8("SELECT * FROM people JOIN names ON people.id = names.person_id WHERE people.id = :id"));
    query.bindValue(QString::fromUtf8(":id"), id);

    if (!query.exec()) {
        qCritical() << "Error:" << query.lastError();
    }

    query.first();
    this->record = query.record();

    auto titles = record.field(Schema::Names::titles).value().toString();
    auto given_names = record.field(Schema::Names::givenNames).value().toString();
    auto prefix = record.field(Schema::Names::prefix).value().toString();
    auto surname = record.field(Schema::Names::surname).value().toString();
    auto name = Names::construct_display_name(titles, given_names, prefix, surname);
    ui->displayName->setText(name);

    auto sex = record.field(Data::Person::Table::SEX).value().toString();
    auto sexSymbol = Data::Person::Sex::toIcon(sex);
    auto sexDescription = Data::Person::Sex::toDisplay(sex);
    ui->sexIcon->setText(sexSymbol);
    ui->sexIcon->setToolTip(sexDescription);

    auto *tabWidget = ui->tabWidget;

    // Create tab for events
//    auto *evenTabContainer = new QWidget();
//    auto *evenTabContainerLayout = new QVBoxLayout(evenTabContainer);
//    evenTabContainerLayout->setSpacing(0);
//    auto *eventToolbar = new QToolBar(evenTabContainer);
//    auto *action = new QAction(eventToolbar);
//    action->setText(tr("Add Event"));
//    action->setIcon(QIcon::fromTheme(QString::fromUtf8("list-add")));
//    eventToolbar->addAction(action);
//    evenTabContainerLayout->addWidget(eventToolbar);
//    auto *treeView = new EventTableView(this->id, evenTabContainer);
//    evenTabContainerLayout->addWidget(treeView);
//    tabWidget->addTab(evenTabContainer, i18n("Events"));


    // Create tab for names
    auto *nameTab = new PersonNameTab(this->id, tabWidget);
    tabWidget->addTab(nameTab, i18n("Namen"));

    this->populateBirth();

    Q_EMIT(this->personNameChanged(this->id, name));
}

PersonDetailView::~PersonDetailView() {
    delete ui;
}
