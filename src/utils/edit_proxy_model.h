//
// Created by niko on 22/10/24.
//

#ifndef EDIT_PROXY_MODEL_H
#define EDIT_PROXY_MODEL_H

#include <QIdentityProxyModel>

/**
 * A proxy model that makes some columns or rows read-only, regardless of what the source models says.
 */
class EditProxyModel: public QIdentityProxyModel {
    Q_OBJECT

public:
    explicit EditProxyModel(QObject* parent = nullptr);

    void addReadOnlyColumns(const QList<int> &columns);

    Qt::ItemFlags flags(const QModelIndex &index) const override;

private:
    QList<int> columns;

};

#endif //EDIT_PROXY_MODEL_H
