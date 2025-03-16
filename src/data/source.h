/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include <QIdentityProxyModel>
#include <QSqlTableModel>

/**
 * Raw base model for sources.
 *
 * This represents the model as a flat list, as they are stored in the database.
 */
class SourcesTableModel : public QSqlTableModel {
    Q_OBJECT

public:
    static constexpr int ID = 0;
    static constexpr int TITLE = 1;
    static constexpr int AUTHOR = 2;
    static constexpr int SOURCE_DATE = 3;
    static constexpr int CONFIDENCE = 4;
    static constexpr int PARENT_ID = 5;

    explicit SourcesTableModel(QObject* parent = nullptr);
};
