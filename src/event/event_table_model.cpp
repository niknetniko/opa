//
// Created by niko on 9/04/2022.
//

#include <KLocalizedString>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QSqlRecord>
#include <QSqlField>
#include <QDate>
#include <QString>
#include "event_table_model.h"

#include "data/event.h"

EventTableModel::EventTableModel(long long personId, QObject *parent) : QSqlQueryModel(parent), personId(personId),
                                                                  sortOrder(Qt::SortOrder::AscendingOrder),
                                                                  sortColumn(Data::Event::Table::DATE) {
    this->regenerateQuery();
}

QVariant EventTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    // Some roles are not implemented in the parent class, so ignore those.
    if (role == Qt::UserRole) {
        role = Qt::DisplayRole;
    }

    auto value = QSqlQueryModel::headerData(section, orientation, role);

    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            if (record().fieldName(section) == Data::Event::Table::ID) {
                return i18n("ID");
            }
            if (record().fieldName(section) == Data::Event::Table::DATE) {
                return i18n("Date");
            }
            if (record().fieldName(section) == Data::Event::Table::TYPE) {
                return i18n("Type");
            }
        }
    }

    return value;
}

QVariant EventTableModel::data(const QModelIndex &item, int role) const {
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
        if (record().fieldName(item.column()) == Data::Event::Table::ID) {
            // Format the ID.
            return QString::fromUtf8("E%1").arg(value.toULongLong(), 4, 10, QLatin1Char('0'));
        }
        if (record().fieldName(item.column()) == Data::Event::Table::DATE) {
            // Format the date.
            return QLocale().toString(value.toDate(), QLocale::LongFormat);
        }
        if (record().fieldName(item.column()) == Data::Event::Table::TYPE) {
            // Format the date.
            return Data::Event::Table::typeToDisplay(value.toString());
        }
    }

    // Modify the text colour.
//    if (originalRole == Qt::ForegroundRole) {
//        // Modify from every row.
//        auto colorIndex = item.siblingAtColumn(2);
//        auto colorValue = QSqlQueryModel::data(colorIndex, Qt::ItemDataRole::DisplayRole);
//        QColor color;
//        color.setNamedColor(colorValue.toString());
//        return QVariant::fromValue(color);
//    }

    return value;
}

void EventTableModel::sort(int column, Qt::SortOrder order) {
    this->sortColumn = this->headerData(column, Qt::Horizontal, Qt::UserRole).toString();
    this->sortOrder = order;
    this->regenerateQuery();
}

void EventTableModel::regenerateQuery() {
    QString queryString = QString::fromUtf8("SELECT id, type, date FROM event WHERE person = :person_id");

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

    QSqlQuery query;
    query.prepare(queryString);
    query.bindValue(QString::fromUtf8(":person_id"), this->personId);

    if(!query.exec()) {
        qDebug() << query.lastQuery();
        qCritical() << "Error executing query: " << query.lastError().text();
    }

    this->setQuery(std::move(query));
}


