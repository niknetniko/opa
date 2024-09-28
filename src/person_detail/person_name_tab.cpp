//
// Created by niko on 28/09/24.
//

#include <QVBoxLayout>
#include <QToolBar>
#include <KLocalizedString>
#include "person_name_tab.h"
#include "names/names_overview_view.h"
#include "data/names.h"

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
    // Allow editing a name.
    connect(this->editAction, &QAction::triggered, nameTableView, &NamesOverviewView::editSelectedName);
    // Allow deleting a name.
    connect(this->removeAction, &QAction::triggered, nameTableView, &NamesOverviewView::removeSelectedName);
}

void PersonNameTab::onNameSelected(const QAbstractItemModel *model, const QItemSelection &selected) {
    if (selected.isEmpty()) {
        this->editAction->setEnabled(false);
        this->removeAction->setEnabled(false);
        return;
    }

    this->editAction->setEnabled(true);

    // We do not allow removal of the main name.
    // The MAIN column is the second one.
    // TODO: do not hardcode this.
    auto primaryNameIndex = model->index(selected.indexes().first().row(), 1);
    auto isPrimaryName = model->data(primaryNameIndex).toBool();
    this->removeAction->setEnabled(!isPrimaryName);
}
