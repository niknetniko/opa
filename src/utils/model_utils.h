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
const T *find_source_model_of_type(const QAbstractProxyModel *model) {
    const QAbstractItemModel *potential_model;
    // The first iteration is always possible.
    do {
        potential_model = model->sourceModel();
        // If this is the type we search, stop now.
        const T* second_cast = qobject_cast<const T*>(potential_model);
        if (second_cast != nullptr) {
            return second_cast;
        }
        // Else continue while we are in a proxy model.
    } while (qobject_cast<const QAbstractProxyModel *>(model) != nullptr);

    // We have not found anything.
    return nullptr;
}

#endif //OPA_MODEL_UTILS_H
