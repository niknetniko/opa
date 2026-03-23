/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "family_members_model.h"

#include "../event/event_types.h"
#include "../name/names.h"
#include "core/data_event_broker.h"
#include "family_repository.h"

#include <KLocalizedString>
#include <limits>

// Internal ID encoding:
//   Top-level rows:  0x0000'0000'XXXX'XXXX  (high 32 bits = 0, low 32 = item index in `items`, or BASTARD_PARENT_KEY)
//   Child rows:      (parentKey + 1) << 32  |  (childItemIndex + 1)
//   This ensures top-level IDs never overlap with child IDs (high 32 bits are 0 vs non-zero).

static quintptr encodeParent(int itemIndex) {
    return static_cast<quintptr>(static_cast<quint32>(itemIndex));
}

static quintptr encodeChild(int parentKey, int childItemIndex) {
    return (static_cast<quintptr>(static_cast<quint32>(parentKey + 1)) << 32) |
           static_cast<quintptr>(static_cast<quint32>(childItemIndex + 1));
}

static bool isChildId(quintptr id) {
    return (id >> 32) != 0;
}

FamilyMembersModel::FamilyMembersModel(IntegerPrimaryKey personId, QObject* parent) :
    QAbstractItemModel(parent),
    personId(personId) {

    connectToTable<Schema::People>(this);
    connectToTable<Schema::Names>(this);
    connectToTable<Schema::Events>(this);
    connectToTable<Schema::EventTypes>(this);
    connectToTable<Schema::EventRoles>(this);
    connectToTable<Schema::EventRelations>(this);

    reload();
}

void FamilyMembersModel::reload() {
    beginResetModel();
    FamilyRepository repo;
    items = repo.findFamilyMembersForPerson(personId);
    rebuildMapping();
    endResetModel();
}

void FamilyMembersModel::rebuildMapping() {
    mapping.clear();

    // Map partner person ID -> index in items (of their marriage/relationship row)
    QHash<IntegerPrimaryKey, int> partnerToRelationshipRow;
    for (int i = 0; i < items.size(); ++i) {
        const auto& item = items[i];
        auto eventType = enumFromString<EventTypes::Values>(item.eventType);
        if (EventTypes::relationshipStartingEvents().contains(eventType)) {
            partnerToRelationshipRow[item.personId] = i;
        }
    }

    // Each birth row nests under its co-parent's relationship row (or bastard key if none)
    for (int i = 0; i < items.size(); ++i) {
        const auto& item = items[i];
        auto eventType = enumFromString<EventTypes::Values>(item.eventType);
        if (eventType == EventTypes::Values::Birth) {
            int parentKey = BASTARD_PARENT_KEY;
            if (item.partnerId.has_value()) {
                auto partnerId = *item.partnerId;
                if (partnerToRelationshipRow.contains(partnerId)) {
                    parentKey = partnerToRelationshipRow[partnerId];
                }
            }
            mapping[parentKey].append(i);
        }
    }
}

bool FamilyMembersModel::hasBastardChildren() const {
    return mapping.contains(BASTARD_PARENT_KEY);
}

bool FamilyMembersModel::isBastardParentRow(const QModelIndex& index) const {
    if (index.parent().isValid()) {
        return false;
    }
    auto keys = mapping.keys();
    if (index.row() >= keys.size()) {
        return false;
    }
    return keys[index.row()] == BASTARD_PARENT_KEY;
}

QModelIndex FamilyMembersModel::index(int row, int column, const QModelIndex& parent) const {
    if (!hasIndex(row, column, parent)) {
        return {};
    }

    auto keys = mapping.keys();

    if (!parent.isValid()) {
        // Top-level row: internalId encodes the key (item index or BASTARD_PARENT_KEY)
        if (row >= keys.size()) {
            return {};
        }
        int key = keys[row];
        return createIndex(row, column, encodeParent(key));
    } else {
        // Child row
        int parentKey = static_cast<int>(static_cast<quint32>(parent.internalId()));
        if (!mapping.contains(parentKey)) {
            return {};
        }
        const auto& children = mapping[parentKey];
        if (row >= children.size()) {
            return {};
        }
        int childItemIndex = children[row];
        return createIndex(row, column, encodeChild(parentKey, childItemIndex));
    }
}

QModelIndex FamilyMembersModel::parent(const QModelIndex& child) const {
    if (!child.isValid()) {
        return {};
    }

    quintptr id = child.internalId();
    if (!isChildId(id)) {
        // Top-level row has no parent
        return {};
    }

    // Decode parent key
    int parentKey = static_cast<int>(static_cast<quint32>(id >> 32)) - 1;
    auto keys = mapping.keys();
    int parentRow = keys.indexOf(parentKey);
    if (parentRow == -1) {
        return {};
    }
    return createIndex(parentRow, 0, encodeParent(parentKey));
}

int FamilyMembersModel::rowCount(const QModelIndex& parent) const {
    if (!parent.isValid()) {
        return mapping.size();
    }

    if (parent.column() != 0) {
        return 0;
    }

    quintptr id = parent.internalId();
    if (isChildId(id)) {
        // Child rows have no children
        return 0;
    }

    int key = static_cast<int>(static_cast<quint32>(id));
    if (!mapping.contains(key)) {
        return 0;
    }
    return mapping[key].size();
}

int FamilyMembersModel::columnCount(const QModelIndex& parent) const {
    Q_UNUSED(parent);
    return 5; // TYPE, DATE, PERSON_ID, DISPLAY_NAME, EVENT_ID
}

QVariant FamilyMembersModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) {
        return {};
    }
    if (role != Qt::DisplayRole && role != Qt::EditRole) {
        return {};
    }

    if (isBastardParentRow(index)) {
        if (index.column() == TYPE) {
            return i18n("Children with no other parent");
        }
        if (index.column() == PERSON_ID) {
            return QStringLiteral("");
        }
        return {};
    }

    quintptr id = index.internalId();
    int itemIndex;
    if (isChildId(id)) {
        // Decode child item index
        itemIndex = static_cast<int>(static_cast<quint32>(id)) - 1;
    } else {
        // Top-level row: the key is the item index
        itemIndex = static_cast<int>(static_cast<quint32>(id));
    }

    if (itemIndex < 0 || itemIndex >= items.size()) {
        return {};
    }
    return dataForItem(items[itemIndex], index.column());
}

QVariant FamilyMembersModel::dataForItem(const FamilyMemberEntity& item, int column) const {
    switch (column) {
        case TYPE:
            return item.eventType;
        case DATE:
            return item.date;
        case PERSON_ID:
            return item.personId;
        case DISPLAY_NAME:
            return construct_display_name(item.titles, item.givenNames, item.prefix, item.surname);
        case EVENT_ID:
            return item.eventId;
        default:
            return {};
    }
}

QVariant FamilyMembersModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal) {
        return {};
    }
    switch (section) {
        case TYPE:
            return i18n("Event type");
        case DATE:
            return i18n("Date");
        case PERSON_ID:
            return i18n("Person ID");
        case DISPLAY_NAME:
            return i18n("Name");
        case EVENT_ID:
            return i18n("Event ID");
        default:
            return {};
    }
}

Qt::ItemFlags FamilyMembersModel::flags(const QModelIndex& index) const {
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}
