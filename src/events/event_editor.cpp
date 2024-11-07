#include "event_editor.h"

#include <QMessageBox>
#include <QSqlError>
#include <QSqlTableModel>
#include <data/data_manager.h>
#include <data/event.h>
#include <utils/formatted_identifier_delegate.h>
#include <utils/model_utils_find_source_model_of_type.h>
#include <utils/proxy_enabled_relational_delegate.h>

class CustomSqlRelationalModel;

EventEditor::EventEditor(
    QAbstractItemModel *eventRelationModel,
    QAbstractItemModel *eventModel,
    bool newEvent,
    QWidget *parent
) :
    QDialog(parent) {
    this->eventRelationModel = eventRelationModel;
    this->eventModel = eventModel;
    this->newEvent = newEvent;

    this->form = new Ui::EventEditorForm();
    form->setupUi(this);

    // Connect the buttons.
    connect(
        form->dialogButtons, &QDialogButtonBox::accepted, this, &QDialog::accept
    ); // NOLINT(*-unused-return-value)
    connect(
        form->dialogButtons, &QDialogButtonBox::rejected, this, &QDialog::reject
    ); // NOLINT(*-unused-return-value)

    connectComboBox(eventRelationModel, EventRelationsModel::ROLE, form->eventRoleComboBox);
    qDebug() << "Events model count:" << eventModel->columnCount();
    connectComboBox(eventModel, EventsModel::TYPE, form->eventTypeComboBox);

    if (newEvent) {
        this->setWindowTitle(i18n("Nieuwe gebeurtenis toevoegen"));
    } else {
        auto nameId = format_id(FormattedIdentifierDelegate::EVENT, eventModel->index(0, 0).data());
        this->setWindowTitle(i18n("%1 bewerken", nameId));
    }

    eventRelationMapper = new QDataWidgetMapper(this);
    eventRelationMapper->setModel(this->eventRelationModel);
    eventRelationMapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
    eventRelationMapper->addMapping(form->eventRoleComboBox, EventRelationsModel::ROLE);
    eventRelationMapper->setItemDelegate(new SuperSqlRelationalDelegate(this));
    eventRelationMapper->toFirst();

    eventMapper = new QDataWidgetMapper(this);
    eventMapper->setModel(this->eventModel);
    eventMapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
    eventMapper->addMapping(form->eventDatePicker, EventsModel::DATE);
    eventMapper->addMapping(form->eventTypeComboBox, EventsModel::TYPE);
    eventMapper->addMapping(form->eventNameEdit, EventsModel::NAME);
    eventMapper->setItemDelegate(new SuperSqlRelationalDelegate(this));
    eventMapper->toFirst();

    qDebug() << "Current event is now " << eventMapper->currentIndex();
    qDebug() << "Model has rows:" << eventModel->rowCount();

    connect(eventMapper, &QDataWidgetMapper::currentIndexChanged, this, [](int index) {
        qDebug() << "eventMapper index changed to " << index;
    });
    connect(eventRelationMapper, &QDataWidgetMapper::currentIndexChanged, this, [](int index) {
        qDebug() << "eventRelationMapper index changed to " << index;
    });
}

EventEditor::~EventEditor() {
    delete this->form;
}

void EventEditor::accept() {
    // Attempt to submit the mapper changes.
    qDebug() << "Current event index is " << this->eventMapper->currentIndex();
    qDebug() << "Is the current event index valid? "
             << this->eventModel->index(this->eventMapper->currentIndex(), 0).isValid();
    if (this->eventMapper->submit() && this->eventRelationMapper->submit()) {
        // We are done.
        QDialog::accept();
    } else {
        // Find the original model.
        auto eventSqlModel = findSourceModelOfType<QSqlQueryModel>(this->eventModel);
        assert(eventSqlModel != nullptr);
        auto relationSqlModel = findSourceModelOfType<QSqlQueryModel>(this->eventRelationModel);
        assert(relationSqlModel != nullptr);
        auto eventError = eventSqlModel->lastError();
        auto relationError = relationSqlModel->lastError();
        qWarning() << "Event error was:" << eventError.text();
        qWarning() << "Event relation error was:" << relationError.text();
        qDebug() << "Raw event error: " << eventError.text();
        qDebug() << "Raw event relation error: " << relationError.text();
        QMessageBox::critical(
            this,
            i18n("Fout bij opslaan"),
            i18n("The changes could not be saved for some reason:\n") + eventError.text() +
                relationError.text()
        );
    }
}

void EventEditor::reject() {
    this->eventRelationModel->revert();
    this->eventModel->revert();
    if (this->newEvent) {
        qDebug() << "Removing cancelled addition...";
        this->eventRelationModel->removeRow(this->eventRelationModel->rowCount() - 1);
        qDebug() << "Removing cancelled addition...";
        this->eventModel->removeRow(this->eventModel->rowCount() - 1);
    }
    QDialog::reject();
}
