//
// Created by niko on 8/02/24.
//

#include <QDialog>
#include <QQuickWidget>
#include <QSqlQuery>
#include <QMessageBox>
#include <QSqlError>
#include <QCompleter>
#include <QString>
#include <QDialogButtonBox>
#include <QItemEditorFactory>
#include <QSqlRelationalDelegate>
#include <KLocalizedString>

#include "name_editor.h"
#include "data/names.h"
#include "data/data_manager.h"
#include "utils/model_utils.h"
#include "utils/formatted_identifier_delegate.h"


NamesEditor::NamesEditor(QAbstractProxyModel *model, bool newRow, QWidget *parent) : QDialog(parent) {
    this->model = model;
    this->newRow = newRow;

    this->form = new Ui::NameEditorForm();
    form->setupUi(this);

    // Connect the buttons.
    connect(form->dialogButtons, &QDialogButtonBox::accepted, this, &QDialog::accept); // NOLINT(*-unused-return-value)
    connect(form->dialogButtons, &QDialogButtonBox::rejected, this, &QDialog::reject); // NOLINT(*-unused-return-value)

    // Set up the name origin combobox.
    auto *originCompleterModel = new NameOriginTableModel(this);
    originCompleterModel->setSort(NameOriginTableModel::ORIGIN, Qt::SortOrder::AscendingOrder);
    originCompleterModel->select();
    form->origin->setModel(originCompleterModel);
    form->origin->setModelColumn(NameOriginTableModel::ORIGIN);
    form->origin->setItemDelegate(new QSqlRelationalDelegate(form->origin));

    // Get the ID of the current name, if not new.
    if (newRow) {
        this->setWindowTitle(i18n("Nieuwe naam toevoegen"));
    } else {
        auto nameId = format_id(FormattedIdentifierDelegate::NAME,
                                originCompleterModel->data(originCompleterModel->index(0, 0)));
        this->setWindowTitle(i18n("%1 bewerken", nameId));
    }

    auto *baseModel = DataManager::getInstance(this)->namesModel();

    // Set up autocomplete on the last name.
    // We want to sort this, so we need to create a new model.
    // Additionally, we want to use all last names, not just the once from the current person.
    auto *surnameCompleter = new QCompleter(baseModel);
    surnameCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    surnameCompleter->setCompletionColumn(NamesTableModel::SURNAME);
    surnameCompleter->setCompletionMode(QCompleter::PopupCompletion);
    form->surname->setCompleter(surnameCompleter);

    auto *givenNameCompleter = new QCompleter(baseModel);
    givenNameCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    givenNameCompleter->setCompletionColumn(NamesTableModel::GIVEN_NAMES);
    givenNameCompleter->setCompletionMode(QCompleter::PopupCompletion);
    form->givenNames->setCompleter(surnameCompleter);

    auto isMain = this->model->data(this->model->index(0, NamesTableModel::MAIN)).toBool();
    if (isMain) {
        form->primaryName->setEnabled(false);
    }

    // Map the data from the database to the form.
    this->mapper = new QDataWidgetMapper(this);
    mapper->setModel(this->model);
    mapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
    mapper->addMapping(form->titles, NamesTableModel::TITLES);
    mapper->addMapping(form->givenNames, NamesTableModel::GIVEN_NAMES);
    mapper->addMapping(form->prefix, NamesTableModel::PREFIX);
    mapper->addMapping(form->surname, NamesTableModel::SURNAME);
    mapper->addMapping(form->origin, NamesTableModel::ORIGIN_ID);
    mapper->addMapping(form->primaryName, NamesTableModel::MAIN);
    mapper->toFirst();
}

void NamesEditor::accept() {
    // Attempt to submit the mapper changes.
    if (this->mapper->submit()) {
        // We are done.
        QDialog::accept();
    } else {
        // Find the original model.
        auto *sqlModel = find_source_model_of_type<QSqlQueryModel>(this->model);
        assert(sqlModel != nullptr);
        auto lastError = sqlModel->lastError();
        auto errorText = lastError.text();
        qWarning() << "Error was:";
        qWarning() << errorText;
        QMessageBox::critical(this, i18n("Fout bij opslaan"),
                              i18n("The changes could not be saved for some reason:\n") + errorText);

        qDebug() << "Native error code is " << lastError.nativeErrorCode();
        qDebug() << "Last query is " << sqlModel->query().lastQuery();
        return;
    }
}

void NamesEditor::reject() {
    this->model->revert();
    if (this->newRow) {
        qDebug() << "Removing cancelled addition...";
        this->model->removeRow(this->model->rowCount() - 1);
    }
    QDialog::reject();
}

NamesEditor::~NamesEditor() {
    delete this->form;
}
