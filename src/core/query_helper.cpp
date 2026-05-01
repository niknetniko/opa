/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */


#include "query_helper.h"

#include <QSqlError>
#include <QString>

using namespace QueryHelper;
using namespace Qt::StringLiterals;

SqlQueryBuilder& SqlQueryBuilder::from(const QString& table) {
    tableName = table;
    return *this;
}

SqlQueryBuilder& SqlQueryBuilder::select(const QStringList& columns) {
    selectColumns = columns;
    return *this;
}

SqlQueryBuilder& SqlQueryBuilder::where(const QString& condition) {
    whereClauses << condition;
    return *this;
}

SqlQueryBuilder& SqlQueryBuilder::bind(const QString& placeholder, const QVariant& value) {
    bindings.insert(placeholder, value);
    return *this;
}

SqlQueryBuilder& SqlQueryBuilder::orderBy(const QString& column, Qt::SortOrder order) {
    if (!column.isEmpty()) {
        orderByClause = column + (order == Qt::AscendingOrder ? u" ASC"_s : u" DESC"_s);
    }
    return *this;
}

SqlQueryBuilder& SqlQueryBuilder::limit(int limit) {
    limitCount = limit;
    return *this;
}

SqlQueryBuilder& SqlQueryBuilder::offset(int offset) {
    offsetCount = offset;
    return *this;
}

SqlQueryBuilder& SqlQueryBuilder::applyCriteria(const QueryCriteria& criteria) {
    if (!criteria.filterText.isEmpty()) {
        this->where(u"%1 LIKE :search_text"_s.arg(criteria.filterColumn));
        this->bind(u":search_text"_s, QString(u"%"_s + criteria.filterText + u"%"_s));
    }

    for (const auto [key, value]: criteria.filters.asKeyValueRange()) {
        auto sanitizedKey = key;
        sanitizedKey.replace(u"."_s, u"_"_s);
        auto paramName = u":%1"_s.arg(sanitizedKey);
        this->where(u"%1 = %2"_s.arg(key, paramName));
        this->bind(paramName, value);
    }

    if (!criteria.sortColumn.isEmpty()) {
        this->orderBy(criteria.sortColumn, criteria.sortOrder);
    }

    if (criteria.limit > 0) {
        this->limit(criteria.limit);
        this->offset(criteria.offset);
    }

    return *this;
}

std::tuple<QString, QVariantMap> SqlQueryBuilder::construct() const {
    QString sql = baseSql;

    if (sql.isEmpty() && !tableName.isEmpty()) {
        QString cols = selectColumns.isEmpty() ? u"*"_s : selectColumns.join(u", "_s);
        sql = u"SELECT %1 FROM %2"_s.arg(cols, tableName);
    }

    if (!whereClauses.isEmpty()) {
        // Safely append WHERE or AND depending on the base query
        if (sql.contains(u" WHERE "_s, Qt::CaseInsensitive)) {
            sql += u" AND "_s + whereClauses.join(u" AND "_s);
        } else {
            sql += u" WHERE "_s + whereClauses.join(u" AND "_s);
        }
    }

    if (!orderByClause.isEmpty()) {
        sql += u" ORDER BY "_s + orderByClause;
    }

    if (limitCount > 0) {
        sql += u" LIMIT :limit OFFSET :offset"_s;
    }

    QVariantMap copiedBindings = bindings;
    if (limitCount > 0) {
        copiedBindings.insert(u":limit"_s, limitCount);
        copiedBindings.insert(u":offset"_s, offsetCount);
    }

    return {std::move(sql), std::move(copiedBindings)};
}

std::tuple<QSqlQuery, bool> QueryHelper::executeWithResult(const QString& sql, const QVariantMap& bindings) {
    QSqlQuery query;

    if (!query.prepare(sql)) {
        qWarning() << "Failed to prepare query" << sql;
        qWarning() << query.lastError().text();
        return {std::move(query), false};
    }

    for (const auto& [key, value]: bindings.asKeyValueRange()) {
        query.bindValue(key, value);
    }

    auto result = query.exec();

    if (!result) {
        qWarning() << "Failed to execute query" << query.executedQuery();
        qWarning() << query.lastError().text();
    }

    return {std::move(query), result};
}

bool QueryHelper::execute(const QString& sql, const QVariantMap& bindings) {
    auto [_, result] = executeWithResult(sql, bindings);
    return result;
}

std::optional<IntegerPrimaryKey> QueryHelper::insert(const QString& sql, const QVariantMap& bindings) {
    auto [query, result] = executeWithResult(sql, bindings);

    if (!result) {
        return std::nullopt;
    }

    auto lastId = query.lastInsertId();
    if (lastId.isValid() && !lastId.isNull()) {
        auto signedId = lastId.toLongLong();

        if (signedId < 0) {
            qCritical() << "Negative ID returned from database:" << lastId;
            return std::nullopt;
        }

        return signedId;
    }

    return std::nullopt;
}
