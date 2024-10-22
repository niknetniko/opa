//
// Created by niko on 2/10/24.
//

#include <KLocalizedString>
#include <QToolBar>
#include <QSortFilterProxyModel>
#include <QSqlQuery>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QSqlRecord>
#include <QMessageBox>
#include <QProgressDialog>
#include <QSqlError>

#include "event_roles_management_view.h"
#include "data/data_manager.h"
#include "data/event.h"
#include "utils/builtin_model.h"
#include "utils/builtin_text_translating_delegate.h"
#include "utils/edit_proxy_model.h"
#include "utils/model_utils.h"

EventRolesManagementWindow::EventRolesManagementWindow(QWidget *parent) : QWidget(parent, Qt::Window) {
    this->setWindowTitle(i18n("Beheer naamoorsprongen"));

    auto *nameToolbar = new QToolBar(this);
    nameToolbar->setOrientation(Qt::Vertical);
    nameToolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    auto addAction = new QAction(nameToolbar);
    addAction->setText(i18n("Toevoegen"));
    addAction->setToolTip(i18n("Voeg een nieuwe rol voor gebeurtenissen toe"));
    addAction->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
    nameToolbar->addAction(addAction);
    connect(addAction, &QAction::triggered, this, &EventRolesManagementWindow::addRole);

    this->removeAction = new QAction(nameToolbar);
    removeAction->setText(i18n("Verwijderen"));
    removeAction->setToolTip(i18n("Verwijder deze rol"));
    removeAction->setIcon(QIcon::fromTheme(QStringLiteral("edit-delete")));
    removeAction->setEnabled(false);
    nameToolbar->addAction(removeAction);
    connect(removeAction, &QAction::triggered, this, &EventRolesManagementWindow::removeRole);

    auto repairAction = new QAction(nameToolbar);
    repairAction->setText(i18n("Opschonen"));
    repairAction->setToolTip(i18n("Verwijder lege en dubbele rollen"));
    repairAction->setIcon(QIcon::fromTheme(QStringLiteral("tools-wizard")));
    repairAction->setEnabled(true);
    nameToolbar->addAction(repairAction);
    connect(repairAction, &QAction::triggered, this, &EventRolesManagementWindow::repairRoles);

    this->model = DataManager::get().eventRolesModel();

    // Show an icon for the built-in rows.
    auto builtinIconModel = new BuiltinModel(this);
    builtinIconModel->setSourceModel(model);
    builtinIconModel->setColumns(EventRolesModel::BUILTIN, EventRolesModel::ROLE);

    // Make only the ID column not editable.
    auto editableModel = new EditProxyModel(this);
    editableModel->setSourceModel(builtinIconModel);
    editableModel->addReadOnlyColumns({EventRolesModel::ID});

    // We want to filter and sort.
    auto filterProxyModel = new QSortFilterProxyModel(this);
    filterProxyModel->setSourceModel(editableModel);

    tableView = new QTableView(this);
    tableView->setModel(filterProxyModel);
    tableView->setSortingEnabled(true);
    tableView->sortByColumn(EventRolesModel::ID, Qt::AscendingOrder);
    tableView->setShowGrid(false);
    tableView->setSelectionMode(QTableView::SelectionMode::SingleSelection);
    tableView->verticalHeader()->hide();
    tableView->horizontalHeader()->resizeSections(QHeaderView::Stretch);
    tableView->horizontalHeader()->setSectionResizeMode(EventRolesModel::ID, QHeaderView::ResizeToContents);
    tableView->horizontalHeader()->setSectionResizeMode(EventRolesModel::ROLE, QHeaderView::Stretch);
    tableView->horizontalHeader()->setHighlightSections(false);

    auto originTranslator = new BuiltinTextTranslatingDelegate(tableView);
    originTranslator->setTranslator(EventRoles::toDisplayString);
    tableView->setItemDelegateForColumn(EventRolesModel::ROLE, originTranslator);

    connect(tableView->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this,
            &EventRolesManagementWindow::onSelectionChanged);
    // Support models being reset.
    connect(tableView->model(), &QAbstractItemModel::modelReset, this, [this]() {
        this->onSelectionChanged(QItemSelection(), QItemSelection());
    });

    auto *layout = new QHBoxLayout(this);
    layout->setSpacing(0);
    layout->addWidget(tableView);
    layout->addWidget(nameToolbar);
}

void EventRolesManagementWindow::addRole() {
    auto newRecord = this->model->record();
    newRecord.setGenerated(EventRolesModel::ID, false);
    newRecord.setGenerated(EventRolesModel::BUILTIN, false);
    if (!this->model->insertRecord(-1, newRecord)) {
        qWarning() << "Could not insert new name origin!";
        qDebug() << model->lastError();
        return;
    }
    // The table might be sorted, so we need to select the row with the highest ID.
    auto id = this->model->query().lastInsertId();

    auto searchIndex = this->tableView->model()->index(0, 0);
    auto newlyInserted = this->tableView->model()->match(searchIndex, Qt::DisplayRole, id);

    if (newlyInserted.isEmpty()) {
        qWarning() << "Could not find new origin for some reason";
        return;
    }

    tableView->scrollTo(newlyInserted.first());
    tableView->setFocus();
    tableView->selectionModel()->select(newlyInserted.first(), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::SelectCurrent);
    tableView->edit(newlyInserted.first());
}

void EventRolesManagementWindow::onSelectionChanged(const QItemSelection &selected, [[maybe_unused]] const QItemSelection &deselected) {
    if (selected.isEmpty()) {
        this->removeAction->setEnabled(false);
        return;
    }

    auto selectedIndex = selected.indexes().first();
    auto rootIndex = map_to_source_model(selectedIndex);

    if (this->model->index(rootIndex.row(), EventRolesModel::BUILTIN).data().toBool()) {
        this->removeAction->setEnabled(false);
        return;
    }

    // Get the ID of the current selected row.
    auto id = this->model->index(rootIndex.row(), EventRolesModel::ID).data();

    auto *relationModel = DataManager::get().eventRelationsModel();
    auto usage = relationModel->match(relationModel->index(0, EventRelationsModel::ROLE_ID), Qt::DisplayRole, id);
    this->removeAction->setEnabled(usage.isEmpty());
}

void EventRolesManagementWindow::removeRole() {
    auto selection = this->tableView->selectionModel();
    if (!selection->hasSelection()) {
        return;
    }

    auto selectedIndex = selection->selection().first().indexes().first();
    auto rootIndex = map_to_source_model(selectedIndex);

    this->model->removeRow(rootIndex.row());
    this->model->select();
}

void EventRolesManagementWindow::repairRoles() {
    QMessageBox messageBox(this);
    messageBox.setIcon(QMessageBox::Question);
    messageBox.setText(i18n("Rollen opruimen?"));
    messageBox.setInformativeText(
            i18n("Dit zal dubbele rollen samenvoegen en lege oorsprongen verwijderen. Wilt u doorgaan?"));
    messageBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    messageBox.setDefaultButton(QMessageBox::Ok);

    if (messageBox.exec() == QMessageBox::Cancel) {
        return;
    }

    // This is a blocking operation but should not take that long.
    QProgressDialog progress(i18n("Rollen opschonen..."), QString(), 0, 4, this);
    progress.setModal(true);
    progress.setValue(0);

    // Trim all roles.
    for (int r = 0; r < this->model->rowCount(); ++r) {
        auto index = this->model->index(r, EventRolesModel::ROLE);
        auto trimmed = index.data().toString().simplified();
        auto lowered = trimmed.toLower();
        if (!lowered.isEmpty()) {
            lowered[0] = lowered[0].toTitleCase();
        }

        qDebug() << "Trimmed origin " << lowered;
        this->model->setData(index, lowered);
    }
    progress.setValue(1);

    // Determine duplicates
    QHash<QString, QVector<IntegerPrimaryKey>> valueToIds;
    QHash<IntegerPrimaryKey, QString> idToValue;
    for (int r = 0; r < this->model->rowCount(); ++r) {
        auto index = this->model->index(r, EventRolesModel::ID).data().toLongLong();
        auto value = this->model->index(r, EventRolesModel::ROLE).data().toString();
        valueToIds[value].append(index);
        idToValue[index] = value;
    }
    qDebug() << "Values to ID is " << valueToIds;
    progress.setValue(2);

    auto relationModel = DataManager::get().eventRelationsModel();

    for (int r = 0; r < relationModel->rowCount(); ++r) {
        auto index = relationModel->index(r, EventRelationsModel::ROLE_ID);

        qDebug() << "Considering name " << relationModel->index(r, EventRolesModel::ID).data();
        qDebug() << "  data is " << index.data();

        // If the model is empty, stop it now.
        if (!index.isValid() || index.data().isNull()) {
            qDebug() << "  has empty origin, so unset the table";
            relationModel->setData(index, QString());
            continue;
        }

        auto originInModel = index.data().toLongLong();
        auto value = idToValue[originInModel];
        auto idsForThisRole = valueToIds[value];

        // If there are no duplicates, we do not need to update anything.
        if (idsForThisRole.length() == 1) {
            qDebug() << "  has role without duplicates";
            continue;
        }

        // Update the name to point to the first origin.
        qDebug() << "  has duplicated role in " << idsForThisRole;
        auto keepId = idsForThisRole.first();
        qDebug() << "   will keep " << keepId;
        relationModel->setData(index, keepId);
    }

    progress.setValue(2);

    // Determine the list of removals.
    QSet<IntegerPrimaryKey> toRemove;
    for (auto i = valueToIds.begin(); i != valueToIds.end(); ++i) {
        // Always add the empty origins.
        if (i.key() == QString()) {
            toRemove.unite(QSet(i.value().begin(), i.value().end()));
            continue;
        }
        // There is only one, so this is OK.
        if (i.value().length() == 1) {
            continue;
        }
        toRemove.unite(QSet(std::next(i.value().begin()), i.value().end()));
    }
    progress.setValue(3);

    // Finally, remove the rows that are in the set.
    for (int r = this->model->rowCount() - 1; r >= 0; r--) {
        auto id = this->model->index(r, EventRolesModel::ID).data().toLongLong();
        if (toRemove.contains(id)) {
            qDebug() << "Will remove row " << r << " with ID " << id;
            bool res = this->model->removeRow(r);
            qDebug() << "Effect of removal is " << res;
        }
    }

    this->model->select();
    progress.setValue(4);
}
