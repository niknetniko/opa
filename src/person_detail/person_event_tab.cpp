/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "person_event_tab.h"

#include "person_event_overview_view.h"

#include <KLocalizedString>
#include <QToolBar>
#include <QVBoxLayout>

PersonEventTab::PersonEventTab(IntegerPrimaryKey person, QWidget* parent) : QWidget(parent) {
    // Create a toolbar.
    auto* nameToolbar = new QToolBar(this);

    this->addAction = new QAction(nameToolbar);
    this->addAction->setText(i18n("Nieuwe naam toevoegen"));
    this->addAction->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
    nameToolbar->addAction(this->addAction);

    this->editAction = new QAction(nameToolbar);
    this->editAction->setText(i18n("Naam bewerken"));
    this->editAction->setIcon(QIcon::fromTheme(QStringLiteral("edit-entry")));
    this->editAction->setEnabled(false);
    nameToolbar->addAction(this->editAction);

    this->removeAction = new QAction(nameToolbar);
    this->removeAction->setText(i18n("Remove event"));
    this->removeAction->setIcon(QIcon::fromTheme(QStringLiteral("edit-delete")));
    this->removeAction->setEnabled(false);
    nameToolbar->addAction(this->removeAction);

    this->unlinkAction = new QAction(nameToolbar);
    this->unlinkAction->setText(i18n("Unlink event"));
    this->unlinkAction->setIcon(QIcon::fromTheme(QStringLiteral("remove-link")));
    this->unlinkAction->setEnabled(false);
    nameToolbar->addAction(this->unlinkAction);

    // Create a table.
    auto* eventView = new EventsOverviewView(person, this);

    // Add them together.
    auto* nameTabContainerLayout = new QVBoxLayout(this);
    nameTabContainerLayout->setSpacing(0);
    nameTabContainerLayout->addWidget(nameToolbar);
    nameTabContainerLayout->addWidget(eventView);

    connect(addAction, &QAction::triggered, eventView, &EventsOverviewView::handleNewEvent);
    connect(eventView, &EventsOverviewView::selectedEvent, this, &PersonEventTab::onEventSelected);
    connect(editAction, &QAction::triggered, eventView, &EventsOverviewView::editSelectedEvent);
    connect(removeAction, &QAction::triggered, eventView, &EventsOverviewView::removeSelectedEvent);
    connect(unlinkAction, &QAction::triggered, eventView, &EventsOverviewView::unlinkSelectedEvent);
}

void PersonEventTab::onEventSelected([[maybe_unused]] const QAbstractItemModel* model, const QItemSelection& selected)
    const {
    // TODO: prevent something here?
    this->editAction->setEnabled(!selected.isEmpty());
    this->removeAction->setEnabled(!selected.isEmpty());
    this->unlinkAction->setEnabled(!selected.isEmpty());
}
