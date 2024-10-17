//
// Created by niko on 3/09/22.
//

#ifndef OPA_EVENT_H
#define OPA_EVENT_H

#include <QString>
#include <QSqlTableModel>

#include "utils/custom_sql_relational_model.h"

namespace KnownEventRols {
    const auto Primary = QStringLiteral("primary");
    const auto Witness = QStringLiteral("witness");
};

namespace KnownEventTypes {
    const auto BIRTH = QStringLiteral("birth");
    const auto DEATH = QStringLiteral("death");
    const auto MARRIAGE = QStringLiteral("marriage");
    const auto DIVORCE = QStringLiteral("divorce");
    const auto BAPTISM = QStringLiteral("baptism");
    const auto CONFIRMATION = QStringLiteral("confirmation");
    const auto FIRST_COMMUNION = QStringLiteral("first_communion");
    const auto FUNERAL = QStringLiteral("funeral");
}

class EventRolesModel : public QSqlTableModel {
    Q_OBJECT

public:
    static constexpr int ID = 0;
    static constexpr int ROLE = 1;

    explicit EventRolesModel(QObject *parent);
};

class EventTypesModel : public QSqlTableModel {
    Q_OBJECT

public:
    static constexpr int ID = 0;
    static constexpr int TYPE = 1;

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
    static constexpr int TYPE = 3;

    explicit EventsModel(QObject *parent, QSqlTableModel *typesModel);
};

namespace PersonEventsModel {
    constexpr int ROLE = 0;
    constexpr int TYPE = 1;
    constexpr int DATE = 2;
    constexpr int NAME = 3;
    constexpr int ID = 4;
}

#endif //OPA_EVENT_H
