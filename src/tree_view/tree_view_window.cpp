
/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "tree_view_window.h"

#include "data/family.h"
#include "person_tree_graph_model.h"
#include "utils/formatted_identifier_delegate.h"

#include <QAction>
#include <QToolBar>
#include <QtNodes/BasicGraphicsScene>
#include <QtNodes/GraphicsView>

TreeViewWindow::TreeViewWindow(IntegerPrimaryKey person, QWidget* parent) : QMainWindow(parent) {
    auto* graphModel = new PersonTreeGraphModel(person);
    auto rootIndex = graphModel->findByChildId(person).constFirst();
    auto* model = rootIndex.model();
    auto name = model->index(rootIndex.row(), AncestorDisplayModel::DISPLAY_NAME).data().toString();
    auto id = format_id(FormattedIdentifierDelegate::PERSON, person);
    setWindowTitle(QStringLiteral("Pedigree for %1 [%2]").arg(name, id));

    auto* scene = new QtNodes::BasicGraphicsScene(*graphModel);
    scene->setOrientation(Qt::Vertical);
    auto* view = new QtNodes::GraphicsView(scene);

    auto* toolbar = addToolBar(i18n("Navigate"));

    auto* zoomIn = new QAction(toolbar);
    zoomIn->setText(i18n("Zoom in"));
    zoomIn->setIcon(QIcon::fromTheme(QStringLiteral("zoom-in")));
    toolbar->addAction(zoomIn);
    connect(zoomIn, &QAction::triggered, view, &QtNodes::GraphicsView::scaleUp);

    auto* zoomOut = new QAction(toolbar);
    zoomOut->setText(i18n("Zoom out"));
    zoomOut->setIcon(QIcon::fromTheme(QStringLiteral("zoom-out")));
    toolbar->addAction(zoomOut);
    connect(zoomOut, &QAction::triggered, view, &QtNodes::GraphicsView::scaleDown);

    auto* center = new QAction(toolbar);
    center->setText(i18n("Center on root"));
    center->setIcon(QIcon::fromTheme(QStringLiteral("zoom-select")));
    toolbar->addAction(center);
    connect(center, &QAction::triggered, view, &QtNodes::GraphicsView::centerScene);

    setCentralWidget(view);
    toolbar->setMovable(false);

    view->setContextMenuPolicy(Qt::ActionsContextMenu);
    auto* openPersonAction = new QAction(view);
    openPersonAction->setText(i18n("Open person details"));
    connect(openPersonAction, &QAction::triggered, [&]() {
        // TODO: how can we get the current model here?
        // Mouse position in scene coordinates.
        // QPointF posView = view->mapToScene(view->mapFromGlobal(QCursor::pos()));
        // const NodeId newId = graphModel.addNode();
        // graphModel.setNodeData(newId, NodeRole::Position, posView);
    });
    // TODO: remove some existing actions
    view->addAction(openPersonAction);
}
