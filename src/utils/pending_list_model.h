/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "database/schema.h"

#include <QList>
#include <QSet>
#include <functional>
#include <optional>

/**
 * Generic in-memory buffer that tracks pending additions and removals for a list of items.
 *
 * One use case is e.g. a model for adding a row to a table, in which the row is also
 * linked to other rows (e.g. events and sources). When adding a new event, linking the
 * sources is not possible yet, as the event does not exist in the database yet.
 *
 * The case when editing an existing event might also need this: we do not want to commit
 * changes to sources directly, as then we cannot undo them when the user presses cancel.
 *
 * @tparam T    The entity type (e.g. SourceEntity).
 * @tparam Id   The primary key type (defaults to IntegerPrimaryKey).
 */
template<typename T, typename Id = IntegerPrimaryKey>
class PendingListModel {
public:
    using IdExtractor = std::function<Id(const T&)>;
    using LookupFn = std::function<std::optional<T>(Id)>;
    using CommitFn = std::function<bool(Id)>;

    PendingListModel(QList<T> initial, IdExtractor idExtractor, LookupFn lookup)
        : initialItems(std::move(initial)),
          extractId(std::move(idExtractor)),
          lookupById(std::move(lookup)) {
    }

    /**
     * Returns the combined view: initial items minus removals, plus pending additions.
     */
    [[nodiscard]] QList<T> items() const {
        QList<T> result;
        for (const auto& item : initialItems) {
            if (!pendingRemovals.contains(extractId(item))) {
                result.append(item);
            }
        }
        result.append(pendingAdds);
        return result;
    }

    /**
     * Schedule an addition.
     */
    bool add(Id id) {
        // Check if already in initial items (and not removed).
        for (const auto& item : initialItems) {
            if (extractId(item) == id && !pendingRemovals.contains(id)) {
                return false;
            }
        }
        // Check if already in pending adds.
        for (const auto& item : pendingAdds) {
            if (extractId(item) == id) {
                return false;
            }
        }
        // If it was previously removed from initial, un-remove it instead of adding.
        if (pendingRemovals.contains(id)) {
            pendingRemovals.remove(id);
            return true;
        }
        auto entity = lookupById(id);
        if (!entity) {
            return false;
        }
        pendingAdds.append(std::move(*entity));
        return true;
    }

    /**
     * Schedule a removal.
     */
    bool remove(Id id) {
        // Check pending adds first.
        for (int i = 0; i < pendingAdds.size(); ++i) {
            if (extractId(pendingAdds[i]) == id) {
                pendingAdds.removeAt(i);
                return true;
            }
        }
        // Must be an initial item.
        for (const auto& item : initialItems) {
            if (extractId(item) == id) {
                pendingRemovals.insert(id);
                return true;
            }
        }
        return false;
    }

    /**
     * Commit all pending changes by calling the provided functions.
     */
    void commit(const CommitFn& addFn, const CommitFn& removeFn) const {
        for (const auto& item : pendingAdds) {
            addFn(extractId(item));
        }
        for (const auto& id : pendingRemovals) {
            removeFn(id);
        }
    }

private:
    QList<T> initialItems;
    IdExtractor extractId;
    LookupFn lookupById;

    QList<T> pendingAdds;
    QSet<Id> pendingRemovals;
};
