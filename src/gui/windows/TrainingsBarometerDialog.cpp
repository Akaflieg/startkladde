#include "TrainingsBarometerDialog.h"
#include "ui_TrainingsBarometerDialog.h"

TrainingsBarometerDialog::TrainingsBarometerDialog(QWidget *parent, QString name, int hours, int takeoffs) :
    QDialog(parent), barometer (hours, takeoffs, name), ui(new Ui::TrainingsBarometerDialog)
{
    ui->setupUi(this);
    setWindowTitle("Trainingsbarometer");

    QImage img = barometer.generate_barometer();

    ui->label->setPixmap(QPixmap::fromImage(img));
    resize(barometer.preferredSize());
}

TrainingsBarometerDialog::~TrainingsBarometerDialog()
{
    delete ui;
}
