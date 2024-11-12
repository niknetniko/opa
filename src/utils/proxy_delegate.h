/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include <QStyledItemDelegate>

/**
 * A delegate that "delegates" to other Delegates depending on the column.
 *
 * Columns for which there is no mapping will be mapped to the default delegate instead.
 * This default is currently a QStyledItemDelegate.
 *
 * This idea is inspired by https://stackoverflow.com/a/10368966
 */
class ProxyDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    explicit ProxyDelegate(QObject* parent = nullptr);

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    [[nodiscard]] QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void destroyEditor(QWidget* editor, const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
    void
    updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    bool editorEvent(
        QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index
    ) override;

    void addMapping(int column, QAbstractItemDelegate* mapper);

private:
    QHash<int, QAbstractItemDelegate*> delegateMapping;

    [[nodiscard]] const QAbstractItemDelegate* getDelegateConst(const QModelIndex& index) const;
    QAbstractItemDelegate* getDelegate(const QModelIndex& index);
};
