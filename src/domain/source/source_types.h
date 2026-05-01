/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "utils/model_utils.h"

#include <QString>

namespace SourceTypes {
Q_NAMESPACE;

enum class Values {
    Certificate,
    Register,
    Census,
    Will,
    Letter,
    Newspaper,
    Book,
    Website,
    Photograph,
};

Q_ENUM_NS(Values);

const QHash<Values, KLazyLocalizedString> sourceTypeToString{
    {Values::Certificate, kli18n("Certificate")},
    {Values::Register, kli18n("Register")},
    {Values::Census, kli18n("Census")},
    {Values::Will, kli18n("Will")},
    {Values::Letter, kli18n("Letter")},
    {Values::Newspaper, kli18n("Newspaper")},
    {Values::Book, kli18n("Book")},
    {Values::Website, kli18n("Website")},
    {Values::Photograph, kli18n("Photograph")},
};

const auto toDisplayString = [](const QString& databaseValue) {
    return genericToDisplayString<Values>(databaseValue, sourceTypeToString);
};
}
