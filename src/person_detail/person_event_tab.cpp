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
    auto* toolbar = new QToolBar(this);

    this->addAction = new QAction(toolbar);
    this->addAction->setText(i18n("Add new event"));
    this->addAction->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
    toolbar->addAction(this->addAction);

    this->editAction = new QAction(toolbar);
    this->editAction->setText(i18n("Edit event"));
    this->editAction->setIcon(QIcon::fromTheme(QStringLiteral("edit-entry")));
    this->editAction->setEnabled(false);
    toolbar->addAction(this->editAction);

    this->removeAction = new QAction(toolbar);
    this->removeAction->setText(i18n("Remove event"));
    this->removeAction->setIcon(QIcon::fromTheme(QStringLiteral("edit-delete")));
    this->removeAction->setEnabled(false);
    toolbar->addAction(this->removeAction);

    this->linkAction = new QAction(toolbar);
    this->linkAction->setText(i18n("Link event"));
    this->linkAction->setIcon(QIcon::fromTheme(QStringLiteral("insert-link")));
    toolbar->addAction(this->linkAction);

    this->unlinkAction = new QAction(toolbar);
    this->unlinkAction->setText(i18n("Unlink event"));
    this->unlinkAction->setIcon(QIcon::fromTheme(QStringLiteral("remove-link")));
    this->unlinkAction->setEnabled(false);
    toolbar->addAction(this->unlinkAction);

    // Create a table.
    auto* eventView = new EventsOverviewView(person, this);

    // Add them together.
    auto* nameTabContainerLayout = new QVBoxLayout(this);
    nameTabContainerLayout->setSpacing(0);
    nameTabContainerLayout->addWidget(toolbar);
    nameTabContainerLayout->addWidget(eventView);

    connect(addAction, &QAction::triggered, eventView, &EventsOverviewView::handleNewEvent);
    connect(eventView, &EventsOverviewView::selectedEvent, this, &PersonEventTab::onEventSelected);
    connect(editAction, &QAction::triggered, eventView, &EventsOverviewView::editSelectedEvent);
    connect(removeAction, &QAction::triggered, eventView, &EventsOverviewView::removeSelectedEvent);
    connect(unlinkAction, &QAction::triggered, eventView, &EventsOverviewView::unlinkSelectedEvent);
    connect(linkAction, &QAction::triggered, eventView, &EventsOverviewView::linkExistingEvent);
}

void PersonEventTab::onEventSelected([[maybe_unused]] const QAbstractItemModel* model, const QItemSelection& selected)
    const {
    // TODO: prevent something here?
    this->editAction->setEnabled(!selected.isEmpty());
    this->removeAction->setEnabled(!selected.isEmpty());
    this->unlinkAction->setEnabled(!selected.isEmpty());
}
