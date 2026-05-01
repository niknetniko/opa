/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "media_list_model.h"

#include "../../core/data_event_broker.h"
#include "media_repository.h"

#include <KLocalizedString>
#include <QFileInfo>

using namespace Qt::StringLiterals;

MediaListModel::MediaListModel(QObject* parent) : ObjectTableModel(parent) {
    this->setColumn(ID, i18n("ID"), &MediaEntity::id);
    this->setColumn(TITLE, i18n("Title"), [](const MediaEntity& e) -> QVariant {
        return e.title.value_or(QFileInfo(e.path).fileName());
    });
    this->setColumn(PATH, i18n("File"), &MediaEntity::path);
    this->setColumn(MIME_TYPE, i18n("Type"), &MediaEntity::mimeType);
    connectToTable<Schema::Media>(this);
    reload();
}

void MediaListModel::reload() {
    MediaRepository repo;
    this->setItems(repo.findAll());
}
