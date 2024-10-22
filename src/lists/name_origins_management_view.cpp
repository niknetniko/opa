#include "name_origins_management_view.h"

#include <KLocalizedString>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QMessageBox>
#include <QProgressDialog>
#include <QSqlError>

#include "data/data_manager.h"
#include "data/names.h"
#include "utils/builtin_text_translating_delegate.h"
#include "utils/model_utils.h"

NameOriginsManagementWindow::NameOriginsManagementWindow(QWidget *parent) : SimpleListManagementWindow(parent) {
    this->setWindowTitle(i18n("Manage name origins"));

    setModel(DataManager::get().nameOriginsModel());
    setColumns(NamesTableModel::ID, NameOriginTableModel::ORIGIN, NameOriginTableModel::BUILTIN);
    setTranslator(NameOrigins::toDisplayString);

    initializeLayout();
}

QString NameOriginsManagementWindow::addItemDescription() {
    return i18n("Add name origin");
}

bool NameOriginsManagementWindow::repairConfirmation() {
    QMessageBox messageBox(this);
    messageBox.setIcon(QMessageBox::Question);
    messageBox.setText(i18n("Clean up name origins?"));
    messageBox.setInformativeText(
        i18n("This will merge duplicate and remove empty name origins. Do you want to continue?"));
    messageBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    messageBox.setDefaultButton(QMessageBox::Ok);

    return messageBox.exec() == QMessageBox::Ok;
}

QString NameOriginsManagementWindow::removeItemDescription() {
    return i18n("Remove name origin");
}

bool NameOriginsManagementWindow::removeMarkedReferences(const QHash<QString, QVector<IntegerPrimaryKey> > &valueToIds,
                                                         const QHash<IntegerPrimaryKey, QString> &idToValue) {
    auto nameModel = DataManager::get().namesModel();

    for (int r = 0; r < nameModel->rowCount(); ++r) {
        auto index = nameModel->index(r, NamesTableModel::ORIGIN_ID);

        // If the model is empty, stop it now.
        if (!index.isValid() || index.data().isNull()) {
            if (!nameModel->setData(index, QString())) {
                qWarning() << "Could not update data...";
                qWarning() << nameModel->lastError();
            }
            continue;
        }

        auto originInModel = index.data().toLongLong();
        auto value = idToValue[originInModel];
        auto idsForThisOrigin = valueToIds[value];

        // If there are no duplicates, we do not need to update anything.
        if (idsForThisOrigin.length() == 1) {
            continue;
        }

        // Update the name to point to the first origin.
        if (!nameModel->setData(index, idsForThisOrigin.first())) {
            qWarning() << "Could not update data...";
            qWarning() << nameModel->lastError();
        }
    }

    return true;
}

QString NameOriginsManagementWindow::repairItemDescription() {
    return i18n("Clean up name origins");
}

bool NameOriginsManagementWindow::isUsed(const QVariant &id) {
    auto *nameModel = DataManager::get().namesModel();
    auto usage = nameModel->match(nameModel->index(0, NamesTableModel::ORIGIN_ID), Qt::DisplayRole, id);
    return !usage.isEmpty();
}
