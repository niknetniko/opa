
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
    for (auto* mapper: std::as_const(this->mappers)) {
        mapper->revert();
        if (isNewItem) {
            if (!mapper->model()->removeRow(0)) {
                qWarning() << "Could not remove newly row for mapper" << mapper;
                if (auto* sourceModel = findSourceModelOfType<QSqlQueryModel>(mapper->model())) {
                    qDebug() << sourceModel->lastError();
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
