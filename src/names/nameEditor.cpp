//
// Created by niko on 8/02/24.
//

#include <QDialog>
#include <QQuickWidget>
#include <QSqlQuery>
#include <QDataWidgetMapper>
#include <QMessageBox>
#include <QSqlError>
#include <QCompleter>
#include <QString>
#include <QDialogButtonBox>
#include <QItemEditorFactory>
#include <QSqlRelationalDelegate>
#include <KLocalizedString>
#include "nameEditor.h"
#include "ui_nameEditor.h"
#include "names.h"
#include "data/names.h"
#include "data/data_manager.h"
#include "utils/model_utils.h"


NamesEditor::NamesEditor(QAbstractProxyModel *model, bool newRow, QWidget *parent) : QDialog(parent) {
    this->model = model;
    this->newRow = newRow;

    auto *form = new Ui::NameEditorForm();
    form->setupUi(this);

    // Connect the buttons.
    connect(form->dialogButtons, &QDialogButtonBox::accepted, this, &QDialog::accept); // NOLINT(*-unused-return-value)
    connect(form->dialogButtons, &QDialogButtonBox::rejected, this, &QDialog::reject); // NOLINT(*-unused-return-value)

    // Set up the name origin combobox.
    auto* originCompleterModel = new NameOriginTableModel(this);
    originCompleterModel->setSort(NameOriginTableModel::ORIGIN, Qt::SortOrder::AscendingOrder);
    originCompleterModel->select();
    form->origin->setModel(originCompleterModel);
    form->origin->setModelColumn(NameOriginTableModel::ORIGIN);
    form->origin->setItemDelegate(new QSqlRelationalDelegate(form->origin));

    // Set up autocomplete on the last name.
    // We want to sort this, so we need to create a new model.
    // Additionally, we want to use all last names, not just the once from the current person.
    auto* surnameCompleter = new QCompleter(DataManager::getInstance(this)->primaryNamesModel(this));
    surnameCompleter->setCaseSensitivity(Qt::CaseInsensitive);
//    surnameCompleter->setModelSorting(QCompleter::CaseSensitivelySortedModel);
    surnameCompleter->setCompletionColumn(NamesTableModel::SURNAME);
    surnameCompleter->setCompletionMode(QCompleter::PopupCompletion);
    form->surname->setCompleter(surnameCompleter);

//    auto* surnameAutocompleteModel = new NamesTableModel(this);
//    surnameAutocompleteModel->setSort(NamesTableModel::SURNAME, Qt::SortOrder::AscendingOrder);
//    surnameAutocompleteModel->select();
//    auto* surnameCompleter = new QCompleter(surnameAutocompleteModel, this);
//    surnameCompleter->setCaseSensitivity(Qt::CaseInsensitive);
//    surnameCompleter->setModelSorting(QCompleter::CaseSensitivelySortedModel);
//    surnameCompleter->setCompletionColumn(NamesTableModel::SURNAME);
//    surnameCompleter->setCompletionMode(QCompleter::PopupCompletion);
//    form->surname->setCompleter(surnameCompleter);

    // Set up autocomplete on the given names.
    // This is very basic: it will not tokenize the names.
//    auto* givenNamesAutocompleteModel = new NamesTableModel(this);
//    givenNamesAutocompleteModel->setSort(NamesTableModel::GIVEN_NAMES, Qt::SortOrder::AscendingOrder);
//    givenNamesAutocompleteModel->select();
//    auto* givenNameCompleter = new QCompleter(givenNamesAutocompleteModel, this);
//    givenNameCompleter->setCaseSensitivity(Qt::CaseInsensitive);
//    givenNameCompleter->setModelSorting(QCompleter::CaseSensitivelySortedModel);
//    givenNameCompleter->setCompletionColumn(NamesTableModel::GIVEN_NAMES);
//    givenNameCompleter->setCompletionMode(QCompleter::PopupCompletion);
//    form->givenNames->setCompleter(givenNameCompleter);

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
    // TODO: fix this somehow?
    if (this->mapper->submit()) {
        // We are done.
        QDialog::accept();
    } else {
        // Find the original model.
        auto* sqlModel = find_source_model_of_type<QSqlQueryModel>(this->model);
        assert(sqlModel != nullptr);
        auto lastError = sqlModel->lastError();
        auto errorText = lastError.text();
        qWarning() << "Error was:";
        qWarning() << errorText;
        QMessageBox::critical(this, i18n("Fout bij opslaan"), i18n("The changes could not be saved for some reason:\n") + errorText);

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
