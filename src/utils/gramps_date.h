#ifndef OPA_GRAMPS_DATE_H
#define OPA_GRAMPS_DATE_H

#include <optional>
#include <QMetaType>
#include <QDebug>
#include <QObject>


enum DateModifier {
    NONE,
    BEFORE,
    AFTER,
//    SPAN,
    TEXT
};

enum DateQuality {
    EXACT,
    ESTIMATED,
    CALCULATED
};

enum Calendar {
    GREGORIAN,
    JULIAN,
    HEBREW,
    FRENCH,
    PERSIAN,
    ISLAMIC,
    SWEDISH
};

enum NewYear {
    JAN_1,
    MAR_1,
    MAR_25,
    SEP_1
};


struct DateParts {

};


/**
 * Main date class for Opa.
 *
 * * Origin *
 * This date class is a modified version of the Date class in Gramps.
 * See https://github.com/gramps-project/gramps/blob/35f20a8893599710f0cfb49685249759cdada6db/gramps/gen/lib/date.py.
 *
 *
 * The class supports partial dates, compound dates and alternate calendars.
 *
 *
 *
 */
class GrampsDate {

// TODO: make private
public:
    DateModifier modifier = DateModifier::NONE;
    DateQuality quality = DateQuality::EXACT;
    std::optional<int> day;
    std::optional<int> month;
    std::optional<int> year;
    Calendar calendar = Calendar::GREGORIAN;

public:

    // Required by QMetaType.
    GrampsDate() = default;
    ~GrampsDate() = default;
    GrampsDate(const GrampsDate&) = default;
    GrampsDate &operator=(const GrampsDate&) = default;

    GrampsDate(DateModifier modifier, DateQuality quality, const std::optional<int>& day,
               const std::optional<int>& month, const std::optional<int>& year, Calendar calendar) : modifier(modifier),
                                                                                                     quality(quality),
                                                                                                     day(day),
                                                                                                     month(month),
                                                                                                     year(year),
                                                                                                     calendar(
                                                                                                             calendar) {};



};

Q_DECLARE_METATYPE(GrampsDate);

QDebug operator<<(QDebug dbg, const GrampsDate &date);


#endif //OPA_GRAMPS_DATE_H
