/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "database/schema.h"
#include <qabstractitemmodel.h>

#include <QPointF>
#include <QSizeF>
#include <QtNodes/AbstractGraphModel>


class QAbstractItemModel;
using QtNodes::ConnectionId;
using QtNodes::ConnectionPolicy;
using QtNodes::NodeFlag;
using QtNodes::NodeId;
using QtNodes::NodeRole;
using QtNodes::PortIndex;
using QtNodes::PortRole;
using QtNodes::PortType;

struct NodeGeometryData {
    QSize size;
    QPointF pos;
};

class QSqlQueryModel;
/**
 * Maps a normal Qt model from the database to one that graph can use.
 */
class PersonTreeGraphModel : public QtNodes::AbstractGraphModel {
    Q_OBJECT

public:
    explicit PersonTreeGraphModel(IntegerPrimaryKey person);

    [[nodiscard]] QtNodes::NodeFlags nodeFlags(NodeId nodeId) const override;

    std::unordered_set<NodeId> allNodeIds() const override;
    std::unordered_set<ConnectionId> allConnectionIds(NodeId nodeId) const override;

    std::unordered_set<ConnectionId> connections(NodeId nodeId, PortType portType, PortIndex index) const override;

    bool connectionExists(ConnectionId connectionId) const override;
    bool connectionPossible(ConnectionId connectionId) const override;
    bool detachPossible(ConnectionId) const override;
    bool nodeExists(NodeId nodeId) const override;
    QVariant nodeData(NodeId nodeId, NodeRole role) const override;
    bool setNodeData(NodeId nodeId, NodeRole role, QVariant value) override;

    QVariant portData(NodeId nodeId, PortType portType, PortIndex index, PortRole role) const override;
    bool setPortData(NodeId nodeId, PortType portType, PortIndex index, const QVariant& value, PortRole role) override;

    // Unused.
    NodeId newNodeId() override;
    NodeId addNode(QString nodeType) override;
    void addConnection(ConnectionId connectionId) override;
    bool deleteConnection(ConnectionId connectionId) override;
    bool deleteNode(NodeId nodeId) override;

private:
    QAbstractItemModel* sourceModel;
    mutable std::unordered_map<NodeId, NodeGeometryData> _nodeGeometryData;

    void calculateNodePositions() const;

    QModelIndexList findByChildId(NodeId childId) const;
};
