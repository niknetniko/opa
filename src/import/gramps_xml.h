/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <libxml/tree.h>

#include <QPromise>
#include <QString>

using GrampsXmlRoot = std::unique_ptr<xmlDoc, decltype(&xmlFreeDoc)>;

struct GrampsXmlAnalysis {
    GrampsXmlRoot document = {nullptr, xmlFreeDoc};
    bool valid;
    QString error;
    int people, families, events, sources, places, media, repositories, notes, citations;
};


void validateGrampsXml(QPromise<GrampsXmlAnalysis>& promise, const QString& filename);

void importGrampsResult(QPromise<bool>& promise, const GrampsXmlAnalysis& result);
