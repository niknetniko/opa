#include "event_roles_management_window.h"

#include "data/data_manager.h"
#include "data/event.h"
#include "utils/builtin_text_translating_delegate.h"
#include "utils/model_utils_find_source_model_of_type.h"

#include <KLocalizedString>
#include <QMessageBox>
#include <QProgressDialog>
#include <QSqlQuery>

EventRolesManagementWindow::EventRolesManagementWindow() {
    setWindowTitle(i18n("Manage event roles"));

    setModel(DataManager::get().eventRolesModel());
    setColumns(EventRolesModel::ID, EventRolesModel::ROLE, EventRolesModel::BUILTIN);
    setTranslator(EventRoles::toDisplayString);

    initializeLayout();
}

bool EventRolesManagementWindow::repairConfirmation() {
    QMessageBox messageBox(this);
    messageBox.setIcon(QMessageBox::Question);
    messageBox.setText(i18n("Clean up event roles?"));
    messageBox.setInformativeText(
        i18n("This will merge duplicate and remove empty event roles. Do you want to continue?")
    );
    messageBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    messageBox.setDefaultButton(QMessageBox::Ok);
    return messageBox.exec() == QMessageBox::Ok;
}

void EventRolesManagementWindow::removeMarkedReferences(
    const QHash<QString, QVector<IntegerPrimaryKey>>& valueToIds, const QHash<IntegerPrimaryKey, QString>& idToValue
) {
    auto* relationModel = DataManager::get().eventRelationsModel();

    removeReferencesFromModel(valueToIds, idToValue, relationModel, EventRelationsModel::ROLE_ID);
}

bool EventRolesManagementWindow::isUsed(const QVariant& id) {
    auto* relationModel = DataManager::get().eventRelationsModel();
    auto usage = relationModel->match(relationModel->index(0, EventRelationsModel::ROLE_ID), Qt::DisplayRole, id);
    return !usage.isEmpty();
}

QString EventRolesManagementWindow::translatedItemCount(int itemCount) const {
    return i18np("%1 event role", "%1 event roles", itemCount);
}

QString EventRolesManagementWindow::translatedItemDescription(const QString& item, bool isBuiltIn) const {
    if (isBuiltIn) {
        return i18n("Built-in event role '%1'", item);
    }
    return i18n("Event role '%1'", item);
}
