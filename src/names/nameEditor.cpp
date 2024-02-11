//
// Created by niko on 8/02/24.
//

#include <QQuickWidget>
#include <QSqlQuery>
#include <QDataWidgetMapper>
#include <QMessageBox>
#include <QSqlError>
#include "nameEditor.h"
#include "ui_nameEditor.h"
#include "names.h"


NamesEditor::NamesEditor(NamesTableModel *model, int selectedRow, QWidget *parent) : QDialog(parent) {
    this->model = model;

    auto *form = new Ui::NameEditorForm();
    form->setupUi(this);

    // Connect the buttons.
    connect(form->dialogButtons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(form->dialogButtons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    this->mapper = new QDataWidgetMapper(this);
    mapper->setModel(this->model);
    mapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
    // TODO
//    mapper->addMapping(1);
    mapper->addMapping(form->titles, NamesTableModel::TITLES);
    mapper->addMapping(form->givenNames, NamesTableModel::GIVEN_NAMES);
    mapper->addMapping(form->prefix, NamesTableModel::PREFIX);
    mapper->addMapping(form->surname, NamesTableModel::SURNAME);
    mapper->addMapping(form->originCombo, NamesTableModel::ORIGIN);
    mapper->setCurrentIndex(selectedRow);
}

void NamesEditor::accept() {
    // Attempt to submit the mapper changes.
    if (this->mapper->submit() && this->model->submitAll()) {
        // We are done.
        QDialog::accept();
    } else {
        QMessageBox::critical(this, "Problem during saving", "The changes could not be saved for some reason");
        qDebug() << "Error was: " << this->model->lastError();
        qDebug() << "Query was: " << this->model->query().lastQuery();
        return;
    }
}

void NamesEditor::reject() {
    this->model->revertAll();
    QDialog::reject();
}
