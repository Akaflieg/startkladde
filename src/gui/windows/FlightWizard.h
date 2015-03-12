#ifndef FLIGHTWIZARD_H
#define FLIGHTWIZARD_H

#include <QDialog>
#include <QPushButton>
#include <QVariant>

class QModelIndex;
class DbManager;
class Cache;

class DataButton : public QPushButton
{
private:
    QVariant data;
public:
    DataButton(QWidget* p) : QPushButton(p) { }
    void setData(QVariant d) { data = d; }
    QVariant getData() { return data; }
};

namespace Ui {
    class FlightWizard;
}

class FlightWizard : public QDialog
{
    Q_OBJECT

public:
    explicit FlightWizard(QWidget *parent, DbManager& dbManager);
    ~FlightWizard();

private:
    Ui::FlightWizard *ui;
    void init();
    void adaptButtons();


private slots:
    void prevButton_clicked();
    void nextButton_clicked();
    void planeButton_clicked();

private:
    DbManager& dbManager;
    Cache& cache;
};

#endif // FLIGHTWIZARD_H
