
/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include <QSqlTableModel>


template<typename T>
bool modelTransaction(QSqlTableModel* model, T&& transaction) {
    auto originalStrategy = model->editStrategy();
    assert(!model->isDirty());
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);

    // Do the transaction.
    transaction();

    bool wasSubmitted = model->submitAll();
    model->setEditStrategy(originalStrategy);
    return wasSubmitted;
}
