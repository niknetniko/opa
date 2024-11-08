#include <KLocalizedString>
#include <QDate>
#include <QSqlQuery>
#include <QToolBar>

#include "data/data_manager.h"
#include "data/event.h"
#include "data/person.h"
#include "database/schema.h"
#include "person_detail_view.h"
#include "person_event_tab.h"
#include "person_name_tab.h"
#include "ui_person_detail_view.h"
#include "utils/formatted_identifier_delegate.h"

PersonDetailView::PersonDetailView(IntegerPrimaryKey id, QWidget* parent) : QFrame(parent) {
    this->ui = new Ui::PersonDetailView();
    ui->setupUi(this);

    this->model = DataManager::get().personDetailsModel(this, id);
    this->populate();

    // Create tab for events
    auto* eventTab = new PersonEventTab(id, ui->tabWidget);
    ui->tabWidget->addTab(eventTab, i18n("Gebeurtenissen"));

    // Create tab for names
    auto* nameTab = new PersonNameTab(id, ui->tabWidget);
    ui->tabWidget->addTab(nameTab, i18n("Namen"));

    // Connect the model to this view, so we update when the data is changed.
    connect(this->model, &QAbstractProxyModel::dataChanged, this, &PersonDetailView::populate);
}

void PersonDetailView::populate() {
    assert(model->rowCount() == 1);

    auto rawPersonId = model->index(0, PersonDetailModel::ID).data();
    auto personId = format_id(FormattedIdentifierDelegate::PERSON, rawPersonId);
    ui->id->setText(personId);
    ui->displayName->setText(this->getDisplayName());

    auto sex = model->index(0, PersonDetailModel::SEX).data().toString();
    auto sexSymbol = Data::Person::Sex::toIcon(sex);
    auto sexDescription = Data::Person::Sex::toDisplay(sex);
    ui->sexIcon->setText(sexSymbol);
    ui->sexIcon->setToolTip(sexDescription);

    Q_EMIT this->dataChanged(rawPersonId.toLongLong());
}

PersonDetailView::~PersonDetailView() {
    delete ui;
}

bool PersonDetailView::hasId(IntegerPrimaryKey id) const {
    return model->index(0, PersonDetailModel::ID).data() == id;
}

QString PersonDetailView::getDisplayName() const {
    return model->index(0, PersonDetailModel::DISPLAY_NAME).data().toString();
}
