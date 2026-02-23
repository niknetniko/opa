/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "database/schema.h"
#include "utils/builtin_text_translating_delegate.h"

#include <QAbstractItemModel>
#include <QIdentityProxyModel>
#include <QMainWindow>
#include <QSqlTableModel>
#include <QTableView>
#include <QWidget>

class SimpleListManagementWindow;

class StatusTooltipModel : public QIdentityProxyModel {
    Q_OBJECT

public:
    explicit StatusTooltipModel(SimpleListManagementWindow* parent);

    [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;
};

class SimpleListManagementWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit SimpleListManagementWindow();

    friend class StatusTooltipModel;

public Q_SLOTS:
    /**
     * Adds an item to the underlying model and prepares it for editing.
     */
    virtual void addItem() const;

    /**
     * Remove an item from the underlying model.
     */
    virtual void removeItem() const;

    /**
     * Run the repair procedure on the items in the model.
     */
    virtual void repairItems();

    /**
     * Called when the selection is changed in the view.
     */
    void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

protected:
    void initializeLayout();

    void setColumns(int idColumn, int displayColumn, int builtinColumn);

    void setTranslator(const std::function<QString(QString)>& translator);

    void setModel(QAbstractItemModel* model);

    void setSqlModel(QSqlTableModel* model);

    static void removeReferencesFromModel(
        const QHash<QString, QVector<IntegerPrimaryKey>>& valueToIds,
        const QHash<IntegerPrimaryKey, QString>& idToValue,
        QSqlTableModel* foreignModel,
        int foreignKeyColumn
    );

    virtual bool repairConfirmation() = 0;

    virtual bool isUsed(const QVariant& id) = 0;

    virtual void removeMarkedReferences(
        const QHash<QString, QVector<IntegerPrimaryKey>>& valueToIds, const QHash<IntegerPrimaryKey, QString>& idToValue
    ) = 0;

    [[nodiscard]] virtual QString translatedItemCount(int itemCount) const;

    [[nodiscard]] virtual QString translatedItemDescription(const QString& item, bool isBuiltIn) const;

protected:
    QTableView* tableView = nullptr;

private:
    int idColumn = -1;
    int displayColumn = -1;
    int builtinColumn = -1;
    std::function<QString(QString)> translator;
    QAction* removeAction = nullptr;
    QAbstractItemModel* model = nullptr;
    QSqlTableModel* sqlModel = nullptr;
    BuiltinTextTranslatingDelegate* originTranslator = nullptr;
};
