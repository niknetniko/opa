/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "database/schema.h"
#include <qtestcase.h>

#include <QSqlError>
#include <QSqlQuery>
#include <QTest>
#include <source_location>


inline QByteArray printError(const QSqlQuery& query) {
    auto result = QStringLiteral("");
    auto error = query.lastError();
    if (!error.nativeErrorCode().isEmpty()) {
        result += QLatin1Char('(') + error.nativeErrorCode() + QStringLiteral(") ");
    }
    result += QLatin1Char('\'');
    result += error.text() + QLatin1Char('\'');
    result += QLatin1Char('(') + query.lastQuery() + QLatin1Char(')');
    return result.toLocal8Bit();
}

template<typename T>
constexpr void verifyOrThrow(
    const T& statement,
    const char* expression,
    const char* description,
    std::source_location location = std::source_location::current()
) {
    if (!QTest::qVerify(static_cast<bool>(statement), expression, description, location.file_name(), location.line())) {
        throw std::logic_error(std::string("verify failed: ") + expression);
    }
}

#define VERIFY_OR_THROW(statement) verifyOrThrow(statement, #statement, "")
#define VERIFY_OR_THROW2(statement, error) verifyOrThrow(statement, #statement, printError(error).constData())

inline IntegerPrimaryKey insertQuery(const QString& query) {
    QSqlQuery query_;
    VERIFY_OR_THROW2(query_.exec(query), query_);
    auto insertedRowIdVariant = query_.lastInsertId();
    VERIFY_OR_THROW(insertedRowIdVariant.isValid());
    return insertedRowIdVariant.toInt();
}

inline IntegerPrimaryKey selectQuery(const QString& query) {
    QSqlQuery query_;
    VERIFY_OR_THROW(query_.exec(query));
    VERIFY_OR_THROW(query_.next());
    auto theValue = query_.value(0);
    VERIFY_OR_THROW(theValue.isValid());
    return theValue.toInt();
}
