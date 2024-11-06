#include "model_utils_find_source_model_of_type.h"
#include "opa_date.h"

OpaDateModel::OpaDateModel(QObject *parent) : QIdentityProxyModel(parent) {
}

int OpaDateModel::dateColumn() const {
    return this->theDateColumn;
}

void OpaDateModel::setDateColumn(int column) {
    // Verify that the columns exist.
    assert(0 <= column);
    qDebug() << "Column count is" << this->columnCount();
    assert(column < this->columnCount());
    this->theDateColumn = column;
}

QVariant OpaDateModel::data(const QModelIndex &index, int role) const {
    auto rawData = QIdentityProxyModel::data(index, role);
    if (index.column() == this->theDateColumn && rawData.isValid() && !rawData.isNull()) {
        auto model = OpaDate::fromDatabaseRepresentation(rawData.toString());
        if (role == Qt::DisplayRole || role == Qt::EditRole) {
            return model.toDisplayText();
        } else if (role == ModelRole) {
            return QVariant::fromValue(model);
        }
    }
    return rawData;
}

bool OpaDateModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    QVariant savedValue = value;
    if (index.column() == this->theDateColumn && value.isValid() && !value.isNull()) {
        if (role == Qt::DisplayRole || role == Qt::EditRole) {
            savedValue = OpaDate::fromDisplayText(value.toString()).toDatabaseRepresentation();
        } else if (role == ModelRole) {
            savedValue = value.value<OpaDate>().toDatabaseRepresentation();
        }
    }

    return QIdentityProxyModel::setData(index, savedValue, role);
}
