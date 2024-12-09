
/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include "database/schema.h"

#include <QMainWindow>
#include <QWidget>


namespace QtNodes {
    class BasicGraphicsScene;
    class GraphicsView;
}
class QAbstractItemModel;

class TreeViewWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit TreeViewWindow(IntegerPrimaryKey person, QWidget* parent = nullptr);

    [[nodiscard]] QString generateConnectionStyle() const;
    [[nodiscard]] QString generateNodeStyle() const;
    [[nodiscard]] QString generateGraphicsViewStyle() const;

protected:
    void changeEvent(QEvent* event) override;

private:
    QtNodes::BasicGraphicsScene* scene;
    QtNodes::GraphicsView* graphicsView;
};
