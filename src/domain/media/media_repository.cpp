/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "media_repository.h"

#include "../../core/data_event_broker.h"
#include "../../core/query_helper.h"

#include <optional>

using namespace Qt::StringLiterals;

static const auto SELECT_MEDIA = u"SELECT m.id, m.path, m.title, m.note, m.mime_type FROM media m"_s;

QList<MediaEntity> MediaRepository::findAll() const {
    return fetchAll<MediaEntity>(SELECT_MEDIA + u" ORDER BY m.title ASC, m.path ASC"_s, {});
}

std::optional<MediaEntity> MediaRepository::findById(IntegerPrimaryKey id) const {
    return fetchOne<MediaEntity>(SELECT_MEDIA + u" WHERE m.id = :id"_s, {{u":id"_s, id}});
}

std::optional<IntegerPrimaryKey> MediaRepository::insert(
    const QString& path,
    const std::optional<QString>& title,
    const std::optional<QString>& note,
    const QString& mimeType
) const {
    const auto sql = u"INSERT INTO media (path, title, note, mime_type) VALUES (:path, :title, :note, :mime_type)"_s;
    const auto newId = QueryHelper::insert(
        sql,
        {
            {u":path"_s, path},
            {u":title"_s, title ? QVariant(*title) : QVariant{}},
            {u":note"_s, note ? QVariant(*note) : QVariant{}},
            {u":mime_type"_s, mimeType},
        }
    );
    if (newId) {
        DataEventBroker::instance().notifyChanged<Schema::Media>(*newId);
    }
    return newId;
}

bool MediaRepository::update(
    IntegerPrimaryKey id,
    const std::optional<QString>& title,
    const std::optional<QString>& note
) const {
    return QueryHelper::executeAndNotify<Schema::Media>(
        id,
        u"UPDATE media SET title = :title, note = :note WHERE id = :id"_s,
        {{u":title"_s, title ? QVariant(*title) : QVariant{}},
         {u":note"_s, note ? QVariant(*note) : QVariant{}},
         {u":id"_s, id}}
    );
}

bool MediaRepository::remove(IntegerPrimaryKey id) const {
    return QueryHelper::executeAndNotify<Schema::Media>(id, u"DELETE FROM media WHERE id = :id"_s, {{u":id"_s, id}});
}

// --- Person ---

QList<MediaEntity> MediaRepository::findForPerson(IntegerPrimaryKey personId) const {
    return fetchAll<MediaEntity>(
        SELECT_MEDIA + u" JOIN person_media j ON j.media_id = m.id WHERE j.person_id = :id ORDER BY m.title ASC"_s,
        {{u":id"_s, personId}}
    );
}

bool MediaRepository::attachToPerson(IntegerPrimaryKey personId, IntegerPrimaryKey mediaId) const {
    return QueryHelper::execute(
        u"INSERT OR IGNORE INTO person_media (person_id, media_id) VALUES (:person_id, :media_id)"_s,
        {{u":person_id"_s, personId}, {u":media_id"_s, mediaId}}
    );
}

bool MediaRepository::detachFromPerson(IntegerPrimaryKey personId, IntegerPrimaryKey mediaId) const {
    return QueryHelper::execute(
        u"DELETE FROM person_media WHERE person_id = :person_id AND media_id = :media_id"_s,
        {{u":person_id"_s, personId}, {u":media_id"_s, mediaId}}
    );
}

// --- Name ---

QList<MediaEntity> MediaRepository::findForName(IntegerPrimaryKey nameId) const {
    return fetchAll<MediaEntity>(
        SELECT_MEDIA + u" JOIN name_media j ON j.media_id = m.id WHERE j.name_id = :id ORDER BY m.title ASC"_s,
        {{u":id"_s, nameId}}
    );
}

bool MediaRepository::attachToName(IntegerPrimaryKey nameId, IntegerPrimaryKey mediaId) const {
    return QueryHelper::execute(
        u"INSERT OR IGNORE INTO name_media (name_id, media_id) VALUES (:name_id, :media_id)"_s,
        {{u":name_id"_s, nameId}, {u":media_id"_s, mediaId}}
    );
}

bool MediaRepository::detachFromName(IntegerPrimaryKey nameId, IntegerPrimaryKey mediaId) const {
    return QueryHelper::execute(
        u"DELETE FROM name_media WHERE name_id = :name_id AND media_id = :media_id"_s,
        {{u":name_id"_s, nameId}, {u":media_id"_s, mediaId}}
    );
}

// --- Event ---

QList<MediaEntity> MediaRepository::findForEvent(IntegerPrimaryKey eventId) const {
    return fetchAll<MediaEntity>(
        SELECT_MEDIA + u" JOIN event_media j ON j.media_id = m.id WHERE j.event_id = :id ORDER BY m.title ASC"_s,
        {{u":id"_s, eventId}}
    );
}

bool MediaRepository::attachToEvent(IntegerPrimaryKey eventId, IntegerPrimaryKey mediaId) const {
    return QueryHelper::execute(
        u"INSERT OR IGNORE INTO event_media (event_id, media_id) VALUES (:event_id, :media_id)"_s,
        {{u":event_id"_s, eventId}, {u":media_id"_s, mediaId}}
    );
}

bool MediaRepository::detachFromEvent(IntegerPrimaryKey eventId, IntegerPrimaryKey mediaId) const {
    return QueryHelper::execute(
        u"DELETE FROM event_media WHERE event_id = :event_id AND media_id = :media_id"_s,
        {{u":event_id"_s, eventId}, {u":media_id"_s, mediaId}}
    );
}

// --- EventRelation ---

QList<MediaEntity> MediaRepository::findForEventRelation(IntegerPrimaryKey relationId) const {
    return fetchAll<MediaEntity>(
        SELECT_MEDIA +
            u" JOIN event_relation_media j ON j.media_id = m.id WHERE j.event_relation_id = :id ORDER BY m.title ASC"_s,
        {{u":id"_s, relationId}}
    );
}

bool MediaRepository::attachToEventRelation(IntegerPrimaryKey relationId, IntegerPrimaryKey mediaId) const {
    return QueryHelper::execute(
        u"INSERT OR IGNORE INTO event_relation_media (event_relation_id, media_id) VALUES (:relation_id, :media_id)"_s,
        {{u":relation_id"_s, relationId}, {u":media_id"_s, mediaId}}
    );
}

bool MediaRepository::detachFromEventRelation(IntegerPrimaryKey relationId, IntegerPrimaryKey mediaId) const {
    return QueryHelper::execute(
        u"DELETE FROM event_relation_media WHERE event_relation_id = :relation_id AND media_id = :media_id"_s,
        {{u":relation_id"_s, relationId}, {u":media_id"_s, mediaId}}
    );
}

// --- Source ---

QList<MediaEntity> MediaRepository::findForSource(IntegerPrimaryKey sourceId) const {
    return fetchAll<MediaEntity>(
        SELECT_MEDIA + u" JOIN source_media j ON j.media_id = m.id WHERE j.source_id = :id ORDER BY m.title ASC"_s,
        {{u":id"_s, sourceId}}
    );
}

bool MediaRepository::attachToSource(IntegerPrimaryKey sourceId, IntegerPrimaryKey mediaId) const {
    return QueryHelper::execute(
        u"INSERT OR IGNORE INTO source_media (source_id, media_id) VALUES (:source_id, :media_id)"_s,
        {{u":source_id"_s, sourceId}, {u":media_id"_s, mediaId}}
    );
}

bool MediaRepository::detachFromSource(IntegerPrimaryKey sourceId, IntegerPrimaryKey mediaId) const {
    return QueryHelper::execute(
        u"DELETE FROM source_media WHERE source_id = :source_id AND media_id = :media_id"_s,
        {{u":source_id"_s, sourceId}, {u":media_id"_s, mediaId}}
    );
}

// --- Location ---

QList<MediaEntity> MediaRepository::findForLocation(IntegerPrimaryKey locationId) const {
    return fetchAll<MediaEntity>(
        SELECT_MEDIA + u" JOIN location_media j ON j.media_id = m.id WHERE j.location_id = :id ORDER BY m.title ASC"_s,
        {{u":id"_s, locationId}}
    );
}

bool MediaRepository::attachToLocation(IntegerPrimaryKey locationId, IntegerPrimaryKey mediaId) const {
    return QueryHelper::execute(
        u"INSERT OR IGNORE INTO location_media (location_id, media_id) VALUES (:location_id, :media_id)"_s,
        {{u":location_id"_s, locationId}, {u":media_id"_s, mediaId}}
    );
}

bool MediaRepository::detachFromLocation(IntegerPrimaryKey locationId, IntegerPrimaryKey mediaId) const {
    return QueryHelper::execute(
        u"DELETE FROM location_media WHERE location_id = :location_id AND media_id = :media_id"_s,
        {{u":location_id"_s, locationId}, {u":media_id"_s, mediaId}}
    );
}
