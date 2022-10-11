//
// Created by niko on 9/04/2022.
//

#ifndef OPA_PEOPLE_TABLE_MODEL_H
#define OPA_PEOPLE_TABLE_MODEL_H

#include <QSqlQueryModel>

/**
 * The data model for the PeopleTableView.
 */
class PeopleTableModel : public QSqlQueryModel {
    Q_OBJECT

public:
    explicit PeopleTableModel(QObject *parent = nullptr);

    QVariant data(const QModelIndex &item, int role) const override;

    void sort(int column, Qt::SortOrder order) override;

public Q_SLOTS:
    void onSearchChanged(const QString& text);

private:
    Qt::SortOrder sortOrder;
    QString sortColumn;
    QString searchQuery;

    void regenerateQuery();
};

#endif //OPA_PEOPLE_TABLE_MODEL_H
