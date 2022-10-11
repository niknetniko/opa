//
// Created by niko on 9/04/2022.
//

#include <KLocalizedString>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QColor>
#include "people_table_model.h"

#include "data/person.h"

PeopleTableModel::PeopleTableModel(QObject *parent) : QSqlQueryModel(parent), sortOrder(Qt::SortOrder::AscendingOrder),
                                                      sortColumn(Data::Person::Table::GIVEN_NAMES) {
    this->regenerateQuery();
}

QVariant PeopleTableModel::data(const QModelIndex &item, int role) const {
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
        if (item.column() == 0) {
            // Format the ID.
            return QString("P%1").arg(value.toULongLong(), 4, 10, QLatin1Char('0'));
        }
    }

    // Modify the text colour.
    if (originalRole == Qt::ForegroundRole) {
        // Modify from every row.
        auto colorIndex = item.siblingAtColumn(2);
        auto colorValue = QSqlQueryModel::data(colorIndex, Qt::ItemDataRole::DisplayRole);
        QColor color;
        color.setNamedColor(colorValue.toString());
        return QVariant::fromValue(color);
    }

    return value;
}

void PeopleTableModel::sort(int column, Qt::SortOrder order) {
    qDebug() << "Sorting called...";
    this->sortColumn = this->headerData(column, Qt::Horizontal).toString();
    this->sortOrder = order;
    this->regenerateQuery();
}

void PeopleTableModel::onSearchChanged(const QString &text) {
    this->searchQuery = text;
    this->regenerateQuery();
}

void PeopleTableModel::regenerateQuery() {
    QString queryString = "SELECT person.id, person.given_names, color FROM person LEFT JOIN roots ON person.id = roots.person";

    if (!this->searchQuery.isEmpty()) {
        queryString += " WHERE person.given_names LIKE '%" + this->searchQuery + "%'";
    }

    QString sortString;
    switch (this->sortOrder) {
        case Qt::SortOrder::AscendingOrder:
            sortString = "ASC";
            break;
        case Qt::SortOrder::DescendingOrder:
            sortString = "DESC";
            break;
    }

    queryString += " ORDER BY person." + this->sortColumn  + " " + sortString;

    qDebug() << queryString;

    this->setQuery(queryString);
}


