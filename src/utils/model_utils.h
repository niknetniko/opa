//
// Created by niko on 28/09/24.
//

#ifndef OPA_MODEL_UTILS_H
#define OPA_MODEL_UTILS_H

#include <qabstractproxymodel.h>
#include <QIdentityProxyModel>

/**
 * Find the first source model of a specific type.
 * @tparam T The type to search for.
 * @param model The proxy model to find the source for.
 * @return The model pointer or nullptr if there is none.
 */
template<class T>
const T *find_source_model_of_type(const QAbstractItemModel *model) {
    // A special case: if this is the model we search, just return it.
    const T *secondCast = qobject_cast<const T *>(model);
    if (secondCast != nullptr) {
        return secondCast;
    }

    // Else, start going down the path.
    const QAbstractItemModel *potentialModel = model;
    const QAbstractProxyModel *proxyModel;
    while ((proxyModel = qobject_cast<const QAbstractProxyModel *>(potentialModel)) != nullptr) {
        potentialModel = proxyModel->sourceModel();
        // If this is the type we search, stop now.
        secondCast = qobject_cast<const T *>(potentialModel);
        if (secondCast != nullptr) {
            return secondCast;
        }
    }

    // We have not found anything.
    return nullptr;
}

/**
 * Convert indices to source models until there are no more proxy models.
 *
 * This will use the model attached to the given index.
 *
 * @param index The index to convert to a source.
 *
 * @return The converted model index.
 */
inline QModelIndex map_to_source_model(const QModelIndex &index) {
    auto model = qobject_cast<const QAbstractProxyModel *>(index.model());
    if (model == nullptr) {
        // There is no proxy model in the index.
        return index;
    }

    QModelIndex converted = model->mapToSource(index);
    while ((model = qobject_cast<const QAbstractProxyModel *>(model->sourceModel())) != nullptr) {
        converted = model->mapToSource(converted);
    }

    // We have not found anything.
    return converted;
}

constexpr int ModelRole = Qt::UserRole + 20;

/**
 * Class the handle a column where an OpaDate is saved as JSON in some column.
 * The model ignores empty strings or null values, but will error on other invalid values.
 */
class OpaDateModel : public QIdentityProxyModel {
    Q_OBJECT

public:
    explicit OpaDateModel(QObject *parent);

    void setDateColumn(int column);

    int dateColumn() const;

    QVariant data(const QModelIndex &proxyIndex, int role = Qt::DisplayRole) const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

private:
    int theDateColumn;
};

#endif //OPA_MODEL_UTILS_H
