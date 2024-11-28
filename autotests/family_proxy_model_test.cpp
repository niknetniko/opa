/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
// ReSharper disable CppMemberFunctionMayBeStatic
// ReSharper disable CppMemberFunctionMayBeConst
#include "./test_utils.h"
#include "data/data_manager.h"
#include "data/event.h"
#include "data/family.h"
#include "data/names.h"
#include "database/database.h"
#include "database/schema.h"
#include <sqlite3.h>

#include <QAbstractItemModelTester>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QTest>

/*
** This function is used to load the contents of a database file on disk
** into the "main" database of open database connection pInMemory, or
** to save the current contents of the database opened by pInMemory into
** a database file on disk. pInMemory is probably an in-memory database,
** but this function will also work fine if it is not.
**
** Parameter zFilename points to a nul-terminated string containing the
** name of the database file on disk to load from or save to. If parameter
** isSave is non-zero, then the contents of the file zFilename are
** overwritten with the contents of the database opened by pInMemory. If
** parameter isSave is zero, then the contents of the database opened by
** pInMemory are replaced by data loaded from the file zFilename.
**
** If the operation is successful, SQLITE_OK is returned. Otherwise, if
** an error occurs, an SQLite error code is returned.
*/
int loadOrSaveDb(sqlite3* pInMemory, const char* zFilename, int isSave) {
    /* Function return code */
    sqlite3* pFile; /* Database connection opened on zFilename */
    /* Backup object used to copy data */
    /* Database to copy to (pFile or pInMemory) */
    /* Database to copy from (pFile or pInMemory) */

    /* Open the database file identified by zFilename. Exit early if this fails
    ** for any reason. */
    int rc = sqlite3_open(zFilename, &pFile);
    if (rc == SQLITE_OK) {

        /* If this is a 'load' operation (isSave==0), then data is copied
        ** from the database file just opened to database pInMemory.
        ** Otherwise, if this is a 'save' operation (isSave==1), then data
        ** is copied from pInMemory to pFile.  Set the variables pFrom and
        ** pTo accordingly. */
        sqlite3* pFrom = isSave ? pInMemory : pFile;
        sqlite3* pTo = isSave ? pFile : pInMemory;

        /* Set up the backup procedure to copy from the "main" database of
        ** connection pFile to the main database of connection pInMemory.
        ** If something goes wrong, pBackup will be set to NULL and an error
        ** code and message left in connection pTo.
        **
        ** If the backup object is successfully created, call backup_step()
        ** to copy data from pFile to pInMemory. Then call backup_finish()
        ** to release resources associated with the pBackup object.  If an
        ** error occurred, then an error code and message will be left in
        ** connection pTo. If no error occurred, then the error code belonging
        ** to pTo is set to SQLITE_OK.
        */
        if (sqlite3_backup* pBackup = sqlite3_backup_init(pTo, "main", pFrom, "main")) {
            (void) sqlite3_backup_step(pBackup, -1);
            (void) sqlite3_backup_finish(pBackup);
        }
        rc = sqlite3_errcode(pTo);
    }

    /* Close the database connection opened on database file zFilename
    ** and return the result of this function. */
    (void) sqlite3_close(pFile);
    return rc;
}

using namespace Qt::Literals::StringLiterals;

class TestFamilyProxyModel : public QObject {
    Q_OBJECT

private:
    IntegerPrimaryKey addPerson(const QString& firstName, const QString& lastName, const QString& sex) {
        QSqlQuery query;
        auto personId = insertQuery(u"INSERT INTO people (root, sex) VALUES (true, '%1')"_s.arg(sex));
        VERIFY_OR_THROW2(
            query.exec(u"INSERT INTO names (person_id, sort, given_names, surname) VALUES (%1, 1, '%2', '%3')"_s
                           .arg(personId)
                           .arg(firstName, lastName)),
            query
        );
        return personId;
    }

    void addParentAndChild() {
        QSqlQuery query;

        // Query some data we need.
        auto marriageTypeId = selectQuery(u"SELECT id FROM event_types WHERE type = 'Marriage'"_s);
        auto primaryRoleId = selectQuery(u"SELECT id FROM event_roles WHERE role = 'Primary'"_s);
        auto partnerRoleId = selectQuery(u"SELECT id FROM event_roles WHERE role = 'Partner'"_s);
        auto birthTypeId = selectQuery(u"SELECT id FROM event_types WHERE type = 'Birth'"_s);
        auto motherRoleId = selectQuery(u"SELECT id FROM event_roles WHERE role = 'Mother'"_s);
        auto fatherRoleId = selectQuery(u"SELECT id FROM event_roles WHERE role = 'Father'"_s);

        // Add two people.
        auto fatherId = addPerson(u"Bob"_s, u"Bober"_s, u"Male"_s);
        auto motherId = addPerson(u"Alice"_s, u"English"_s, u"Female"_s);

        // Marry them.
        auto marriageEvent = insertQuery(u"INSERT INTO events (type_id) VALUES (%1)"_s.arg(marriageTypeId));
        QVERIFY(query.exec(u"INSERT INTO event_relations (event_id, person_id, role_id) VALUES (%1, %2, %3)"_s
                               .arg(marriageEvent)
                               .arg(fatherId)
                               .arg(primaryRoleId)));
        QVERIFY(query.exec(u"INSERT INTO event_relations (event_id, person_id, role_id) VALUES (%1, %2, %3)"_s
                               .arg(marriageEvent)
                               .arg(motherId)
                               .arg(partnerRoleId)));

        // Add a child.
        auto childId = addPerson(u"Child"_s, u"Bober"_s, u"Female"_s);
        auto birthEvent = insertQuery(u"INSERT INTO events (type_id) VALUES (%1)"_s.arg(birthTypeId));

        qDebug() << "Father has ID" << fatherId;

        // Link the child.
        QVERIFY(query.exec(u"INSERT INTO event_relations (event_id, person_id, role_id) VALUES (%1, %2, %3)"_s
                               .arg(birthEvent)
                               .arg(childId)
                               .arg(primaryRoleId)));
        QVERIFY(query.exec(u"INSERT INTO event_relations (event_id, person_id, role_id) VALUES (%1, %2, %3)"_s
                               .arg(birthEvent)
                               .arg(fatherId)
                               .arg(fatherRoleId)));
        QVERIFY(query.exec(u"INSERT INTO event_relations (event_id, person_id, role_id) VALUES (%1, %2, %3)"_s
                               .arg(birthEvent)
                               .arg(motherId)
                               .arg(motherRoleId)));
    }

private Q_SLOTS:
    void init() {
        // Without this, we cannot test anything.
        QVERIFY(QSqlDatabase::isDriverAvailable(u"QSQLITE"_s));

        // Open the database.
        open_database(u":memory:"_s);
        addParentAndChild();
        DataManager::initialize(this);
    }

    void cleanup() {
        DataManager::reset();
        auto db = QSqlDatabase::database();
        db.close();
    }

    void testCorrectDataInDefaultCase() {
        auto* model = DataManager::get().familyModelFor(this, 1);
        QCOMPARE(model->rowCount(), 1);

        auto firstParentIndex = model->index(0, FamilyProxyModel::PERSON_ID);
        QCOMPARE(firstParentIndex.data(), 2);

        // Only the first column has parents.
        qDebug() << "CHECK NON-ZERO COLUMN";
        QCOMPARE(model->rowCount(model->index(0, 3)), 0);
        qDebug() << "CHECK ZERO COLUMN";
        QCOMPARE(model->rowCount(model->index(0, 0)), 1);
    }

    void testWithModelTester() {
        auto* model = DataManager::get().familyModelFor(this, 1);
        new QAbstractItemModelTester(model, QAbstractItemModelTester::FailureReportingMode::QtTest);
    }
};

QTEST_MAIN(TestFamilyProxyModel)

#include "family_proxy_model_test.moc"
