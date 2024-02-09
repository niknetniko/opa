//
// Created by niko on 8/02/24.
//

#include <KLocalizedString>
#include <QException>
#include <QSqlQuery>
#include <QDebug>
#include <QSqlError>
#include <stdexcept>
#include <QMetaEnum>
#include <QHeaderView>
#include <QVBoxLayout>

#include "names.h"
#include "database/schema.h"
#include "formatting.h"
#include "names/nameEditor.h"

QString Names::construct_display_name(const QString &titles, const QString &givenNames, const QString &prefix,
                                      const QString &surname) {
    QStringList nameParts;
    if (!titles.isEmpty()) {
        nameParts.append(titles);
    }
    if (!givenNames.isEmpty()) {
        nameParts.append(givenNames);
    }
    if (!prefix.isEmpty()) {
        nameParts.append(prefix);
    }
    if (!surname.isEmpty()) {
        nameParts.append(surname);
    }
    return nameParts.join(" ");
}

QString Names::origin_to_display(const Names::Origin &origin) {
    switch (origin) {
        case NONE:
            return "";
        case UNKNOWN:
            return i18n("Unknown");
        case PATRILINEAL:
            return i18n("Patrilineal");
        case MATRILINEAL:
            return i18n("Matrilineal");
        case GIVEN:
            return i18n("Given");
        case CHOSEN:
            return i18n("Chosen");
        case PATRONYMIC:
            return i18n("Patronymic");
        case MATRONYMIC:
            return i18n("Matronymic");
        case OCCUPATION:
            return i18n("Occupation");
        case LOCATION:
            return i18n("Location");
        default:
            throw std::invalid_argument("Unhandled case in switch statement");
    }
}

NamesTableModel::NamesTableModel(long long int personId, QObject *parent) : QSqlTableModel(parent) {
    this->personId = personId;
    this->regenerateQuery();
}

void NamesTableModel::regenerateQuery() {
    this->setTable("names");
    this->setFilter(QString("person_id = %1").arg(this->personId));
//    this->setEditStrategy(QSqlTableModel::OnRowChange);
    this->select();
}

QVariant NamesTableModel::data(const QModelIndex &item, int role) const {
    auto value = QSqlTableModel::data(item, role);
    if (!value.isValid()) {
        return value;
    }

    // Modify the data itself.
    if (role == Qt::DisplayRole) {
        if (record().fieldName(item.column()) == Schema::Names::id) {
            // Format the ID.
            return format_name_id(value);
        } else if (record().fieldName(item.column()) == Schema::Names::main) {
            if (value.toBool()) {
                return "✅";
            } else {
                return "";
            }
        } else if (record().fieldName(item.column()) == Schema::Names::origin) {
            auto metaEnum = QMetaEnum::fromType<Names::Origin>();
            const auto *asString = value.toString().toUpper().toStdString().c_str();
            auto originEnum = static_cast<Names::Origin>(metaEnum.keyToValue(asString));
            return Names::origin_to_display(originEnum);
        }
    }

    return value;
}

SortableAndFilterableModel::SortableAndFilterableModel(long long id, QObject *parent) : QSortFilterProxyModel(parent) {
    this->personId = id;
    this->setSourceModel(new SelectedDataNamesTableModel(id));
}

NamesTableView::NamesTableView(long long int personId, QWidget *parent) : QWidget(parent) {
    this->model = new SortableAndFilterableModel(personId, this);
    // TODO: fix this and make it proper.
//    while (model->canFetchMore()) { model->fetchMore(); }

    tableView = new QTableView(this);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->verticalHeader()->setVisible(false);
    tableView->sortByColumn(1, Qt::SortOrder::AscendingOrder);
    tableView->horizontalHeader()->resizeSections(QHeaderView::Stretch);
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    tableView->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    tableView->setSortingEnabled(true);
    // We are done setting up, attach the model.
    tableView->setModel(model);

    // Wrap in a VBOX for layout reasons.
    auto *layout = new QVBoxLayout(this);
    layout->addWidget(tableView);

    connect(tableView->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this,
            &NamesTableView::handleSelectedNewRow);
}

void NamesTableView::handleSelectedNewRow(const QItemSelection &selected, const QItemSelection &deselected) {
    // Hack
    if (selected.empty()) {
        return;
    }

    auto *editorWindow = new NamesEditor(this->model->personId, this);
    editorWindow->show();
    editorWindow->adjustSize();

    // TODO: open something?
}


SelectedDataNamesTableModel::SelectedDataNamesTableModel(long long int personId, QObject *parent): KRearrangeColumnsProxyModel(parent) {
    this->setSourceModel(new NamesTableModel(personId, parent));
    this->setSourceColumns({0, 9, 3, 2, 5, 6, 7});

    this->setHeaderData(0, Qt::Horizontal, i18n("ID"));
    this->setHeaderData(1, Qt::Horizontal, i18n("Main"));
    this->setHeaderData(2, Qt::Horizontal, i18n("Titles"));
    this->setHeaderData(3, Qt::Horizontal, i18n("Given names"));
    this->setHeaderData(4, Qt::Horizontal, i18n("Prefixes"));
    this->setHeaderData(5, Qt::Horizontal, i18n("Surname"));
    this->setHeaderData(6, Qt::Horizontal, i18n("Origin"));
}
