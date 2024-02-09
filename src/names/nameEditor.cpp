//
// Created by niko on 8/02/24.
//

#include <QQuickWidget>
#include <QVBoxLayout>
#include <QDataWidgetMapper>
#include <QMessageBox>
#include <QSqlError>
#include "nameEditor.h"
#include "ui_nameEditor.h"
#include "names.h"


NamesEditor::NamesEditor(long long int personId, QWidget *parent) : QDialog(parent) {

    auto* form = new Ui::NameEditorForm();
    form->setupUi(this);
    connect(form->dialogButtons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(form->dialogButtons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    this->mapper = new QDataWidgetMapper(this);
    mapper->setModel(new SelectedDataNamesTableModel(personId, this));
    mapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
    // TODO
//    mapper->addMapping(1);
    mapper->addMapping(form->titles, 2);
    mapper->addMapping(form->givenNames, 3);
    mapper->addMapping(form->prefix, 4);
    mapper->addMapping(form->surname, 5);
    mapper->addMapping(form->originCombo, 6);
    mapper->toFirst();
}

void NamesEditor::accept() {
    if (this->mapper != nullptr) {
        auto submitted = mapper->submit();
        if (submitted) {
            QDialog::accept();
        } else {
            QMessageBox::critical(this, "Problem during saving", "The changes could not be saved for some reason");
            auto warn = static_cast<QSqlQueryModel>(mapper->model()).lastError();
            qDebug() << "Error was: " << warn.text().toStdString().c_str();
            return;
        }
    }
    QDialog::accept();
}
