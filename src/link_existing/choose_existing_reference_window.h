/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include <QDialog>


class QTableView;
class QAbstractItemModel;

class ChooseExistingReferenceWindow : public QDialog {
    Q_OBJECT

public:
    /**
     * Ask the user to select a row from the given table.
     *
     * The returned value is either invalid if the user declined to select anything, or the data read from the given
     * column. The window allows the user to search the table, optionally filtered on the given search column.
     *
     * @param searchColumn Optional column to limit search.
     * @param resultColumn Column to read results from.
     * @param sourceModel The model providing the data.
     * @param parent The parent requesting the item.
     * @return The selected item or invalid if nothing was chosen.
     */
    static QVariant
    selectItem(int resultColumn, QAbstractItemModel* sourceModel, QWidget* parent, int searchColumn = -1);

public Q_SLOTS:
    void accept() override;

protected Q_SLOTS:
    void itemSelected(const QModelIndex& selected);

private:
    explicit ChooseExistingReferenceWindow(
        int searchColumn, int resultColumn, QAbstractItemModel* sourceModel, QWidget* parent
    );

    int searchColumn;
    int resultColumn;
    QAbstractItemModel* sourceModel;
    QTableView* tableView;
    QVariant selected;
};
