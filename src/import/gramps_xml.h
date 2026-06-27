/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <QPromise>
#include <QString>

struct GrampsXmlAnalysis {
    bool valid;
    QString error;
    int people, families, events, sources, places;

    // TODO, save parsed version.
};


void validateGrampsXml(QPromise<GrampsXmlAnalysis>& promise, const QString& filename);
