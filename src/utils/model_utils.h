
/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include <QSqlTableModel>

/**
 * Execute a "transaction" on a model.
 *
 * Concretely, it will set the model to manual submit, submit all changes
 * and finally restore the original submit policy.
 */
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
