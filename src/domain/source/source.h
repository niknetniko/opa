/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include "utils/model_utils.h"

#include <KLazyLocalizedString>
#include <QSqlTableModel>

namespace Confidence {
Q_NAMESPACE

/**
 * The confidence denotes how much weight should be assigned to the data from
 * a particular source.
 * This is mostly a personal judgement, but some guidance is provided in the
 * description of the values below.
 */
enum class Values {
    /**
     * No confidence at all, probably wrong.
     * You should consider these more of a "mention" than really supporting
     * of the associated data.
     *
     * Examples include very big online genealogies without any sources,
     * or sources for which it is known that the data might be wrong.
     */
    VeryLow,
    /**
     * Tertiary sources with low confidence.
     * Data associated with this source should be considered possible, rather than fact.
     *
     * These might be online genealogies that you checked with sampling, or that are
     * generally correct in your experience.
     */
    Low,
    /**
     * Tertiary sources with normal confidence.
     * Data associated with this source should be considered probable but not yet proven.
     *
     * For example, an online genealogy that has sources themselves and sampling indicates
     * that it is generally correct.
     */
    Normal,
    /**
     * Secondary sources with higher confidence.
     * These sources should generally be considered "fact", unless some editing mistake
     * introduced errors into the source.
     *
     * Examples of this include family reconstructions or "klappers" for Church records.
     */
    High,
    /**
     * Mainly for primary sources.
     *
     * Primary sources are records from the time itself, such as birth certificates.
     * While those can and do have mistakes, they are generally the most reliable source
     * available.
     *
     * To illustrate the personalness of these judgements, if you have a secondary source,
     * such as a family reconstruction for which you are very certain it is correct, you
     * might also denote it as very high.
     */
    VeryHigh,
    /** Unknown at this time. */
    Unknown
};

Q_ENUM_NS(Values)

const QHash<Values, KLazyLocalizedString> confidenceToString = {
    {Values::Unknown, kli18n("Unknown")},
    {Values::VeryLow, kli18n("Very low")},
    {Values::Low, kli18n("Low")},
    {Values::Normal, kli18n("Normal")},
    {Values::High, kli18n("High")},
    {Values::VeryHigh, kli18n("Very high")},
};

const auto toDisplayString = [](const QString& databaseValue) {
    return genericToDisplayString<Values>(databaseValue, confidenceToString);
};
}

/**
 * Raw base model for sources.
 *
 * This represents the model as a flat list, as they are stored in the database.
 */
class SourcesTableModel : public QSqlTableModel {
    Q_OBJECT

public:
    static constexpr int ID = 0;
    static constexpr int TITLE = 1;
    static constexpr int AUTHOR = 2;
    static constexpr int PUBLICATION = 3;
    static constexpr int CONFIDENCE = 4;
    static constexpr int NOTE = 5;
    static constexpr int PARENT_ID = 6;
    static constexpr int TYPE_ID = 7;

    explicit SourcesTableModel(QObject* parent = nullptr);
};
