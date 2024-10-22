//
// Created by niko on 22/10/24.
//

#ifndef SIMPLE_LIST_MANAGER_H
#define SIMPLE_LIST_MANAGER_H

#include <QSqlTableModel>
#include <QTableView>
#include <QWidget>

#include "database/schema.h"

class SimpleListManagementWindow : public QWidget {
    Q_OBJECT

public:
    explicit SimpleListManagementWindow(QWidget *parent);

public Q_SLOTS:
    void addItem() const;

    void removeItem() const;

    void repairItems();

    void onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

protected:
    void initializeLayout();

    void setColumns(int idColumn, int displayColumn, int builtinColumn);

    void setTranslator(const std::function<QString(QString)> &translator);

    void setModel(QSqlTableModel *model);

    static void removeReferencesFromModel(const QHash<QString, QVector<IntegerPrimaryKey>> &valueToIds,
                                          const QHash<IntegerPrimaryKey, QString> &idToValue, QSqlTableModel* foreignModel, int foreignKeyColumn);

    virtual bool repairConfirmation() = 0;

    virtual bool isUsed(const QVariant &id) = 0;

    virtual void removeMarkedReferences(const QHash<QString, QVector<IntegerPrimaryKey> > &valueToIds,
                                        const QHash<IntegerPrimaryKey, QString> &idToValue) = 0;

private:
    int idColumn = -1;
    int displayColumn = -1;
    int builtinColumn = -1;
    std::function<QString(QString)> translator;
    QAction *removeAction = nullptr;
    QTableView *tableView = nullptr;
    QSqlTableModel *model = nullptr;
};

#endif //SIMPLE_LIST_MANAGER_H
