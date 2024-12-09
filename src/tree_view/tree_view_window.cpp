/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "tree_view_window.h"

#include "data/family.h"
#include "main/main_window.h"
#include "person_tree_graph_model.h"
#include "utils/formatted_identifier_delegate.h"

#include <QAction>
#include <QApplication>
#include <QToolBar>
#include <QtNodes/BasicGraphicsScene>
#include <QtNodes/GraphicsView>
#include <QtNodes/StyleCollection>

TreeViewWindow::TreeViewWindow(IntegerPrimaryKey person, QWidget* parent) : QMainWindow(parent) {
    auto* graphModel = new PersonTreeGraphModel(person);
    auto rootIndex = graphModel->findByChildId(person).constFirst();
    auto* model = rootIndex.model();
    auto name = model->index(rootIndex.row(), AncestorDisplayModel::DISPLAY_NAME).data().toString();
    auto id = format_id(FormattedIdentifierDelegate::PERSON, person);
    setWindowTitle(QStringLiteral("Pedigree for %1 [%2]").arg(name, id));

    QtNodes::ConnectionStyle::setConnectionStyle(generateConnectionStyle());
    QtNodes::NodeStyle::setNodeStyle(generateNodeStyle());
    QtNodes::GraphicsViewStyle::setStyle(generateGraphicsViewStyle());

    scene = new QtNodes::BasicGraphicsScene(*graphModel);
    scene->setOrientation(Qt::Vertical);
    graphicsView = new QtNodes::GraphicsView(scene);

    auto* toolbar = addToolBar(i18n("Navigate"));

    auto* zoomIn = new QAction(toolbar);
    zoomIn->setText(i18n("Zoom in"));
    zoomIn->setIcon(QIcon::fromTheme(QStringLiteral("zoom-in")));
    toolbar->addAction(zoomIn);
    connect(zoomIn, &QAction::triggered, graphicsView, &QtNodes::GraphicsView::scaleUp);

    auto* zoomOut = new QAction(toolbar);
    zoomOut->setText(i18n("Zoom out"));
    zoomOut->setIcon(QIcon::fromTheme(QStringLiteral("zoom-out")));
    toolbar->addAction(zoomOut);
    connect(zoomOut, &QAction::triggered, graphicsView, &QtNodes::GraphicsView::scaleDown);

    auto* center = new QAction(toolbar);
    center->setText(i18n("Center on root"));
    center->setIcon(QIcon::fromTheme(QStringLiteral("zoom-select")));
    toolbar->addAction(center);
    connect(center, &QAction::triggered, graphicsView, &QtNodes::GraphicsView::centerScene);

    setCentralWidget(graphicsView);
    toolbar->setMovable(false);

    connect(scene, &QtNodes::BasicGraphicsScene::nodeDoubleClicked, this, [](NodeId nodeId) {
        openOrSelectPerson(nodeId, true);
    });

    // // TODO: remove some existing actions
    // view->addAction(openPersonAction);
}

QString TreeViewWindow::generateConnectionStyle() const {
    auto palette = QApplication::palette();
    auto accent = palette.color(QPalette::Accent).name();

    return QStringLiteral(R"(
  {
    "ConnectionStyle": {
        "ConstructionColor": "gray",
        "NormalColor": "%1",
        "SelectedColor": "%1",
        "SelectedHaloColor": "%1",
        "HoveredColor": "%1",

        "LineWidth": 3.0,
        "ConstructionLineWidth": 2.0,
        "PointDiameter": 10.0,

        "UseDataDefinedColors": false
    }
  }
  )")
        .arg(accent);
}

QString TreeViewWindow::generateNodeStyle() const {
    auto palette = QApplication::palette();
    auto highlight = palette.color(QPalette::Highlight).name();
    auto base = palette.color(QPalette::Base).name();
    auto normalText = palette.color(QPalette::Text).name();
    auto disabledText = palette.color(QPalette::Disabled, QPalette::Text).name();
    auto shadow = palette.color(QPalette::Shadow).name();
    auto mid = palette.color(QPalette::Mid).name();

    return QStringLiteral(R"(
  {
    "NodeStyle": {
        "NormalBoundaryColor": "%6",
        "SelectedBoundaryColor": "%1",
        "GradientColor0": "%2",
        "GradientColor1": "%2",
        "GradientColor2": "%2",
        "GradientColor3": "%2",
        "ShadowColor": "%5",
        "FontColor" : "%3",
        "FontColorFaded" : "%4",
        "ConnectionPointColor": [169, 169, 169],
        "FilledConnectionPointColor": "%1",
        "ErrorColor": "red",
        "WarningColor": [128, 128, 0],

        "PenWidth": 1.0,
        "HoveredPenWidth": 1.5,

        "ConnectionPointDiameter": 8.0,

        "Opacity": 1
    }
  }
  )")
        .arg(highlight, base, normalText, disabledText, shadow, mid);
}

QString TreeViewWindow::generateGraphicsViewStyle() const {
    auto palette = QApplication::palette();
    auto window = palette.color(QPalette::Window).name();
    auto base = palette.color(QPalette::Base).name();

    return QStringLiteral(R"(
  {
    "GraphicsViewStyle": {
        "BackgroundColor": "%1",
        "FineGridColor": "transparent",
        "CoarseGridColor": "%2"
    }
  }
  )")
        .arg(window, base);
}

void TreeViewWindow::changeEvent(QEvent* event) {
    switch (event->type()) {
        case QEvent::PaletteChange:
        case QEvent::StyleChange: {
            QtNodes::ConnectionStyle::setConnectionStyle(generateConnectionStyle());
            QtNodes::NodeStyle::setNodeStyle(generateNodeStyle());
            QtNodes::GraphicsViewStyle::setStyle(generateGraphicsViewStyle());
            if (this->scene && this->graphicsView) {
                qDebug() << "Updating scene";
                scene->onModelReset();
                graphicsView->resetCachedContent();
                const auto& flowViewStyle = QtNodes::StyleCollection::flowViewStyle();
                graphicsView->setBackgroundBrush(flowViewStyle.BackgroundColor);
            }
            break;
        }
        default:
            break;
    }

    QMainWindow::changeEvent(event);
}
