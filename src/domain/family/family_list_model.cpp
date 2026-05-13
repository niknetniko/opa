/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "family_list_model.h"

#include "core/data_event_broker.h"
#include "dates/genealogical_date.h"
#include "domain/name/names.h"
#include "family_repository.h"

#include <KLocalizedString>

// Internal ID encoding mirrors FamilyMembersModel:
//   Top-level rows: high 32 bits = 0, low 32 = family list index
//   Child rows:     high 32 bits = (familyListIndex + 1), low 32 = (rowIndex + 1)

namespace {
quintptr encodeParent(int familyListIndex) {
    return static_cast<quintptr>(static_cast<quint32>(familyListIndex));
}

quintptr encodeChild(int familyListIndex, int rowIndex) {
    return (static_cast<quintptr>(static_cast<quint32>(familyListIndex + 1)) << 32U) |
           static_cast<quintptr>(static_cast<quint32>(rowIndex + 1));
}

bool isChildId(quintptr id) {
    return (id >> 32U) != 0;
}
}

FamilyListModel::FamilyListModel(QObject* parent) : QAbstractItemModel(parent) {
    connectToTable<Schema::Families>(this);
    connectToTable<Schema::Events>(this);
    connectToTable<Schema::EventRelations>(this);
    connectToTable<Schema::Names>(this);

    reload();
}

void FamilyListModel::reload() {
    beginResetModel();
    FamilyRepository repo;
    rows = repo.findAllFamiliesOverview();
    rebuildMapping();
    endResetModel();
}

void FamilyListModel::rebuildMapping() {
    families.clear();
    childRows.clear();
    familyDisplayNames.clear();

    for (int i = 0; i < rows.size(); ++i) {
        const auto& row = rows[i];
        if (!familyDisplayNames.contains(row.familyId)) {
            families.append(row.familyId);
            familyDisplayNames[row.familyId] = row.familyDisplayName;
        }
        childRows[row.familyId].append(i);
    }
}

QModelIndex FamilyListModel::index(int row, int column, const QModelIndex& parent) const {
    if (!hasIndex(row, column, parent)) {
        return {};
    }

    if (!parent.isValid()) {
        if (row >= families.size()) {
            return {};
        }
        return createIndex(row, column, encodeParent(row));
    }

    int familyListIndex = static_cast<int>(static_cast<quint32>(parent.internalId()));
    if (familyListIndex >= families.size()) {
        return {};
    }
    const auto& children = childRows[families[familyListIndex]];
    if (row >= children.size()) {
        return {};
    }
    return createIndex(row, column, encodeChild(familyListIndex, children[row]));
}

QModelIndex FamilyListModel::parent(const QModelIndex& child) const {
    if (!child.isValid()) {
        return {};
    }

    quintptr id = child.internalId();
    if (!isChildId(id)) {
        return {};
    }

    int familyListIndex = static_cast<int>(static_cast<quint32>(id >> 32U)) - 1;
    if (familyListIndex < 0 || familyListIndex >= families.size()) {
        return {};
    }
    return createIndex(familyListIndex, 0, encodeParent(familyListIndex));
}

int FamilyListModel::rowCount(const QModelIndex& parent) const {
    if (!parent.isValid()) {
        return families.size();
    }
    if (parent.column() != 0) {
        return 0;
    }

    quintptr id = parent.internalId();
    if (isChildId(id)) {
        return 0;
    }

    int familyListIndex = static_cast<int>(static_cast<quint32>(id));
    if (familyListIndex >= families.size()) {
        return 0;
    }
    return childRows[families[familyListIndex]].size();
}

int FamilyListModel::columnCount(const QModelIndex& parent) const {
    Q_UNUSED(parent);
    return 7; // DISPLAY_NAME, TYPE, DATE, ROLE, PERSON_ID, EVENT_ID, FAMILY_ID
}

QVariant FamilyListModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) {
        return {};
    }
    if (role != Qt::DisplayRole && role != Qt::EditRole) {
        return {};
    }

    quintptr id = index.internalId();
    if (!isChildId(id)) {
        int familyListIndex = static_cast<int>(static_cast<quint32>(id));
        if (familyListIndex >= families.size()) {
            return {};
        }
        IntegerPrimaryKey familyId = families[familyListIndex];
        switch (index.column()) {
            case FAMILY_ID:
                return familyId;
            case DISPLAY_NAME:
                return familyDisplayNames[familyId];
            default:
                return {};
        }
    }

    int rowIndex = static_cast<int>(static_cast<quint32>(id)) - 1;
    if (rowIndex < 0 || rowIndex >= rows.size()) {
        return {};
    }
    const auto& row = rows[rowIndex];
    switch (index.column()) {
        case FAMILY_ID:
            return {};
        case DISPLAY_NAME:
            return construct_display_name(row.titles, row.givenNames, row.prefix, row.surname);
        case TYPE:
            return row.eventType;
        case DATE:
            if (row.eventDate.isEmpty()) {
                return {};
            }
            return GenealogicalDate::fromDatabaseRepresentation(row.eventDate).toLocalizedText();
        case ROLE:
            return row.role;
        case PERSON_ID:
            return row.personId;
        case EVENT_ID:
            return row.eventId;
        default:
            return {};
    }
}

QVariant FamilyListModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal) {
        return {};
    }
    switch (section) {
        case FAMILY_ID:
            return i18n("Family ID");
        case DISPLAY_NAME:
            return i18n("Name");
        case TYPE:
            return i18n("Event type");
        case DATE:
            return i18n("Date");
        case ROLE:
            return i18n("Role");
        case PERSON_ID:
            return i18n("Person ID");
        case EVENT_ID:
            return i18n("Event ID");
        default:
            return {};
    }
}

Qt::ItemFlags FamilyListModel::flags(const QModelIndex& index) const {
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}
