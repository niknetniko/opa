/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "names.h"

QString construct_display_name(
    const QString& titles,
    const QString& givenNames,
    const QString& prefix,
    const QString& surname
) {
    QStringList nameParts;
    if (!titles.isEmpty()) {
        nameParts.append(titles);
    }
    if (!givenNames.isEmpty()) {
        nameParts.append(givenNames);
    }
    if (!prefix.isEmpty()) {
        nameParts.append(prefix);
    }
    if (!surname.isEmpty()) {
        nameParts.append(surname);
    }
    return nameParts.join(QStringLiteral(" "));
}
