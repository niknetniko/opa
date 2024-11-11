/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
// ReSharper disable CppMemberFunctionMayBeStatic
#include "utils/model_utils_find_source_model_of_type.h"

#include <QIdentityProxyModel>
#include <QStandardItemModel>
#include <QTest>

using namespace Qt::Literals::StringLiterals;

class ModelA : public QStandardItemModel {
    Q_OBJECT
};

class ModelB : public ModelA {
    Q_OBJECT
};

class ProxyModelA : public QIdentityProxyModel {
    Q_OBJECT
};

class ProxyModelB : public QIdentityProxyModel {
    Q_OBJECT
};

class TestFindSourceModelOfType : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void testOneLevel() const {
        ProxyModelA proxyModelA;
        ModelA modelA;

        proxyModelA.setSourceModel(&modelA);

        const auto* foundModel = findSourceModelOfType<ModelA>(&proxyModelA);
        QCOMPARE(foundModel, &modelA);
    }

    void testTwoLevels() const {
        ProxyModelA proxyModelA;
        ProxyModelB proxyModelB;
        ModelA modelA;

        proxyModelB.setSourceModel(&modelA);
        proxyModelA.setSourceModel(&proxyModelB);

        const auto* foundModel = findSourceModelOfType<ModelA>(&proxyModelA);
        QCOMPARE(foundModel, &modelA);
    }

    void testNotFound() const {
        ProxyModelA proxyModelA;
        ProxyModelB proxyModelB;
        ModelA modelA;

        proxyModelB.setSourceModel(&modelA);
        proxyModelA.setSourceModel(&proxyModelB);

        const auto* foundModel = findSourceModelOfType<ModelB>(&proxyModelA);
        QCOMPARE(foundModel, nullptr);
    }

    void testDirectCast() const {
        ModelA modelA;
        const auto* foundModel = findSourceModelOfType<ModelA>(&modelA);
        QCOMPARE(foundModel, &modelA);
    }

    void testInheritance() const {
        ProxyModelA proxyModelA;
        ModelB modelB;

        proxyModelA.setSourceModel(&modelB);

        // ModelB inherits from ModelA, so this should succeed
        const auto* foundModelA = findSourceModelOfType<ModelA>(&proxyModelA);
        QCOMPARE(foundModelA, &modelB);

        const auto* foundModelB = findSourceModelOfType<ModelB>(&proxyModelA);
        QCOMPARE(foundModelB, &modelB);
    }
};

QTEST_MAIN(TestFindSourceModelOfType)

#include "model_utils_find_source_model_of_type.moc"
