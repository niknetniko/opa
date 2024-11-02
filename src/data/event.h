#pragma once

#include <QString>

#include "utils/custom_sql_relational_model.h"
#include "utils/model_utils.h"

namespace EventRoles {
    Q_NAMESPACE

    enum Values {
        Primary,
        Witness
    };

    Q_ENUM_NS(Values)

    static const QHash<Values, KLazyLocalizedString> nameOriginToString = {
        {Primary, kli18n("Primary")},
        {Witness, kli18n("Witness")}
    };

    static const std::function toDisplayString = [](const QString &databaseValue) {
        return genericToDisplayString<Values>(databaseValue, nameOriginToString);
    };
}

class EventRolesModel : public QSqlTableModel {
    Q_OBJECT

public:
    static constexpr int ID = 0;
    static constexpr int ROLE = 1;
    static constexpr int BUILTIN = 2;

    explicit EventRolesModel(QObject *parent);
};

namespace EventTypes {
    Q_NAMESPACE

    enum Values {
        Birth,
        Death,
        Marriage,
        Divorce,
        Baptism,
        Funeral
    };

    Q_ENUM_NS(Values)

    static const QHash<Values, KLazyLocalizedString> nameOriginToString = {
        {Birth, kli18n("Birth")},
        {Death, kli18n("Death")},
        {Marriage, kli18n("Marriage")},
        {Divorce, kli18n("Divorce")},
        {Baptism, kli18n("Baptism")},
        {Funeral, kli18n("Funeral")}
    };

    static const std::function toDisplayString = [](const QString &databaseValue) {
        return genericToDisplayString<Values>(databaseValue, nameOriginToString);
    };
}

class EventTypesModel : public QSqlTableModel {
    Q_OBJECT

public:
    static constexpr int ID = 0;
    static constexpr int TYPE = 1;
    static constexpr int BUILTIN = 2;

    explicit EventTypesModel(QObject *parent);
};

class EventRelationsModel : public CustomSqlRelationalModel {
    Q_OBJECT

public:
    static constexpr int EVENT_ID = 0;
    static constexpr int PERSON_ID = 1;
    static constexpr int ROLE_ID = 2;
    static constexpr int ROLE = 3;

    explicit EventRelationsModel(QObject *parent, QSqlTableModel *rolesModel);
};

class EventsModel : public CustomSqlRelationalModel {
    Q_OBJECT

public:
    static constexpr int ID = 0;
    static constexpr int TYPE_ID = 1;
    static constexpr int DATE = 2;
    static constexpr int NAME = 3;
    static constexpr int TYPE = 4;

    explicit EventsModel(QObject *parent, QSqlTableModel *typesModel);
};

namespace PersonEventsModel {
    constexpr int ROLE = 0;
    constexpr int TYPE = 1;
    constexpr int DATE = 2;
    constexpr int NAME = 3;
    constexpr int ID = 4;
    constexpr int ROLE_ID = 5;
}
