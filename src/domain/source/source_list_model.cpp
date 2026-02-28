/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "source_list_model.h"

#include "../../core/data_event_broker.h"
#include "source_repository.h"

#include <KLocalizedString>

SourcesListModel::SourcesListModel(QObject* parent) : ObjectTableModel(parent) {
    this->setColumn(ID, i18n("ID"), &SourceEntity::id);
    this->setColumn(TITLE, i18n("Title"), &SourceEntity::title);
    this->setColumn(TYPE, i18n("Type"), &SourceEntity::type);
    this->setColumn(AUTHOR, i18n("Author"), &SourceEntity::author);
    this->setColumn(PUBLICATION, i18n("Publication"), &SourceEntity::publication);
    this->setColumn(CONFIDENCE, i18n("Confidence"), &SourceEntity::confidence);
    this->setColumn(NOTE, i18n("Note"), &SourceEntity::note);
    this->setColumn(PARENT_ID, i18n("Parent"), [](const SourceEntity& e) -> QVariant {
        return e.parentId.has_value() ? QVariant(e.parentId.value()) : QVariant{};
    });
    connectToTable<Schema::Sources>(this);
    reload();
}

void SourcesListModel::reload() {
    SourceRepository repo;
    this->setItems(repo.findAll());
}
