
/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "tree_view_window.h"

#include "person_tree_graph_model.h"

#include <QtNodes/BasicGraphicsScene>
#include <QtNodes/GraphicsView>

TreeViewWindow::TreeViewWindow(IntegerPrimaryKey person, QWidget* parent) : QWidget(parent) {
    auto* graphModel = new PersonTreeGraphModel(person);
    auto* scene = new QtNodes::BasicGraphicsScene(*graphModel);
    auto* view = new QtNodes::GraphicsView(scene);
    scene->setOrientation(Qt::Vertical);

    view->setWindowTitle(QStringLiteral("Simple Node Graph"));
    view->show();
}
