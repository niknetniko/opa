//
// Created by niko on 25/09/24.
//

#ifndef OPA_DATA_NAMES_H
#define OPA_DATA_NAMES_H

#include <QSqlTableModel>
#include <QSqlRelationalTableModel>

#include "database/schema.h"
#include "utils/custom_sql_relational_model.h"

/**
 * Base model for the names table.
 */
class NamesTableModel : public CustomSqlRelationalModel {
Q_OBJECT

public:
    static constexpr int ID = 0;
    static constexpr int PERSON_ID = 1;
    static constexpr int SORT = 2;
    static constexpr int TITLES = 3;
    static constexpr int GIVEN_NAMES = 4;
    static constexpr int PREFIX = 5;
    static constexpr int SURNAME = 6;
    static constexpr int ORIGIN_ID = 7;
    static constexpr int ORIGIN = 8;

    explicit NamesTableModel(QObject *parent, QSqlTableModel* originsModel);
};

class NameOriginTableModel : public QSqlTableModel {
Q_OBJECT

public:
    static constexpr int ID = 0;
    static constexpr int ORIGIN = 1;

    explicit NameOriginTableModel(QObject *parent = nullptr);

    Qt::ItemFlags flags(const QModelIndex &index) const override;
};

#endif //OPA_DATA_NAMES_H
