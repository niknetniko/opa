#include "simple_list_manager.h"

#include <QToolBar>
#include <KLocalizedString>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QProgressDialog>
#include <QSortFilterProxyModel>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>

#include "database/schema.h"
#include "utils/builtin_model.h"
#include "utils/builtin_text_translating_delegate.h"
#include "utils/edit_proxy_model.h"
#include "utils/model_utils.h"

SimpleListManagementWindow::SimpleListManagementWindow(QWidget *parent): QWidget(parent, Qt::Window) {
}

void SimpleListManagementWindow::addItem() const {
    auto newRecord = this->model->record();
    newRecord.setGenerated(idColumn, false);
    newRecord.setGenerated(builtinColumn, false);
    if (!this->model->insertRecord(-1, newRecord)) {
        qWarning() << model->lastError();
        return;
    }
    // The table might be sorted, so we need to select the row with the highest ID.
    auto id = this->model->query().lastInsertId();

    auto searchIndex = this->tableView->model()->index(0, 0);
    auto newlyInserted = this->tableView->model()->match(searchIndex, Qt::DisplayRole, id);

    if (newlyInserted.isEmpty()) {
        qWarning() << "Could not find newly inserted item...";
        return;
    }

    auto insertedIndex = newlyInserted.first();
    auto editIndex = this->tableView->model()->index(insertedIndex.row(), displayColumn);

    tableView->scrollTo(insertedIndex);
    tableView->setFocus();
    tableView->selectionModel()->select(insertedIndex,
                                        QItemSelectionModel::ClearAndSelect | QItemSelectionModel::SelectCurrent);
    tableView->edit(editIndex);
}

void SimpleListManagementWindow::removeItem() const {
    auto selection = this->tableView->selectionModel();
    if (!selection->hasSelection()) {
        return;
    }

    auto selectedIndex = selection->selection().first().indexes().first();
    auto rootIndex = map_to_source_model(selectedIndex);

    this->model->removeRow(rootIndex.row());
    this->model->select();
}

void SimpleListManagementWindow::repairItems() {
    if (!repairConfirmation()) {
        return;
    }

    QProgressDialog progress(i18n("Opschonen..."), QString(), 0, 5, this);
    progress.setModal(true);
    progress.setValue(0);

    // Trim all values.
    for (int r = 0; r < this->model->rowCount(); ++r) {
        auto index = this->model->index(r, displayColumn);
        auto trimmed = index.data().toString().simplified();
        auto lowered = trimmed.toLower();
        if (!lowered.isEmpty()) {
            lowered[0] = lowered[0].toTitleCase();
        }
        this->model->setData(index, lowered);
    }
    progress.setValue(1);

    // Determine duplicates
    QHash<QString, QVector<IntegerPrimaryKey> > valueToIds;
    QHash<IntegerPrimaryKey, QString> idToValue;
    for (int r = 0; r < this->model->rowCount(); ++r) {
        auto index = this->model->index(r, idColumn).data().toLongLong();
        auto value = this->model->index(r, displayColumn).data().toString();
        valueToIds[value].append(index);
        idToValue[index] = value;
    }
    progress.setValue(2);

    this->removeMarkedReferences(valueToIds, idToValue);
    progress.setValue(3);

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
    progress.setValue(4);

    // Finally, remove the rows that are in the set.
    for (int r = this->model->rowCount() - 1; r >= 0; r--) {
        if (toRemove.contains(this->model->index(r, idColumn).data().toLongLong())) {
            if (this->model->removeRow(r)) {
                qWarning() << "Could not remove row " << r << "!";
                qWarning() << model->lastError();
            }
        }
    }

    this->model->select();
    progress.setValue(5);
}

void SimpleListManagementWindow::onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected) {
    if (selected.isEmpty()) {
        this->removeAction->setEnabled(false);
        return;
    }

    auto selectedIndex = selected.indexes().first();
    auto rootIndex = map_to_source_model(selectedIndex);

    if (this->model->index(rootIndex.row(), builtinColumn).data().toBool()) {
        this->removeAction->setEnabled(false);
        return;
    }

    auto id = this->model->index(rootIndex.row(), idColumn).data();

    this->removeAction->setEnabled(!isUsed(id));
}

void SimpleListManagementWindow::initializeLayout() {
    auto *toolbar = new QToolBar(this);
    toolbar->setOrientation(Qt::Vertical);
    toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    auto addAction = new QAction(toolbar);
    addAction->setText(i18n("Add"));
    addAction->setToolTip(addItemDescription());
    addAction->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
    toolbar->addAction(addAction);
    connect(addAction, &QAction::triggered, this, &SimpleListManagementWindow::addItem);

    removeAction = new QAction(toolbar);
    removeAction->setText(i18n("Delete"));
    removeAction->setToolTip(removeItemDescription());
    removeAction->setIcon(QIcon::fromTheme(QStringLiteral("edit-delete")));
    removeAction->setEnabled(false);
    toolbar->addAction(removeAction);
    connect(removeAction, &QAction::triggered, this, &SimpleListManagementWindow::removeItem);

    auto repairAction = new QAction(toolbar);
    repairAction->setText(i18n("Clean up"));
    repairAction->setToolTip(repairItemDescription());
    repairAction->setIcon(QIcon::fromTheme(QStringLiteral("tools-wizard")));
    repairAction->setEnabled(true);
    toolbar->addAction(repairAction);
    connect(repairAction, &QAction::triggered, this, &SimpleListManagementWindow::repairItems);

    // Show an icon for the built-in rows.
    auto builtinIconModel = new BuiltinModel(this);
    builtinIconModel->setSourceModel(model);
    builtinIconModel->setColumns(builtinColumn, displayColumn);

    // Make only the ID column not editable.
    auto editableModel = new EditProxyModel(this);
    editableModel->setSourceModel(builtinIconModel);
    editableModel->addReadOnlyColumns({idColumn});

    // We want to filter and sort.
    auto filterProxyModel = new QSortFilterProxyModel(this);
    filterProxyModel->setSourceModel(editableModel);

    tableView = new QTableView(this);
    tableView->setModel(filterProxyModel);
    tableView->setSortingEnabled(true);
    tableView->sortByColumn(idColumn, Qt::AscendingOrder);
    tableView->setShowGrid(false);
    tableView->setSelectionMode(QTableView::SelectionMode::SingleSelection);
    tableView->verticalHeader()->hide();
    tableView->horizontalHeader()->resizeSections(QHeaderView::Stretch);
    tableView->horizontalHeader()->setSectionResizeMode(idColumn, QHeaderView::ResizeToContents);
    tableView->horizontalHeader()->setSectionResizeMode(displayColumn, QHeaderView::Stretch);
    tableView->horizontalHeader()->setHighlightSections(false);

    auto originTranslator = new BuiltinTextTranslatingDelegate(tableView);
    originTranslator->setTranslator(translator);
    tableView->setItemDelegateForColumn(displayColumn, originTranslator);

    connect(tableView->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this,
            &SimpleListManagementWindow::onSelectionChanged);

    // Support models being reset.
    connect(tableView->model(), &QAbstractItemModel::modelReset, this, [this]() {
        this->onSelectionChanged(QItemSelection(), QItemSelection());
    });

    auto *layout = new QHBoxLayout(this);
    layout->setSpacing(0);
    layout->addWidget(tableView);
    layout->addWidget(toolbar);
}

void SimpleListManagementWindow::setColumns(int idColumn, int displayColumn, int builtinColumn) {
    this->idColumn = idColumn;
    this->displayColumn = displayColumn;
    this->builtinColumn = builtinColumn;
}

void SimpleListManagementWindow::setTranslator(const std::function<QString(QString)> &translator) {
    this->translator = translator;
}

void SimpleListManagementWindow::setModel(QSqlTableModel *model) {
    this->model = model;
}
