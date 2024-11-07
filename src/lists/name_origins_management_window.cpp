#include "name_origins_management_window.h"

#include <KLocalizedString>
#include <QMessageBox>
#include <QProgressDialog>
#include <QSqlQuery>

#include "data/data_manager.h"
#include "data/names.h"
#include "utils/builtin_text_translating_delegate.h"
#include "utils/model_utils_find_source_model_of_type.h"

NameOriginsManagementWindow::NameOriginsManagementWindow() {
    setWindowTitle(i18n("Manage name origins"));

    setModel(DataManager::get().nameOriginsModel());
    setColumns(NamesTableModel::ID, NameOriginTableModel::ORIGIN, NameOriginTableModel::BUILTIN);
    setTranslator(NameOrigins::toDisplayString);

    initializeLayout();
}

bool NameOriginsManagementWindow::repairConfirmation() {
    QMessageBox messageBox(this);
    messageBox.setIcon(QMessageBox::Question);
    messageBox.setText(i18n("Clean up name origins?"));
    messageBox.setInformativeText(
        i18n("This will merge duplicate and remove empty name origins. Do you want to continue?")
    );
    messageBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    messageBox.setDefaultButton(QMessageBox::Ok);

    return messageBox.exec() == QMessageBox::Ok;
}

void NameOriginsManagementWindow::removeMarkedReferences(
    const QHash<QString, QVector<IntegerPrimaryKey>> &valueToIds,
    const QHash<IntegerPrimaryKey, QString> &idToValue
) {
    auto nameModel = DataManager::get().namesModel();
    removeReferencesFromModel(valueToIds, idToValue, nameModel, NamesTableModel::ORIGIN_ID);
}

bool NameOriginsManagementWindow::isUsed(const QVariant &id) {
    auto *nameModel = DataManager::get().namesModel();
    auto usage =
        nameModel->match(nameModel->index(0, NamesTableModel::ORIGIN_ID), Qt::DisplayRole, id);
    return !usage.isEmpty();
}

QString NameOriginsManagementWindow::translatedItemCount(int itemCount) const {
    return i18np("%1 name origin", "%1 name origins", itemCount);
}

QString
NameOriginsManagementWindow::translatedItemDescription(const QString &item, bool isBuiltIn) const {
    if (isBuiltIn) {
        return i18n("Built-in name origin '%1'", item);
    }
    return i18n("Name origin '%1'", item);
}
