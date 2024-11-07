//
// Created by niko on 28/09/24.
//

#include <KLocalizedString>
#include <QToolBar>
#include <QVBoxLayout>

#include "data/names.h"
#include "names/names_overview_view.h"
#include "person_name_tab.h"

PersonNameTab::PersonNameTab(IntegerPrimaryKey person, QWidget *parent) : QWidget(parent) {
    // Create a toolbar.
    auto *nameToolbar = new QToolBar(this);

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
    this->removeAction->setText(i18n("Naam verwijderen"));
    this->removeAction->setIcon(QIcon::fromTheme(QStringLiteral("edit-delete")));
    this->removeAction->setEnabled(false);
    nameToolbar->addAction(this->removeAction);

    this->upAction = new QAction(nameToolbar);
    this->upAction->setText(i18n("Omhoog"));
    this->upAction->setIcon(QIcon::fromTheme(QStringLiteral("go-up")));
    this->upAction->setEnabled(false);
    nameToolbar->addAction(this->upAction);

    this->downAction = new QAction(nameToolbar);
    this->downAction->setText(i18n("Omlaag"));
    this->downAction->setIcon(QIcon::fromTheme(QStringLiteral("go-down")));
    this->downAction->setEnabled(false);
    nameToolbar->addAction(this->downAction);

    // Create a table.
    auto *nameTableView = new NamesOverviewView(person, this);

    // Add them together.
    auto *nameTabContainerLayout = new QVBoxLayout(this);
    nameTabContainerLayout->setSpacing(0);
    nameTabContainerLayout->addWidget(nameToolbar);
    nameTabContainerLayout->addWidget(nameTableView);

    // Connect the buttons and stuff.
    // Allow adding new persons.
    connect(this->addAction, &QAction::triggered, nameTableView, &NamesOverviewView::handleNewName);
    // Listen to when a name is selected, to enable or disable some buttons.
    connect(nameTableView, &NamesOverviewView::selectedName, this, &PersonNameTab::onNameSelected);
    // Listen to when the sort changes and allow re-ordering or not.
    connect(nameTableView, &NamesOverviewView::sortChanged, this, &PersonNameTab::onSortChanged);
    // Allow editing a name.
    connect(
        this->editAction, &QAction::triggered, nameTableView, &NamesOverviewView::editSelectedName
    );
    // Allow deleting a name.
    connect(
        this->removeAction,
        &QAction::triggered,
        nameTableView,
        &NamesOverviewView::removeSelectedName
    );

    connect(
        this->upAction, &QAction::triggered, nameTableView, &NamesOverviewView::moveSelectedNameUp
    );
    connect(
        this->downAction,
        &QAction::triggered,
        nameTableView,
        &NamesOverviewView::moveSelectedNameDown
    );
}

void PersonNameTab::onNameSelected(const QAbstractItemModel *model, const QItemSelection &selected)
    const {
    if (selected.isEmpty()) {
        this->editAction->setEnabled(false);
        this->removeAction->setEnabled(false);
        this->downAction->setEnabled(false);
        this->upAction->setEnabled(false);
        return;
    }

    this->editAction->setEnabled(true);

    const int nameSort = model->index(selected.indexes().first().row(), 1).data().toInt();
    const int rowCount = model->rowCount();

    // Do not allow removal of the main name.
    this->removeAction->setEnabled(nameSort != 1);
    // Do not allow moving the first name up.
    this->upAction->setEnabled(nameSort > 1 && rowCount > 1);
    // Do not allow moving the last name down.
    this->downAction->setEnabled(nameSort < model->rowCount() && rowCount > 1);
}

void PersonNameTab::onSortChanged(
    const QAbstractItemModel *model, const QItemSelection &selected, const int logicalIndex
) const {
    // Check if there is currently a selected item.
    if (selected.isEmpty()) {
        // Leave everything alone as it was.
        // The relevant actions should already be disabled.
        return;
    }

    const bool allowUpAndDown = logicalIndex < 0 || logicalIndex == 1;
    // Only allow up and down if the view is not sorted or sorted on the second column.
    this->upAction->setEnabled(this->upAction->isEnabled() && allowUpAndDown);
    this->downAction->setEnabled(this->downAction->isEnabled() && allowUpAndDown);
}
