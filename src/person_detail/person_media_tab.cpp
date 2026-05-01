/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "person_media_tab.h"

#include "domain/media/media_repository.h"
#include "ui/media/media_list_widget.h"

#include <QVBoxLayout>

PersonMediaTab::PersonMediaTab(IntegerPrimaryKey personId, QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    auto* mediaWidget = new MediaListWidget(
        [personId] { return MediaRepository().findForPerson(personId); },
        [personId](IntegerPrimaryKey mediaId) { return MediaRepository().attachToPerson(personId, mediaId); },
        [personId](IntegerPrimaryKey mediaId) { return MediaRepository().detachFromPerson(personId, mediaId); },
        this
    );
    layout->addWidget(mediaWidget);
}
