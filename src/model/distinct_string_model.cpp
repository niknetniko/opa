/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "distinct_string_model.h"

#include "core/query_helper.h"

using namespace Qt::StringLiterals;

DistinctStringModel::DistinctStringModel(QObject* parent) : QStringListModel(parent) {
}

void DistinctStringModel::reload() {
    const auto sql = u"SELECT DISTINCT %1 FROM %2"_s.arg(this->field, this->table);
    QStringList result;
    for (const auto& entity: QueryHelper::fetchAll<QString>(sql, {}, &DistinctStringModel::extract)) {
        result.append(entity);
    }
    this->setStringList(result);
}

void DistinctStringModel::setTable(const QString& table, const QString& field) {
    this->table = table;
    this->field = field;
}

QString DistinctStringModel::extract(const QSqlQuery& query) {
    return query.value(0).toString();
}
