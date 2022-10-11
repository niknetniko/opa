//
// Created by niko on 2/09/2022.
//
#include "person_detail_view.h"

#include <QSqlQuery>
#include <QDebug>
#include <QSqlError>
#include <QSqlField>
#include <QVBoxLayout>

#include "data/person.h"
#include "event/event_table_view.h"

PersonDetailView::PersonDetailView(int id, QWidget* parent): QWidget(parent), id(id) {
    qDebug() << "Created new instance...";
}

void PersonDetailView::populate() {
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
    auto* displayName = new QLabel(name, this);

    // Show details.
    auto *tableView = new EventTableView(this->id, this);
    tableView->show();

    auto* layout = new QVBoxLayout(this);
//    layout->setAlignment(Qt::AlignTop);
    layout->addWidget(displayName);
    layout->addWidget(tableView);

    emit this->personNameChanged(this->id, name);
}
