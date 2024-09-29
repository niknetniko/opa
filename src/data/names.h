//
// Created by niko on 25/09/24.
//

#ifndef OPA_DATA_NAMES_H
#define OPA_DATA_NAMES_H

#include <QSqlTableModel>
#include <QSqlRelationalTableModel>

#include "database/schema.h"

/**
 * Base model for the names table.
 */
class NamesTableModel : public QSqlRelationalTableModel {
Q_OBJECT

public:
    static const int ID = 0;
    static const int PERSON_ID = 1;
    static const int MAIN = 2;
    static const int TITLES = 3;
    static const int GIVEN_NAMES = 4;
    static const int PREFIX = 5;
    static const int SURNAME = 6;
    static const int ORIGIN_ID = 7;

    explicit NamesTableModel(QObject *parent = nullptr);
};

class NameOriginTableModel : public QSqlTableModel {
Q_OBJECT

public:
    static const int ID = 0;
    static const int ORIGIN = 1;

    explicit NameOriginTableModel(QObject *parent = nullptr);
};

#endif //OPA_DATA_NAMES_H
