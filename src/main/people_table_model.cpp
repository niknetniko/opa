//
// Created by niko on 9/04/2022.
//

#include <KLocalizedString>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QColor>
#include <QSqlRecord>
#include <QSqlField>
#include "people_table_model.h"

#include "data/person.h"
#include "names.h"

const int SqlColumnRole = Qt::UserRole + 1;

BasicPeopleTableModel::BasicPeopleTableModel(QObject *parent) : QSqlQueryModel(parent),
                                                                sortOrder(Qt::SortOrder::AscendingOrder),
                                                                sortColumn(Data::Person::Table::GIVEN_NAMES) {
    this->regenerateQuery();
    // TODO: fix this and make it proper.
    while (this->canFetchMore()) { this->fetchMore(); }
}

QVariant BasicPeopleTableModel::data(const QModelIndex &item, int role) const {
    int originalRole = role;

    // Some roles are not implemented in the parent class, so ignore those.
    if (role == Qt::UserRole) {
        role = Qt::DisplayRole;
    }

    auto value = QSqlQueryModel::data(item, role);

    // Modify the data itself.
    if (originalRole == Qt::DisplayRole) {
        if (!value.isValid()) {
            return value;
        }
        if (record().fieldName(item.column()) == Data::Person::Table::ID) {
            // Format the ID.
            return QString::fromUtf8("P%1").arg(value.toULongLong(), 4, 10, QLatin1Char('0'));
        }
    }

    return value;
}

void BasicPeopleTableModel::sort(int column, Qt::SortOrder order) {
    qDebug() << "Sorting called... " << column;
    if (column > 0) {
        column = 1; // We only support sorting column 0 or 1 for now.
    }
    if (column == 0) {
        this->sortColumn = QString::fromUtf8("people.id");
    } else if (column == 1) {
        this->sortColumn = QString::fromUtf8("names.given_names");
    }
    this->sortOrder = order;
    qDebug("New sort column is %s", qPrintable(this->sortColumn));
    this->regenerateQuery();
}

void BasicPeopleTableModel::onSearchChanged(const QString &text) {
    this->searchQuery = text;
    this->regenerateQuery();
}

void BasicPeopleTableModel::regenerateQuery() {
    QString queryString = QString::fromUtf8("SELECT people.id, names.titles, names.given_names, names.prefix, names.surname FROM people JOIN names on people.id = names.person_id WHERE (names.main IS TRUE)");

    if (!this->searchQuery.isEmpty()) {
        queryString += QString::fromUtf8(" AND (names.given_names LIKE '%") + this->searchQuery + QString::fromUtf8("%')");
    }

    QString sortString;
    switch (this->sortOrder) {
        case Qt::SortOrder::AscendingOrder:
            sortString = QString::fromUtf8("ASC");
            break;
        case Qt::SortOrder::DescendingOrder:
            sortString = QString::fromUtf8("DESC");
            break;
    }

    queryString += QString::fromUtf8(" ORDER BY ") + this->sortColumn + QString::fromUtf8(" ") + sortString;

    qDebug() << queryString;

    this->setQuery(queryString);
}


AdditionalColumnModel::AdditionalColumnModel(QObject *parent) : KExtraColumnsProxyModel(parent) {
    qDebug("Appending additional column...");
    this->setSourceModel(new BasicPeopleTableModel(this));
    this->appendColumn(i18n("Naam"));
}

QVariant AdditionalColumnModel::extraColumnData(const QModelIndex &parent, int row, int  /*extraColumn*/, int role) const {
    auto *source = this->sourceModel();
    auto titles = source->index(row, 1, parent).data(role).toString();
    auto givenNames = source->index(row, 2, parent).data(role).toString();
    auto prefix = source->index(row, 3, parent).data(role).toString();
    auto surname = source->index(row, 4, parent).data(role).toString();
    return Names::construct_display_name(titles, givenNames, prefix, surname);
}

void AdditionalColumnModel::onSearchChanged(const QString &text) {
    auto model = static_cast<BasicPeopleTableModel>(this->sourceModel());
    model.onSearchChanged(text);
}

PeopleTableModel::PeopleTableModel(QObject *parent) : KRearrangeColumnsProxyModel(parent) {
    this->setSourceModel(new AdditionalColumnModel(this));
    setSourceColumns(QVector<int>() << 0 << this->sourceModel()->columnCount() - 1);
    this->setHeaderData(0, Qt::Horizontal, i18n("ID"));
    // Find out why this won't work...
//    this->setHeaderData(1, Qt::Horizontal, i18n("Naam"), Qt::DisplayRole);
}

void PeopleTableModel::onSearchChanged(const QString &text) {
    auto model = static_cast<AdditionalColumnModel>(this->sourceModel());
    model.onSearchChanged(text);
}
