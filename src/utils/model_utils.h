//
// Created by niko on 28/09/24.
//

#ifndef OPA_MODEL_UTILS_H
#define OPA_MODEL_UTILS_H

#include <qabstractproxymodel.h>

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

#endif //OPA_MODEL_UTILS_H
