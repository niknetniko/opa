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
#include <QMessageBox>

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
            throw std::invalid_argument(QString("Unhandled case in switch statement %1").arg(origin).toStdString());
    }
}

NamesTableModel::NamesTableModel(IntegerPrimaryKey personId, QObject *parent) : QSqlTableModel(parent) {
    this->personId = personId;
    this->setTable(Schema::Names::TableName);
    if (this->personId != -1) {
        this->setFilter(QString("person_id = %1").arg(this->personId));
    }

    // Set the correct headers.
    this->setHeaderData(ID, Qt::Horizontal, i18n("ID"));
    this->setHeaderData(PERSON_ID, Qt::Horizontal, i18n("Person ID"));
    this->setHeaderData(MAIN, Qt::Horizontal, i18n("Main"));
    this->setHeaderData(TITLES, Qt::Horizontal, i18n("Titles"));
    this->setHeaderData(GIVEN_NAMES, Qt::Horizontal, i18n("Given names"));
    this->setHeaderData(PREFIX, Qt::Horizontal, i18n("Prefixes"));
    this->setHeaderData(SURNAME, Qt::Horizontal, i18n("Surname"));
    this->setHeaderData(ORIGIN, Qt::Horizontal, i18n("Origin"));
}

QVariant NamesTableModel::data(const QModelIndex &item, int role) const {
    auto value = QSqlTableModel::data(item, role);
    if (!value.isValid()) {
        return value;
    }

    // Modify the data itself.
    if (role == Qt::DisplayRole) {
        if (item.column() == ID) {
            // Format the ID.
            return format_name_id(value);
        } else if (item.column() == ORIGIN) {
            auto metaEnum = QMetaEnum::fromType<Names::Origin>();
            Names::Origin originEnum;
            if (value.toString().isEmpty()) {
                originEnum = Names::Origin::NONE;
            } else {
                auto asStdString = value.toString().toUpper().toStdString();
                originEnum = static_cast<Names::Origin>(metaEnum.keyToValue(asStdString.c_str()));
            }

            return Names::origin_to_display(originEnum);
        }
    } else if (role == Qt::EditRole) {
        if (item.column() == ORIGIN) {
            return value.toString();
        }
    }

    return value;
}

bool NamesTableModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (role == Qt::EditRole) {
        if (index.column() == ORIGIN) {
            // Handle this once we use enums everywhere.
//            qDebug() << "Setting value, type is " << value.typeName();
//            auto newValue = QVariant::fromValue(value).toString();
//            return QSqlTableModel::setData(index, newValue, role);
        }
    }

    return QSqlTableModel::setData(index, value, role);
}

NamesTableView::NamesTableView(IntegerPrimaryKey personId, QWidget *parent) : QWidget(parent) {
    this->personId = personId;
    this->baseModel = new NamesTableModel(personId, this);
    this->baseModel->select();
    this->baseModel->setEditStrategy(QSqlTableModel::OnManualSubmit);

    // We only show certain columns here.
    auto *selectedColumnsModel = new KRearrangeColumnsProxyModel(this);
    selectedColumnsModel->setSourceModel(baseModel);
    selectedColumnsModel->setSourceColumns({
                                                   NamesTableModel::ID,
                                                   NamesTableModel::MAIN,
                                                   NamesTableModel::TITLES,
                                                   NamesTableModel::GIVEN_NAMES,
                                                   NamesTableModel::PREFIX,
                                                   NamesTableModel::SURNAME,
                                                   NamesTableModel::ORIGIN
                                           });

    // We want to filter and sort.
    auto *filterProxyModel = new QSortFilterProxyModel(this);
    filterProxyModel->setSourceModel(selectedColumnsModel);

    // TODO: fix this and make it proper.
    // while (model->canFetchMore()) { model->fetchMore(); }

    tableView = new QTableView(this);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->verticalHeader()->setVisible(false);
    tableView->sortByColumn(1, Qt::SortOrder::AscendingOrder);
    tableView->horizontalHeader()->resizeSections(QHeaderView::Stretch);
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    tableView->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    tableView->setSortingEnabled(true);
    // We are done setting up, attach the model.
    tableView->setModel(filterProxyModel);

    // Wrap in a VBOX for layout reasons.
    auto *layout = new QVBoxLayout(this);
    layout->addWidget(tableView);

    connect(tableView->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this,
            &NamesTableView::handleSelectedNewRow);
}

void NamesTableView::handleSelectedNewRow(const QItemSelection &selected, const QItemSelection & /*deselected*/) {
    // Hack
    if (selected.empty()) {
        return;
    }

    auto *editorWindow = new NamesEditor(this->baseModel, selected.indexes().first().row(), this);
    editorWindow->show();
    editorWindow->adjustSize();
}

void NamesTableView::handleNewName() {
    auto newRecord = this->baseModel->record();
    newRecord.setGenerated(NamesTableModel::ID, true);
    newRecord.setValue(NamesTableModel::PERSON_ID, this->personId);
    newRecord.setValue(NamesTableModel::MAIN, false);
    if (!this->baseModel->insertRecord(-1, newRecord)) {
        QMessageBox::warning(this, "Could not insert name", "Problem inserting new name into database.");
        qWarning() << "Error was: " << this->baseModel->lastError();
        qDebug() << "Query was: " << this->baseModel->query().lastQuery();
        return;
    }

    auto index = this->baseModel->rowCount() - 1;

    // Show the dialog for the other data.
    auto *editorWindow = new NamesEditor(this->baseModel, index, this);
    editorWindow->show();
    editorWindow->adjustSize();
}
