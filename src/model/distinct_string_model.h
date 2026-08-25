/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include <QStringListModel>

class QSqlQuery;

class DistinctStringModel : public QStringListModel {
    Q_OBJECT

public:
    DistinctStringModel(QObject* parent = nullptr);

public Q_SLOTS:
    void reload();

protected:
    void setTable(const QString& table, const QString& field);

private:
    QString table;
    QString field;

    static QString extract(const QSqlQuery& query);
};
