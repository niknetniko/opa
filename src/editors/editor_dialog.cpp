/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "editor_dialog.h"

#include "utils/model_utils.h"

#include <QDataWidgetMapper>
#include <QDialogButtonBox>
#include <QSqlError>
#include <QSqlQueryModel>

AbstractEditorDialog::AbstractEditorDialog(bool isNewItem, QWidget* parent) : QDialog(parent), isNewItem(isNewItem) {
}

void AbstractEditorDialog::reject() {
    qDebug() << "Rejecting" << (isNewItem ? "new" : "existing") << "item";

    for (auto* mapper: std::as_const(this->mappers)) {
        qDebug() << "Reverting mapper" << mapper;
        mapper->revert();
        if (isNewItem) {
            qDebug() << "Removing newly added row";
            if (!mapper->model()->removeRow(0)) {
                qWarning() << "Could not remove newly row for mapper" << mapper;
                if (auto* sourceModel = findSourceModelOfType<QSqlQueryModel>(mapper->model())) {
                    qDebug() << sourceModel->lastError();
                } else {
                    qWarning() << "Could not find source model for mapper" << mapper;
                }
            } else {
                // Refresh the model to remove the removed row if possible.
                if (auto* sourceModel = findSourceModelOfType<QSqlTableModel>(mapper->model())) {
                    qDebug() << "Refreshing source model after row deletion";
                    const_cast<QSqlTableModel*>(sourceModel)->select();
                }
            }
        }
    }

    this->revert();

    QDialog::reject();
}

void AbstractEditorDialog::revert() {
}

void AbstractEditorDialog::addMapper(QDataWidgetMapper* mapper) {
    Q_ASSERT(mapper->model()->rowCount() == 1);
    this->mappers.append(mapper);
}

void AbstractEditorDialog::accept() {
    bool mappersAccept = true;
    for (auto* mapper: std::as_const(this->mappers)) {
        bool thisMapperAccepts = mapper->submit();
        if (!thisMapperAccepts) {
            qWarning() << "Mapper" << mapper << "rejects changes!";
            if (auto* sourceModel = findSourceModelOfType<QSqlQueryModel>(mapper->model())) {
                qDebug() << sourceModel->lastError();
            }
        }
        mappersAccept = mappersAccept && thisMapperAccepts;
    }

    if (mappersAccept) {
        QDialog::accept();
    }
}
