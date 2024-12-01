
/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "person_tree_graph_model.h"

#include "data/data_manager.h"
#include "data/family.h"
#include <utils/formatted_identifier_delegate.h>

#include <QSqlError>
#include <QSqlQuery>
#include <QtNodes/StyleCollection>

inline ConnectionId create(NodeId out, NodeId in) {
    return {
        .outNodeId = out,
        .outPortIndex = 0,
        .inNodeId = in,
        .inPortIndex = 0,
    };
}

PersonTreeGraphModel::PersonTreeGraphModel(IntegerPrimaryKey person) {
    this->sourceModel = DataManager::get().ancestorModelFor(this, person);

    connect(sourceModel, &QAbstractItemModel::modelReset, this, [this] {
        Q_EMIT this->modelReset();
        calculateNodePositions();
    });

    qDebug() << "Model for tree view contains..." << sourceModel->rowCount();

    calculateNodePositions();
}

QtNodes::NodeFlags PersonTreeGraphModel::nodeFlags(NodeId nodeId) const {
    Q_UNUSED(nodeId);
    return QtNodes::NoFlags;
}

std::unordered_set<NodeId> PersonTreeGraphModel::allNodeIds() const {
    std::unordered_set<NodeId> result;
    for (int row = 0; row < sourceModel->rowCount(); ++row) {
        result.insert(sourceModel->index(row, AncestorDisplayModel::CHILD_ID).data().toUInt());
    }
    return result;
}

std::unordered_set<ConnectionId> PersonTreeGraphModel::allConnectionIds(NodeId nodeId) const {
    std::unordered_set<ConnectionId> result;

    auto inNodes = connections(nodeId, PortType::In, 0);
    auto outNodes = connections(nodeId, PortType::Out, 0);
    inNodes.merge(outNodes);

    return inNodes;
}

std::unordered_set<ConnectionId>
PersonTreeGraphModel::connections(NodeId nodeId, PortType portType, PortIndex index) const {
    std::unordered_set<ConnectionId> result;
    Q_UNUSED(index);
    if (portType == PortType::In) {
        auto matches =
            sourceModel->match(sourceModel->index(0, AncestorDisplayModel::CHILD_ID), Qt::DisplayRole, nodeId, -1);
        for (auto matchedIndex: std::as_const(matches)) {
            auto fatherData = sourceModel->index(matchedIndex.row(), AncestorDisplayModel::FATHER_ID).data();
            if (!fatherData.isNull()) {
                result.insert(create(fatherData.toUInt(), nodeId));
            }
            auto motherData = sourceModel->index(matchedIndex.row(), AncestorDisplayModel::MOTHER_ID).data();
            if (!motherData.isNull()) {
                result.insert(create(motherData.toUInt(), nodeId));
            }
        }
    } else if (portType == PortType::Out) {
        auto fatherMatches =
            sourceModel->match(sourceModel->index(0, AncestorDisplayModel::FATHER_ID), Qt::DisplayRole, nodeId, -1);
        for (auto matchedIndex: std::as_const(fatherMatches)) {
            auto childData = sourceModel->index(matchedIndex.row(), AncestorDisplayModel::CHILD_ID).data();
            result.insert(create(nodeId, childData.toUInt()));
        }
        auto motherMatches =
            sourceModel->match(sourceModel->index(0, AncestorDisplayModel::MOTHER_ID), Qt::DisplayRole, nodeId, -1);
        for (auto matchedIndex: std::as_const(motherMatches)) {
            auto childData = sourceModel->index(matchedIndex.row(), AncestorDisplayModel::CHILD_ID).data();
            result.insert(create(nodeId, childData.toUInt()));
        }
    }

    return result;
}

bool PersonTreeGraphModel::connectionExists(const ConnectionId connectionId) const {
    auto matches = sourceModel->match(
        sourceModel->index(0, AncestorDisplayModel::CHILD_ID), Qt::DisplayRole, connectionId.inNodeId, -1
    );
    for (auto matchedIndex: std::as_const(matches)) {
        auto fatherData = sourceModel->index(matchedIndex.row(), AncestorDisplayModel::FATHER_ID).data();
        if (!fatherData.isNull() && fatherData.toUInt() == connectionId.outNodeId) {
            return true;
        }
        auto motherData = sourceModel->index(matchedIndex.row(), AncestorDisplayModel::MOTHER_ID).data();
        if (!motherData.isNull() && motherData.toUInt() == connectionId.outNodeId) {
            return true;
        }
    }
    return false;
}


bool PersonTreeGraphModel::connectionPossible(const ConnectionId connectionId) const {
    Q_UNUSED(connectionId);
    return false;
}

bool PersonTreeGraphModel::detachPossible(const ConnectionId connectionId) const {
    Q_UNUSED(connectionId);
    return false;
}

bool PersonTreeGraphModel::nodeExists(const NodeId nodeId) const {
    auto matches = findByChildId(nodeId);
    return !matches.isEmpty();
}

QVariant PersonTreeGraphModel::nodeData(NodeId nodeId, NodeRole role) const {
    switch (role) {
        case NodeRole::Type:
            return QStringLiteral("Person");
        case NodeRole::Position:
            return _nodeGeometryData[nodeId].pos;
        case NodeRole::Size:
            return _nodeGeometryData[nodeId].size;
        case NodeRole::CaptionVisible:
            return true;
        case NodeRole::Caption: {
            // TODO: show more data here.
            auto index = findByChildId(nodeId);
            auto name = sourceModel->index(index.first().row(), AncestorDisplayModel::DISPLAY_NAME).data().toString();
            auto id = format_id(FormattedIdentifierDelegate::PERSON, nodeId);
            return QStringLiteral("%1 (%2)").arg(name, id);
        }
        case NodeRole::Style:
            return QtNodes::StyleCollection::nodeStyle().toJson().toVariantMap();
        case NodeRole::InPortCount:
        case NodeRole::OutPortCount:
            return 1;
        case NodeRole::InternalData:
        case NodeRole::Widget:
        default:
            return {};
    }
}
bool PersonTreeGraphModel::setNodeData(NodeId nodeId, NodeRole role, QVariant value) {
    switch (role) {
        case NodeRole::Position: {
            _nodeGeometryData[nodeId].pos = value.value<QPointF>();
            Q_EMIT this->nodePositionUpdated(nodeId);
            return true;
        }
        case NodeRole::Size: {
            _nodeGeometryData[nodeId].size = value.value<QSize>();
            return true;
        }
        case NodeRole::CaptionVisible:
        case NodeRole::Caption:
        case NodeRole::Style:
        case NodeRole::InternalData:
        case NodeRole::InPortCount:
        case NodeRole::OutPortCount:
        case NodeRole::Widget:
        case NodeRole::Type:
        default:
            return false;
    }
}
QVariant PersonTreeGraphModel::portData(NodeId nodeId, PortType portType, PortIndex index, PortRole role) const {
    Q_UNUSED(index);
    Q_UNUSED(nodeId);
    switch (role) {
        case PortRole::Data:
        case PortRole::DataType:
        default:
            return {};
        case PortRole::ConnectionPolicyRole:
            return QVariant::fromValue(ConnectionPolicy::Many);
        case PortRole::CaptionVisible:
            return true;
        case PortRole::Caption: {
            if (portType == PortType::Out) {
                return QStringLiteral("Child");
            } else {
                return QStringLiteral("Parents");
            }
        }
    }
}

bool PersonTreeGraphModel::setPortData(
    NodeId nodeId, PortType portType, PortIndex index, const QVariant& value, PortRole role
) {
    Q_UNUSED(nodeId);
    Q_UNUSED(portType);
    Q_UNUSED(index);
    Q_UNUSED(value);
    Q_UNUSED(role);

    return false;
}

NodeId PersonTreeGraphModel::newNodeId() {
    return sourceModel->rowCount();
}

NodeId PersonTreeGraphModel::addNode(const QString nodeType) {
    Q_UNUSED(nodeType);
    return newNodeId();
}

void PersonTreeGraphModel::addConnection(const ConnectionId connectionId) {
    Q_UNUSED(connectionId);
}

bool PersonTreeGraphModel::deleteConnection(const ConnectionId connectionId) {
    Q_UNUSED(connectionId);
    return false;
}

bool PersonTreeGraphModel::deleteNode(const NodeId nodeId) {
    Q_UNUSED(nodeId);
    return false;
}

void PersonTreeGraphModel::calculateNodePositions() const {
    // TODO: use a proper algorithm for this.
    // TODO: do not do a double pass.
    QMap<int, QList<NodeId>> levelToNodes;
    for (int row = 0; row < sourceModel->rowCount(); ++row) {
        auto nodeId = sourceModel->index(row, AncestorDisplayModel::CHILD_ID).data().toUInt();
        auto level = sourceModel->index(row, AncestorDisplayModel::LEVEL).data().toInt();
        auto yValue = (level - 1) * 150;
        _nodeGeometryData[nodeId].pos.setY(-yValue);
        levelToNodes[level].append(nodeId);
    }

    for (int level = 1; level <= levelToNodes.count(); ++level) {
        auto nodes = levelToNodes[level];
        std::ranges::sort(nodes);
        for (int nodeIndex = 0; nodeIndex < nodes.count(); ++nodeIndex) {
            NodeId nodeId = nodes[nodeIndex];
            double dIndex = nodeIndex;
            int max = static_cast<int>(nodes.count()) - 1;
            double centeredIndex = dIndex - (max / 2) + (nodes.count() % 2 ? 0 : -0.5);
            double xValue = 300 * centeredIndex;
            _nodeGeometryData[nodeId].pos.setX(xValue);
        }
    }
}
QModelIndexList PersonTreeGraphModel::findByChildId(NodeId childId) const {
    // TODO: should we support multiple parents somehow?
    return sourceModel->match(sourceModel->index(0, AncestorDisplayModel::CHILD_ID), Qt::DisplayRole, childId);
}
