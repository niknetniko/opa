//
// Created by niko on 9/04/2022.
//

#ifndef OPA_EVENT_TABLE_MODEL_H
#define OPA_EVENT_TABLE_MODEL_H

#include <QSqlQueryModel>

/**
 * The data model for the PeopleTableView.
 */
class EventTableModel : public QSqlQueryModel {
    Q_OBJECT

public:
    explicit EventTableModel(long long personId, QObject *parent = nullptr);

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QVariant data(const QModelIndex &item, int role) const override;

    void sort(int column, Qt::SortOrder order) override;

private:
    long long personId;
    Qt::SortOrder sortOrder;
    QString sortColumn;

    void regenerateQuery();
};

#endif //OPA_EVENT_TABLE_MODEL_H
