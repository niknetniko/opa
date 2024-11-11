/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include <QDialog>


class QItemSelection;
class QDialogButtonBox;
class KRearrangeColumnsProxyModel;
class QLabel;
class QGroupBox;
class QBoxLayout;
class QTableView;
class QAbstractItemModel;

class ChooseExistingReferenceWindow : public QDialog {
    Q_OBJECT

public Q_SLOTS:
    void accept() override;

protected Q_SLOTS:
    void itemSelected(const QModelIndex& selected);
    void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected) const;

protected:
    /**
     * Create a dialog option with a table allowing the user to select a row.
     *
     * Note that some stuff such as window title, help text, etc. are left unspecified.
     * It is up to the child class to set those.
     *
     * @param searchColumn
     * @param resultColumn
     * @param sourceModel
     * @param parent
     */
    explicit ChooseExistingReferenceWindow(
        int searchColumn, int resultColumn, QAbstractItemModel* sourceModel, QWidget* parent
    );

    QTableView* tableView;
    QBoxLayout* layout;
    QVariant selected;
    QGroupBox* tableBox;
    QLabel* tableHelpText;

    KRearrangeColumnsProxyModel* displayModel;

private:
    int searchColumn;
    int resultColumn;

    QAbstractItemModel* sourceModel;
    QDialogButtonBox* buttonBox;
};
