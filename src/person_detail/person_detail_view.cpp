#include <QSqlQuery>
#include <QDebug>
#include <QSqlError>
#include <QSqlField>
#include <QToolBar>

#include "person_detail_view.h"
#include "ui_person_detail_view.h"
#include "data/person.h"
#include "event/event_table_view.h"

PersonDetailView::PersonDetailView(long long id, QWidget *parent) :
    QFrame(parent),
    id(id),
    ui(new Ui::PersonDetailView)
{
    ui->setupUi(this);
}

void PersonDetailView::populate() {
    ui->id->setText(QString("P%1").arg(this->id, 4, 10, QLatin1Char('0')));

    // Get the person from the database.
    QSqlQuery query;
    query.prepare("SELECT * FROM person WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qCritical() << "Error:" << query.lastError();
    }

    query.first();
    this->record = query.record();

    auto name = record.field(Data::Person::Table::GIVEN_NAMES).value().toString();
    ui->displayName->setText(name);

    // Show details.
    // This is manually created...
//    ui->eventTab->setWid
    auto *eventTabContainer = ui->eventTab->layout();
    eventTabContainer->setSpacing(0);

    auto *eventToolbar = new QToolBar(ui->eventTab);
//    eventToolbar->layout()->setMargin(0);
    eventToolbar->setMovable(false);
    eventToolbar->setFloatable(false);
    auto* action = new QAction(eventToolbar);
    action->setText("Add Event");
    action->setIcon(QIcon::fromTheme("list-add"));
    eventToolbar->addAction(action);
    eventTabContainer->addWidget(eventToolbar);

    auto *tableView = new EventTableView(this->id, ui->eventTab);
    eventTabContainer->addWidget(tableView);

    emit this->personNameChanged(this->id, name);
}

PersonDetailView::~PersonDetailView()
{
    delete ui;
}
