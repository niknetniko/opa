//
// Created by niko on 9/04/2022.
//

#ifndef OPA_PEOPLE_TABLE_MODEL_H
#define OPA_PEOPLE_TABLE_MODEL_H

#include <QSqlQueryModel>
#include <QSortFilterProxyModel>
#include <KExtraColumnsProxyModel>
#include <KRearrangeColumnsProxyModel>

/**
 * The data model for the PeopleTableView.
 */
class BasicPeopleTableModel : public QSqlQueryModel {
Q_OBJECT

public:
    explicit BasicPeopleTableModel(QObject *parent = nullptr);

    [[nodiscard]] QVariant data(const QModelIndex &item, int role) const override;

    void sort(int column, Qt::SortOrder order) override;

public Q_SLOTS:

    void onSearchChanged(const QString &text);

private:
    Qt::SortOrder sortOrder;
    QString sortColumn;
    QString searchQuery;

    void regenerateQuery();
};

class AdditionalColumnModel: public KExtraColumnsProxyModel {
Q_OBJECT
public:
    explicit AdditionalColumnModel(QObject *parent = nullptr);

    [[nodiscard]] QVariant extraColumnData(const QModelIndex &parent, int row, int extraColumn, int role) const override;

public Q_SLOTS:

    void onSearchChanged(const QString &text);
};


class PeopleTableModel : public KRearrangeColumnsProxyModel {
Q_OBJECT
public:
    explicit PeopleTableModel(QObject *parent = nullptr);

public Q_SLOTS:

    void onSearchChanged(const QString &text);
};

#endif //OPA_PEOPLE_TABLE_MODEL_H
