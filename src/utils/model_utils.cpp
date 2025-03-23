/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "model_utils.h"

void debugPrintModel(const QAbstractItemModel* model, const QModelIndex& parent, int level) {
    if (!model || level == 20) {
        return;
    }

    for (int r = 0; r < model->rowCount(parent); ++r) {
        auto deb = qDebug();
        deb.noquote() << QString(level, QLatin1Char(' '));

        for (int c = 0; c < model->columnCount(parent); ++c) {
            QModelIndex index = model->index(r, c, parent);
            deb.quote() << index.data() << ",";
        }

        auto potentialParent = model->index(r, 0, parent);
        if (potentialParent.isValid()) {
            debugPrintModel(model, potentialParent, level + 1);
        }
    }
}
