//
// Created by niko on 9/04/2022.
//

#include <KLocalizedString>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QColor>
#include "people_list_model.h"

PeopleListModel::PeopleListModel(QObject *parent) : QSqlQueryModel(parent) {
    this->setQuery("SELECT person.id, name, color FROM person LEFT JOIN roots ON person.id = roots.person;");
    this->setHeaderData(0, Qt::Horizontal, i18n("ID"));
    this->setHeaderData(1, Qt::Horizontal, i18n("Name"));
}

QVariant PeopleListModel::data(const QModelIndex &item, int role) const {
    auto value = QSqlQueryModel::data(item, role);

    // Modify the data itself.
    if (role == Qt::DisplayRole) {
        if (!value.isValid()) {
            return value;
        }
        if (item.column() == 0) {
            // Format the ID.
            return QString("P%1").arg(value.toULongLong(), 4, 10, QLatin1Char('0'));
        }
    }

    // Modify the text colour.
    if (role == Qt::ForegroundRole) {
        // Modify from every row.
        auto colorIndex = item.siblingAtColumn(2);
        auto colorValue = QSqlQueryModel::data(colorIndex, Qt::ItemDataRole::DisplayRole);
        QColor color;
        color.setNamedColor(colorValue.toString());
        return QVariant::fromValue(color);
    }

    return value;
}

void PeopleListModel::sort(int column, Qt::SortOrder order) {
    QString columnName = this->headerData(column, Qt::Horizontal).toString();
    QString sortOrder;
    switch (order) {
        case Qt::SortOrder::AscendingOrder:
            sortOrder = "ASC";
            break;
        case Qt::SortOrder::DescendingOrder:
            sortOrder = "DESC";
            break;
    }
    auto query =
            "SELECT person.id, name, color FROM person LEFT JOIN roots ON person.id = roots.person ORDER BY person." +
            columnName + " " + sortOrder;
    this->setQuery(query);
}


