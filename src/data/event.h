//
// Created by niko on 3/09/22.
//

#ifndef OPA_EVENT_H
#define OPA_EVENT_H

#include <QString>
#include <QSqlTableModel>
#include <QSqlRelationalTableModel>

namespace KnownEventRols {
    const QString Primary = QStringLiteral("primary");
    const QString Witness = QStringLiteral("witness");
};

namespace KnownEventTypes {
    const QString BIRTH = QStringLiteral("birth");
    const QString DEATH = QStringLiteral("death");
    const QString MARRIAGE = QStringLiteral("marriage");
    const QString DIVORCE = QStringLiteral("divorce");
    const QString BAPTISM = QStringLiteral("baptism");
    const QString CONFIRMATION = QStringLiteral("confirmation");
    const QString FIRST_COMMUNION = QStringLiteral("first_communion");
    const QString FUNERAL = QStringLiteral("funeral");
}

class EventRolesModel : public QSqlTableModel {
Q_OBJECT

public:
    static const int ID = 0;
    static const int ROLE = 1;

    EventRolesModel(QObject *parent);
};

class EventTypesModel : public QSqlTableModel {
Q_OBJECT

public:
    static const int ID = 0;
    static const int TYPE = 1;

    EventTypesModel(QObject *parent);
};

class EventRelationsModel : public QSqlTableModel {
Q_OBJECT

public:
    static const int EVENT_ID = 0;
    static const int PERSON_ID = 1;
    static const int ROLE_ID = 2;

    EventRelationsModel(QObject *parent);
};

class EventsModel : public QSqlRelationalTableModel {
    Q_OBJECT
public:
    static const int ID = 0;
    static const int TYPE = 1;
    static const int DATE = 2;
    static const int NAME = 3;

    EventsModel(QObject *parent);

    QVariant data(const QModelIndex &item, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &item, const QVariant &value, int role = Qt::EditRole) override;
};

namespace PersonEventsModel {
    const int ROLE = 0;
    const int ID = 1;
    const int DATE = 2;
    const int NAME = 3;
    const int TYPE = 4;
};

class PersonEventsTreeModel {

};


#endif //OPA_EVENT_H
