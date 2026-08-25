/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "../src/import/gramps_xml.h"

#include <QFuture>
#include <QPromise>
#include <QSqlError>
#include <QSqlQuery>
#include <QTemporaryFile>

using namespace Qt::Literals::StringLiterals;

inline QString grampsXml(const QString& body = {}) {
    const auto ns = u"http://gramps-project.org/xml/1.7.2/"_s;
    return u"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<database xmlns=\""_s + ns +
           u"\">\n"
           "  <header>\n"
           "    <created date=\"2024-01-01\" version=\"5.1.3\"/>\n"
           "    <researcher/>\n"
           "  </header>\n"_s +
           body + u"</database>\n"_s;
}

inline GrampsXmlAnalysis validateGramps(const QString& xml) {
    QTemporaryFile file;
    file.setAutoRemove(true);
    if (!file.open()) {
        qFatal("Could not create temporary file for test");
    }
    file.write(xml.toUtf8());
    file.flush();
    file.close();

    QPromise<GrampsXmlAnalysis> promise;
    promise.start();
    validateGrampsXml(promise, file.fileName());
    promise.finish();
    return promise.future().takeResult();
}

inline bool runImport(const GrampsXmlAnalysis& analysis) {
    QPromise<bool> promise;
    promise.start();
    importGrampsResult(promise, analysis);
    promise.finish();
    auto future = promise.future();
    return future.resultCount() > 0 && future.result();
}

inline QSqlQuery execQuery(const QString& sql) {
    QSqlQuery q;
    if (!q.exec(sql)) {
        qWarning() << "Query failed:" << q.lastError().text();
    }
    return q;
}
