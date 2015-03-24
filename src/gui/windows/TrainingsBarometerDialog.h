#ifndef TRAININGSBAROMETERDIALOG_H
#define TRAININGSBAROMETERDIALOG_H

#include <QDialog>
#include "src/gui/Trainingsbarometer.h"

namespace Ui {
    class TrainingsBarometerDialog;
}

class TrainingsBarometerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TrainingsBarometerDialog(QWidget *parent, QString name, int hours, int takeoffs);
    ~TrainingsBarometerDialog();

private:
    Trainingsbarometer barometer;
    Ui::TrainingsBarometerDialog *ui;
};

#endif // TRAININGSBAROMETERDIALOG_H
