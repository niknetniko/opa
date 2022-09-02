//
// Created by niko on 9/04/2022.
//

#ifndef OPA_PEOPLE_LIST_MODEL_H
#define OPA_PEOPLE_LIST_MODEL_H

#include <QSqlQueryModel>

class PeopleListModel : public QSqlQueryModel {
    Q_OBJECT

public:
    explicit PeopleListModel(QObject *parent = nullptr);

    QVariant data(const QModelIndex &item, int role) const override;

    void sort(int column, Qt::SortOrder order) override;
};

#endif //OPA_PEOPLE_LIST_MODEL_H
