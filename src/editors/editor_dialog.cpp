
/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "editor_dialog.h"

#include "utils/model_utils_find_source_model_of_type.h"

#include <QDataWidgetMapper>
#include <QDialogButtonBox>
#include <QSqlError>
#include <QSqlQueryModel>

AbstractEditorDialog::AbstractEditorDialog(QWidget* parent) : QDialog(parent) {
}

void AbstractEditorDialog::reject() {
    for (auto* mapper: std::as_const(this->mappers)) {
        mapper->revert();
    }

    this->revert();

    QDialog::reject();
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
