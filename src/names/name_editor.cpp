#include "name_editor.h"

#include "data/data_manager.h"
#include "data/names.h"
#include "utils/formatted_identifier_delegate.h"
#include "utils/model_utils_find_source_model_of_type.h"
#include "utils/proxy_enabled_relational_delegate.h"

#include <KLocalizedString>
#include <QCompleter>
#include <QDialog>
#include <QMessageBox>
#include <QQuickWidget>
#include <QSqlError>
#include <QSqlQuery>
#include <QString>


NamesEditor::NamesEditor(QAbstractProxyModel* model, bool newRow, QWidget* parent) :
    QDialog(parent),
    model(model),
    newRow(newRow),
    form(new Ui::NameEditorForm()) {
    form->setupUi(this);

    // Connect the buttons.
    connect(form->dialogButtons, &QDialogButtonBox::accepted, this, &QDialog::accept); // NOLINT(*-unused-return-value)
    connect(form->dialogButtons, &QDialogButtonBox::rejected, this, &QDialog::reject); // NOLINT(*-unused-return-value)

    // Set up the name origin combobox.
    connectComboBox(model, NamesTableModel::ORIGIN, form->origin);

    // Get the ID of the current name, if not new.
    if (newRow) {
        this->setWindowTitle(i18n("Nieuwe naam toevoegen"));
    } else {
        auto nameId = format_id(FormattedIdentifierDelegate::NAME, model->index(0, 0).data());
        this->setWindowTitle(i18n("%1 bewerken", nameId));
    }

    auto* baseModel = DataManager::get().namesModel();

    // Set up autocomplete on the last name.
    // We want to sort this, so we need to create a new model.
    // Additionally, we want to use all last names, not just the once from the current person.
    auto* surnameCompleter = new QCompleter(baseModel);
    surnameCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    surnameCompleter->setCompletionColumn(NamesTableModel::SURNAME);
    surnameCompleter->setCompletionMode(QCompleter::PopupCompletion);
    form->surname->setCompleter(surnameCompleter);

    auto* givenNameCompleter = new QCompleter(baseModel);
    givenNameCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    givenNameCompleter->setCompletionColumn(NamesTableModel::GIVEN_NAMES);
    givenNameCompleter->setCompletionMode(QCompleter::PopupCompletion);
    form->givenNames->setCompleter(surnameCompleter);

    // Map the data from the database to the form.
    this->mapper = new QDataWidgetMapper(this);
    mapper->setModel(this->model);
    mapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
    mapper->addMapping(form->titles, NamesTableModel::TITLES);
    mapper->addMapping(form->givenNames, NamesTableModel::GIVEN_NAMES);
    mapper->addMapping(form->prefix, NamesTableModel::PREFIX);
    mapper->addMapping(form->surname, NamesTableModel::SURNAME);
    mapper->addMapping(form->origin, NamesTableModel::ORIGIN);
    mapper->setItemDelegate(new SuperSqlRelationalDelegate(this));
    mapper->toFirst();

    qDebug() << "Current index is now " << mapper->currentIndex();

    connect(mapper, &QDataWidgetMapper::currentIndexChanged, this, [](int index) {
        qDebug() << "DataMapper index changed to " << index;
    });
}

void NamesEditor::accept() {
    // Attempt to submit the mapper changes.
    qDebug() << "Current index is " << this->mapper->currentIndex();
    qDebug() << "Is the current index valid? " << this->model->index(this->mapper->currentIndex(), 0).isValid();
    if (this->mapper->submit()) {
        // We are done.
        QDialog::accept();
    } else {
        // Find the original model.
        const auto* sqlModel = findSourceModelOfType<QSqlQueryModel>(this->model);
        assert(sqlModel != nullptr);
        auto lastError = sqlModel->lastError();
        auto errorText = lastError.text();
        qWarning() << "Error was:" << errorText;
        qDebug() << "Raw error: " << lastError;
        QMessageBox::critical(
            this, i18n("Fout bij opslaan"), i18n("The changes could not be saved for some reason:\n") + errorText
        );

        qDebug() << "Native error code is " << lastError.nativeErrorCode();
        qDebug() << "Last query is " << sqlModel->query().lastQuery();
    }
}

void NamesEditor::reject() {
    this->model->revert();
    if (this->newRow) {
        qDebug() << "Removing cancelled addition...";
        if (!this->model->removeRow(this->model->rowCount() - 1)) {
            qWarning() << "Could not revert name...";
        }
    }
    QDialog::reject();
}

NamesEditor::~NamesEditor() {
    delete this->form;
}
