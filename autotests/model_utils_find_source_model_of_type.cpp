// ReSharper disable CppMemberFunctionMayBeStatic
#include <QTest>

#include "utils/model_utils_find_source_model_of_type.h"

#include <QStandardItemModel>

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
    void testOneLevel() {
        ProxyModelA proxyModelA;
        ModelA modelA;

        proxyModelA.setSourceModel(&modelA);

        auto foundModel = findSourceModelOfType<ModelA>(&proxyModelA);
        QCOMPARE(foundModel, &modelA);
    }

    void testTwoLevels() {
        ProxyModelA proxyModelA;
        ProxyModelB proxyModelB;
        ModelA modelA;

        proxyModelB.setSourceModel(&modelA);
        proxyModelA.setSourceModel(&proxyModelB);

        auto foundModel = findSourceModelOfType<ModelA>(&proxyModelA);
        QCOMPARE(foundModel, &modelA);
    }

    void testNotFound() {
        ProxyModelA proxyModelA;
        ProxyModelB proxyModelB;
        ModelA modelA;

        proxyModelB.setSourceModel(&modelA);
        proxyModelA.setSourceModel(&proxyModelB);

        auto foundModel = findSourceModelOfType<ModelB>(&proxyModelA);
        QCOMPARE(foundModel, nullptr);
    }

    void testDirectCast() {
        ModelA modelA;
        auto foundModel = findSourceModelOfType<ModelA>(&modelA);
        QCOMPARE(foundModel, &modelA);
    }

    void testInheritance() {
        ProxyModelA proxyModelA;
        ModelB modelB;

        proxyModelA.setSourceModel(&modelB);

        // ModelB inherits from ModelA, so this should succeed
        auto foundModelA = findSourceModelOfType<ModelA>(&proxyModelA);
        QCOMPARE(foundModelA, &modelB);

        auto foundModelB = findSourceModelOfType<ModelB>(&proxyModelA);
        QCOMPARE(foundModelB, &modelB);
    }
};

QTEST_MAIN(TestFindSourceModelOfType)

#include "model_utils_find_source_model_of_type.moc"
