#ifndef PERSONDETAILVIEW_H
#define PERSONDETAILVIEW_H

#include <QFrame>
#include <QSqlRecord>

namespace Ui {
    class PersonDetailView;
}

class PersonDetailView : public QFrame {
Q_OBJECT

public:
    const long long id;
    explicit PersonDetailView(long long id, QWidget *parent = nullptr);
    void populate();

    ~PersonDetailView() override;

Q_SIGNALS:
    void personNameChanged(int personId, const QString& newName);


private:
    Ui::PersonDetailView *ui;
    QSqlRecord record;
};

#endif // PERSONDETAILVIEW_H
