#include "simple_list_manager.h"

#include <KLocalizedString>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QProgressDialog>
#include <QSortFilterProxyModel>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QStatusBar>
#include <QToolBar>

#include "database/schema.h"
#include "utils/builtin_model.h"
#include "utils/edit_proxy_model.h"
#include "utils/model_utils.h"

StatusTooltipModel::StatusTooltipModel(SimpleListManagementWindow *parent): QIdentityProxyModel(parent) {
}

QVariant StatusTooltipModel::data(const QModelIndex &index, int role) const {
    Q_ASSERT(checkIndex(index, CheckIndexOption::IndexIsValid));

    auto window = qobject_cast<SimpleListManagementWindow *>(this->parent());
    if (index.isValid() && role == Qt::StatusTipRole && index.column() == window->displayColumn) {
        auto isBuiltin = QIdentityProxyModel::index(index.row(), window->builtinColumn).data().toBool();
        auto id = QIdentityProxyModel::index(index.row(), window->idColumn).data();

        auto rawData = QIdentityProxyModel::index(index.row(), window->displayColumn).data();
        auto value = window->originTranslator->displayText(rawData, QLocale::system());
        return window->translatedItemDescription(value, isBuiltin);
    }

    return QIdentityProxyModel::data(index, role);
}

SimpleListManagementWindow::SimpleListManagementWindow() {
    setAttribute(Qt::WA_DeleteOnClose);
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
    auto isBuiltin = this->model->index(rootIndex.row(), builtinColumn).data().toBool();
    auto id = this->model->index(rootIndex.row(), idColumn).data();
    this->removeAction->setEnabled(!isBuiltin && !isUsed(id));
}

void SimpleListManagementWindow::initializeLayout() {
    auto *toolbar = addToolBar(i18n("Manage"));
    // toolbar->setOrientation(Qt::Vertical);
    toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    auto addAction = new QAction(toolbar);
    addAction->setText(i18n("Add"));
    addAction->setToolTip(i18n("Add a new row to the table"));
    addAction->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
    toolbar->addAction(addAction);
    connect(addAction, &QAction::triggered, this, &SimpleListManagementWindow::addItem);

    removeAction = new QAction(toolbar);
    removeAction->setText(i18n("Delete"));
    removeAction->setToolTip(i18n("Remove the selected row from the table"));
    removeAction->setIcon(QIcon::fromTheme(QStringLiteral("edit-delete")));
    removeAction->setEnabled(false);
    toolbar->addAction(removeAction);
    connect(removeAction, &QAction::triggered, this, &SimpleListManagementWindow::removeItem);

    auto repairAction = new QAction(toolbar);
    repairAction->setText(i18n("Clean up"));
    repairAction->setToolTip(i18n("Remove empty and duplicate rows."));
    repairAction->setIcon(QIcon::fromTheme(QStringLiteral("tools-wizard")));
    repairAction->setEnabled(true);
    toolbar->addAction(repairAction);
    connect(repairAction, &QAction::triggered, this, &SimpleListManagementWindow::repairItems);

    auto centralWidget = new QWidget();

    // We want to have a tooltip.
    auto tooltipModel = new StatusTooltipModel(this);
    tooltipModel->setSourceModel(model);

    // Show an icon for the built-in rows.
    auto builtinIconModel = new BuiltinModel(centralWidget);
    builtinIconModel->setSourceModel(tooltipModel);
    builtinIconModel->setColumns(builtinColumn, displayColumn);

    // Make only the ID column not editable.
    auto editableModel = new EditProxyModel(centralWidget);
    editableModel->setSourceModel(builtinIconModel);
    editableModel->addReadOnlyColumns({idColumn});

    // We want to filter and sort.
    auto filterProxyModel = new QSortFilterProxyModel(centralWidget);
    filterProxyModel->setSourceModel(editableModel);

    tableView = new QTableView(centralWidget);
    tableView->setMouseTracking(true);
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

    originTranslator = new BuiltinTextTranslatingDelegate(this);
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

    auto *searchBox = new QLineEdit(centralWidget);
    searchBox->setPlaceholderText(i18n("Search..."));
    searchBox->setClearButtonEnabled(true);
    connect(searchBox, &QLineEdit::textEdited, filterProxyModel, &QSortFilterProxyModel::setFilterFixedString);
    filterProxyModel->setFilterKeyColumn(displayColumn);
    filterProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    auto gridLayout = new QVBoxLayout(centralWidget);
    gridLayout->addWidget(searchBox, 0);
    gridLayout->addWidget(tableView, 1);

    setCentralWidget(centralWidget);

    auto tableCount = new QLabel;
    auto updater = [tableCount, this] {
        tableCount->setText(translatedItemCount(tableView->model()->rowCount()));
    };
    connect(tableView->model(), &QAbstractItemModel::rowsInserted, this, updater);
    connect(tableView->model(), &QAbstractItemModel::rowsRemoved, this, updater);
    connect(tableView->model(), &QAbstractItemModel::modelReset, this, updater);
    updater();
    statusBar()->addWidget(tableCount);
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

void SimpleListManagementWindow::removeReferencesFromModel(
    const QHash<QString, QVector<IntegerPrimaryKey> > &valueToIds,
    const QHash<IntegerPrimaryKey, QString> &idToValue, QSqlTableModel *foreignModel, int foreignKeyColumn) {
    for (int r = 0; r < foreignModel->rowCount(); ++r) {
        auto index = foreignModel->index(r, foreignKeyColumn);

        // If the model is empty, stop it now.
        if (!index.isValid() || index.data().isNull()) {
            if (!foreignModel->setData(index, QString())) {
                qWarning() << "Could not update data...";
                qWarning() << foreignModel->lastError();
            }
            continue;
        }

        auto originInModel = index.data().toLongLong();
        auto value = idToValue[originInModel];
        auto idsForThisValue = valueToIds[value];

        // If there are no duplicates, we do not need to update anything.
        if (idsForThisValue.length() == 1) {
            continue;
        }

        // Update the name to point to the first origin.
        if (!foreignModel->setData(index, idsForThisValue.first())) {
            qWarning() << "Could not update data...";
            qWarning() << foreignModel->lastError();
        }
    }
}

QString SimpleListManagementWindow::translatedItemCount(int itemCount) const {
    return i18np("%1 item", "%1 items", itemCount);
}

QString SimpleListManagementWindow::translatedItemDescription(const QString &item, bool isBuiltIn) const {
    if (isBuiltIn) {
        return i18n("Built-in item '%1'", item);
    }
    return i18n("Item '%1'", item);
}
