/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "proxy_delegate.h"

ProxyDelegate::ProxyDelegate(QObject* parent) : QStyledItemDelegate(parent) {
}
void ProxyDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
    if (const auto* delegate = getDelegateConst(index)) {
        delegate->paint(painter, option, index);
    } else {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

QSize ProxyDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const {
    if (const auto* delegate = getDelegateConst(index)) {
        return delegate->sizeHint(option, index);
    } else {
        return QStyledItemDelegate::sizeHint(option, index);
    }
}

QWidget*
ProxyDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const {
    if (const auto* delegate = getDelegateConst(index)) {
        return delegate->createEditor(parent, option, index);
    } else {
        return QStyledItemDelegate::createEditor(parent, option, index);
    }
}

void ProxyDelegate::destroyEditor(QWidget* editor, const QModelIndex& index) const {
    if (const auto* delegate = getDelegateConst(index)) {
        delegate->destroyEditor(editor, index);
    } else {
        QStyledItemDelegate::destroyEditor(editor, index);
    }
}

void ProxyDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const {
    if (const auto* delegate = getDelegateConst(index)) {
        delegate->setEditorData(editor, index);
    } else {
        QStyledItemDelegate::setEditorData(editor, index);
    }
}

void ProxyDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const {
    QStyledItemDelegate::setModelData(editor, model, index);
}
void ProxyDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index)
    const {
    if (const auto* delegate = getDelegateConst(index)) {
        delegate->updateEditorGeometry(editor, option, index);
    } else {
        QStyledItemDelegate::updateEditorGeometry(editor, option, index);
    }
}

bool ProxyDelegate::editorEvent(
    QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index
) {
    if (auto* delegate = getDelegate(index)) {
        return delegate->editorEvent(event, model, option, index);
    } else {
        return QStyledItemDelegate::editorEvent(event, model, option, index);
    }
}

void ProxyDelegate::addMapping(int column, QAbstractItemDelegate* mapper) {
    delegateMapping[column] = mapper;
}

const QAbstractItemDelegate* ProxyDelegate::getDelegateConst(const QModelIndex& index) const {
    if (delegateMapping.contains(index.column())) {
        return delegateMapping[index.column()];
    }
    return nullptr;
}

QAbstractItemDelegate* ProxyDelegate::getDelegate(const QModelIndex& index) {
    return const_cast<QAbstractItemDelegate*>(getDelegateConst(index));
}
