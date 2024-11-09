// ReSharper disable CppMemberFunctionMayBeConst
#include "events/event_editor.h"

#include "data/data_manager.h"
#include "data/event.h"
#include "database/database.h"

#include <QDialogButtonBox>
#include <QLineEdit>
#include <QPushButton>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlTableModel>
#include <QTest>
#include <QTimer>

using namespace Qt::Literals::StringLiterals;

class EventEditorTest : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void init() {
        open_database(u":memory:"_s, false);

        QSqlQuery query;
        QVERIFY(query.exec(u"INSERT INTO event_types (id, type, builtin) VALUES (1, 'One', true), (2, 'Two', true)"_s));
        QVERIFY(
            query.exec(u"INSERT INTO event_roles (id, role, builtin) VALUES (1, 'Primary', true), (2, 'Other', true)"_s)
        );
        QVERIFY(query.exec(u"INSERT INTO people (root, sex) VALUES (true, 'male')"_s));

        DataManager::initialize(this);
    }

    void cleanup() {
        auto db = QSqlDatabase::database();
        db.close();
    }

    void testAcceptNewEvent() {
        auto* eventsModel = DataManager::get().eventsModel();
        auto* eventRelationsModel = DataManager::get().eventRelationsModel();

        // Second, insert a new event.
        auto newEventRecord = eventsModel->record();
        newEventRecord.setGenerated(EventsModel::ID, false);
        newEventRecord.setValue(EventsModel::TYPE_ID, 2);

        QVERIFY(eventsModel->insertRecord(-1, newEventRecord));

        auto newEventId = eventsModel->query().lastInsertId();
        QVERIFY(newEventId.isValid());

        auto eventRelationRecord = eventRelationsModel->record();
        eventRelationRecord.setValue(EventRelationsModel::EVENT_ID, newEventId);
        eventRelationRecord.setValue(EventRelationsModel::PERSON_ID, 1);
        eventRelationRecord.setValue(EventRelationsModel::ROLE_ID, 1);

        QVERIFY(eventRelationsModel->insertRecord(-1, eventRelationRecord));

        auto* singleEventModel = DataManager::get().singleEventModel(this, newEventId);
        auto* singleRelationModel = DataManager::get().singleEventRelationModel(this, newEventId, 1, 1);

        auto* editor = new EventEditor(singleRelationModel, singleEventModel, false, nullptr);
        editor->show();
        QVERIFY(QTest::qWaitForWindowFocused(editor));

        auto *roleComboBox = editor->findChild<QComboBox*>("eventRoleComboBox");
        roleComboBox->setFocus(Qt::MouseFocusReason);
        roleComboBox->setCurrentIndex(1);
        roleComboBox->clearFocus();

        auto* eventName = editor->findChild<QLineEdit*>("eventNameEdit");
        eventName->setFocus(Qt::MouseFocusReason);
        eventName->setText(u"The Event Name"_s);
        eventName->clearFocus();

        auto* typeComboBox = editor->findChild<QComboBox*>("eventTypeComboBox");
        typeComboBox->setFocus(Qt::MouseFocusReason);
        typeComboBox->setCurrentIndex(1);
        typeComboBox->clearFocus();
        // TODO: test date somehow.

        qDebug() << "SUBMIT!";
        auto* acceptButton = editor->findChild<QDialogButtonBox*>("dialogButtons")->button(QDialogButtonBox::Ok);
        QTest::mouseClick(acceptButton, Qt::LeftButton);

        QCOMPARE(eventsModel->rowCount(), 1);
        QCOMPARE(eventsModel->index(0, EventsModel::ID).data(), newEventId);
        QCOMPARE(eventsModel->index(0, EventsModel::TYPE).data(), u"Two"_s);
        QCOMPARE(eventsModel->index(0, EventsModel::NAME).data(), u"The Event Name"_s);

        QCOMPARE(eventRelationsModel->rowCount(), 1);
        QCOMPARE(eventRelationsModel->index(0, EventRelationsModel::PERSON_ID).data(), 1);
        QCOMPARE(eventRelationsModel->index(0, EventRelationsModel::ROLE_ID).data(), 2);
        QCOMPARE(eventRelationsModel->index(0, EventRelationsModel::EVENT_ID).data(), newEventId);
    }
};

// void EventEditorTest::testAccept() {
//     editor = new EventEditor(eventRelationsModel, eventsModel, true);
//     // Simulate user input.
//     editor->findChild<QComboBox*>("eventTypeComboBox")->setCurrentText("Workshop");
//     editor->findChild<QLineEdit*>("eventNameEdit")->setText("Qt Workshop");
//     editor->findChild<QDateEdit*>("eventDatePicker")->setDate(QDate::currentDate().addDays(1));
//
//     // Click the "accept" button.
//     auto* acceptButton = editor->findChild<QDialogButtonBox*>("dialogButtons")->button(QDialogButtonBox::AcceptRole);
//     QTest::mouseClick(acceptButton, Qt::LeftButton);
//
//     // Verify that the models have been updated.
//     QCOMPARE(eventsModel->rowCount(), 2);
//     QCOMPARE(eventsModel->data(eventsModel->index(1, EventsModel::TYPE)).toString(), QString("Workshop"));
//     QCOMPARE(eventsModel->data(eventsModel->index(1, EventsModel::NAME)).toString(), QString("Qt Workshop"));
//     QCOMPARE(eventRelationsModel->rowCount(), 2);
//     delete editor;
// }

// void EventEditorTest::testRejectNewEvent() {
//     editor = new EventEditor(eventRelationsModel, eventsModel, true);
//     QCOMPARE(eventsModel->rowCount(), 2); // One extra row for the new event.
//     QCOMPARE(eventRelationsModel->rowCount(), 2); // One extra row for the new event relation.
//
//     // Click the "reject" button.
//     auto* rejectButton = editor->findChild<QDialogButtonBox*>("dialogButtons")->button(QDialogButtonBox::RejectRole);
//     QTest::mouseClick(rejectButton, Qt::LeftButton);
//
//     // Verify that the extra rows have been removed.
//     QCOMPARE(eventsModel->rowCount(), 1);
//     QCOMPARE(eventRelationsModel->rowCount(), 1);
//     delete editor;
// }

// void EventEditorTest::testRejectExistingEvent() {
//     editor = new EventEditor(eventRelationsModel, eventsModel, false);
//     // Simulate user input (changing existing data).
//     editor->findChild<QComboBox*>("eventTypeComboBox")->setCurrentText("Workshop");
//
//     // Click the "reject" button.
//     auto* rejectButton = editor->findChild<QDialogButtonBox*>("dialogButtons")->button(QDialogButtonBox::RejectRole);
//     QTest::mouseClick(rejectButton, Qt::LeftButton);
//
//     // Verify that the model has reverted to its original state.
//     QCOMPARE(eventsModel->data(eventsModel->index(0, EventsModel::TYPE)).toString(), QString("Meeting"));
//     delete editor;
// }

QTEST_MAIN(EventEditorTest)

#include "event_editor.moc"
