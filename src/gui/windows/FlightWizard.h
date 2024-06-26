#ifndef FLIGHTWIZARD_H
#define FLIGHTWIZARD_H

#include <optional>
#include <QDebug>
#include <QDialog>
#include <QPushButton>
#include <QVariant>
#include "src/model/Flight.h"
#include "src/model/Plane.h"
#include "src/db/dbId.h"

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
    enum Page {
        PlanePage = 0, TypePage = 1, PilotsPage = 2, AnonymousPage = 3
    };

    explicit FlightWizard(QWidget *parent, DbManager& dbManager);
    ~FlightWizard();

    virtual void accept();

private:
    Ui::FlightWizard *ui;
    QFont wizardFont;
    void init();
    void init_page1();
    void init_page2();
    void init_page3();
    void showing_page3();
    void adaptButtons();
    void adaptVisibility();
    void adaptFocus();

    Flight determineFlight();

    Plane selectedPlane;
    Flight::Type selectedType;

    static std::optional<QString> planeMatches(QVariant &v, QString completionPrefix);
    static QString planeToString(QVariant &v);
    static QString planeToStringWhenSelected(QVariant &v);
    static std::optional<QString> personMatches(QVariant &v, QString completionPrefix);
    static QString personToStringWhenSelected(QVariant &v);



private slots:
    void prevButton_clicked();
    void nextButton_clicked();
    void planeButton_clicked();
    void typeButton_clicked();
    void updateNextButtonState();

private:
    DbManager& dbManager;
    Cache& cache;
};

#endif // FLIGHTWIZARD_H
