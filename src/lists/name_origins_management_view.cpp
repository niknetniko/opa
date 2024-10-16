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
#include "name_origins_management_view.h"
#include "data/data_manager.h"
#include "data/names.h"

NameOriginsManagementWindow::NameOriginsManagementWindow(QWidget *parent) : QWidget(parent, Qt::Window) {
    this->setWindowTitle(i18n("Beheer naamoorsprongen"));

    auto *nameToolbar = new QToolBar(this);
    nameToolbar->setOrientation(Qt::Vertical);
    nameToolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    auto addAction = new QAction(nameToolbar);
    addAction->setText(i18n("Toevoegen"));
    addAction->setToolTip(i18n("Voeg een nieuwe naamoorsprong toe"));
    addAction->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
    nameToolbar->addAction(addAction);
    connect(addAction, &QAction::triggered, this, &NameOriginsManagementWindow::addOrigin);

    this->removeAction = new QAction(nameToolbar);
    removeAction->setText(i18n("Verwijderen"));
    removeAction->setToolTip(i18n("Verwijder deze naamoorsprong"));
    removeAction->setIcon(QIcon::fromTheme(QStringLiteral("edit-delete")));
    removeAction->setEnabled(false);
    nameToolbar->addAction(removeAction);
    connect(removeAction, &QAction::triggered, this, &NameOriginsManagementWindow::removeOrigin);

    auto repairAction = new QAction(nameToolbar);
    repairAction->setText(i18n("Opschonen"));
    repairAction->setToolTip(i18n("Verwijder lege en dubbele oorsprongen"));
    repairAction->setIcon(QIcon::fromTheme(QStringLiteral("tools-wizard")));
    repairAction->setEnabled(true);
    nameToolbar->addAction(repairAction);
    connect(repairAction, &QAction::triggered, this, &NameOriginsManagementWindow::repairOrigins);

    this->model = DataManager::get().nameOriginsModel();
    // We want to filter and sort.
    auto *filterProxyModel = new QSortFilterProxyModel(this);
    filterProxyModel->setSourceModel(model);

    tableView = new QTableView(this);
    tableView->setModel(filterProxyModel);
    tableView->setSortingEnabled(true);
    tableView->sortByColumn(NameOriginTableModel::ID, Qt::AscendingOrder);
    tableView->setShowGrid(false);
    tableView->setSelectionMode(QTableView::SelectionMode::SingleSelection);
    tableView->verticalHeader()->hide();
    tableView->horizontalHeader()->resizeSections(QHeaderView::Stretch);
    tableView->horizontalHeader()->setSectionResizeMode(NameOriginTableModel::ID, QHeaderView::ResizeToContents);
    tableView->horizontalHeader()->setSectionResizeMode(NameOriginTableModel::ORIGIN, QHeaderView::Stretch);
    tableView->horizontalHeader()->setHighlightSections(false);

    connect(tableView->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this,
            &NameOriginsManagementWindow::onSelectionChanged);
    // Support models being reset.
    connect(tableView->model(), &QAbstractItemModel::modelReset, this, [this]() {
        this->onSelectionChanged(QItemSelection(), QItemSelection());
    });

    auto *layout = new QHBoxLayout(this);
    layout->setSpacing(0);
    layout->addWidget(tableView);
    layout->addWidget(nameToolbar);
}

void NameOriginsManagementWindow::addOrigin() {
    auto newRecord = this->model->record();
    newRecord.setGenerated(NameOriginTableModel::ID, false);
    this->model->insertRecord(-1, newRecord);
    // The table might be sorted, so we need to select the row with the highest ID.
    auto id = this->model->query().lastInsertId();
    // Find the row with that ID.
    for (int r = 0; r < this->tableView->model()->rowCount(); ++r) {
        auto thisRow = this->tableView->model()->index(r, NameOriginTableModel::ID);
        if (thisRow.data() == id) {
            auto editableFieldIndex = this->tableView->model()->index(r, NameOriginTableModel::ORIGIN);
            tableView->scrollTo(editableFieldIndex);
            tableView->setFocus();
            tableView->selectionModel()->select(editableFieldIndex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::SelectCurrent);
            tableView->edit(editableFieldIndex);
            break;
        }
    }
}

void NameOriginsManagementWindow::onSelectionChanged(const QItemSelection &selected, [[maybe_unused]] const QItemSelection &deselected) {
    if (selected.isEmpty()) {
        this->removeAction->setEnabled(false);
        return;
    }

    // TODO: change this to be able to use the ID instead of the actual name.
    // Get the ID of the current selected row.
    auto id = this->model->index(selected.indexes().first().row(), NameOriginTableModel::ID).data();

    qDebug() << "Checking name with id " << id << " for usage";

    // TODO: this is not very efficient
    auto *nameModel = DataManager::get().namesModel();

    auto usage = nameModel->match(nameModel->index(0, NamesTableModel::ORIGIN_ID), Qt::DisplayRole, id);
    this->removeAction->setEnabled(usage.isEmpty());
}

void NameOriginsManagementWindow::removeOrigin() {
    auto selection = this->tableView->selectionModel();
    if (!selection->hasSelection()) {
        return;
    }

    auto selectRow = selection->selection().first().indexes().first().row();
    this->tableView->model()->removeRow(selectRow);
    // Refresh the data since we deleted a row.
    this->model->select();
}

void NameOriginsManagementWindow::repairOrigins() {
    QMessageBox messageBox(this);
    messageBox.setIcon(QMessageBox::Question);
    messageBox.setText(i18n("Naamoorsprongen opruimen?"));
    messageBox.setInformativeText(
            i18n("Dit zal dubbele oorsprongen samenvoegen en lege oorsprongen verwijderen. Wilt u doorgaan?"));
    messageBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    messageBox.setDefaultButton(QMessageBox::Ok);

    if (messageBox.exec() == QMessageBox::Cancel) {
        return;
    }

    // This is a blocking operation but should not take that long.
    QProgressDialog progress(i18n("Oorsprongen opschonen..."), QString(), 0, 4, this);
    progress.setModal(true);
    progress.setValue(0);

    // Trim all origins.
    for (int r = 0; r < this->model->rowCount(); ++r) {
        auto index = this->model->index(r, NameOriginTableModel::ORIGIN);
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
        auto index = this->model->index(r, NameOriginTableModel::ID).data().toLongLong();
        auto value = this->model->index(r, NameOriginTableModel::ORIGIN).data().toString();
        valueToIds[value].append(index);
        idToValue[index] = value;
    }
    qDebug() << "Values to ID is " << valueToIds;
    progress.setValue(2);

    auto nameModel = DataManager::get().namesModel();

    for (int r = 0; r < nameModel->rowCount(); ++r) {
        auto index = nameModel->index(r, NamesTableModel::ORIGIN_ID);

        qDebug() << "Considering name " << nameModel->index(r, NamesTableModel::ID).data();
        qDebug() << "  data is " << index.data();

        // If the model is empty, stop it now.
        if (!index.isValid() || index.data().isNull()) {
            qDebug() << "  has empty origin, so unset the table";
            nameModel->setData(index, QString());
            continue;
        }

        auto originInModel = index.data().toLongLong();
        auto value = idToValue[originInModel];
        auto idsForThisOrigin = valueToIds[value];

        // If there are no duplicates, we do not need to update anything.
        if (idsForThisOrigin.length() == 1) {
            qDebug() << "  has origin without duplicates";
            continue;
        }

        // Update the name to point to the first origin.
        qDebug() << "  has duplicated origin in " << idsForThisOrigin;
        auto keepId = idsForThisOrigin.first();
        qDebug() << "   will keep " << keepId;
        nameModel->setData(index, keepId);
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
        auto id = this->model->index(r, NameOriginTableModel::ID).data().toLongLong();
        if (toRemove.contains(id)) {
            qDebug() << "Will remove row " << r << " with ID " << id;
            bool res = this->model->removeRow(r);
            qDebug() << "Effect of removal is " << res;
        }
    }

    this->model->select();
    progress.setValue(4);
}
