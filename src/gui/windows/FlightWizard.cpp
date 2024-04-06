#include "FlightWizard.h"
#include "src/text.h"
#include "ui_FlightWizard.h"
#include <QStandardItemModel>
#include <QCompleter>
#include <QModelIndex>
#include <QGridLayout>
#include <QMessageBox>
#include "src/gui/SkCompleter.h"
#include "src/db/DbManager.h"
#include "src/db/cache/Cache.h"
#include "src/model/Person.h"
#include "src/model/FlightBase.h"
#include "src/config/Settings.h"

FlightWizard::FlightWizard(QWidget *parent, DbManager& dbManager) :
    QDialog(parent), ui(new Ui::FlightWizard), wizardFont(), dbManager(dbManager), cache(dbManager.getCache())
{
    ui->setupUi(this);
    wizardFont.setPointSize(14);

    init();
}

FlightWizard::~FlightWizard()
{
    delete ui;
}

void FlightWizard::init()
{
    ui->stack->setCurrentIndex(0);
    adaptButtons();

    init_page1();
    init_page2();
    init_page3();

    updateNextButtonState();
}

void FlightWizard::init_page1()
{
    QList<Plane> planeList = cache.getPlanesSortedByUsage();
    QList<QVariant> vPlaneList;
    foreach (Plane p, planeList)
        vPlaneList.append(QVariant::fromValue(p));

    SkCompleter* completer = new SkCompleter(ui->planeEdit, cache, vPlaneList, &planeMatches, &planeToStringWhenSelected);
    ui->planeEdit->setCompleter(completer);

    QList<Plane> planesByUsage = cache.getPlanesSortedByUsage();
    QList<DataButton*> dataButtons;
    QGridLayout* layout = new QGridLayout(ui->speedDial);
    ui->speedDial->setLayout(layout);
    for (int i = 0; i < 9; i++) {

        DataButton* l = new DataButton(ui->speedDial);
        dataButtons.append(l);

        if (planesByUsage.size() > i) {
            l->setData(QVariant::fromValue(planesByUsage[i]));
            l->setText(planesByUsage[i].toNiceString());
            connect(l, SIGNAL(clicked()), this, SLOT(planeButton_clicked()));
        } else {
            l->setData(QVariant());
            l->setText("(Leer)");
        }

        QFont f = l->font();
        f.setPointSize(14);
        l->setFont(f);
        l->setDefault(false);
        l->setAutoDefault(false);
        l->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));
        layout->addWidget(l, i/3, i%3);

        if (i > 1) {
            QWidget::setTabOrder(dataButtons[i-1], dataButtons[i]);
        } else {
            QWidget::setTabOrder(ui->planeEdit, dataButtons[0]);
        }
    }

    QWidget::setTabOrder(dataButtons[8], ui->cancelButton);
}

void FlightWizard::init_page2()
{
    QList<Flight::Type> typeList = Flight::listTypes(false);
    QGridLayout* layout = new QGridLayout(ui->speedDial2);
    ui->speedDial2->setLayout(layout);
    selectedType = Flight::typeNone;

    for (int i = 0; i < typeList.size(); i++) {
        DataButton* b = new DataButton(ui->speedDial2);
        b->setText(Flight::typeText(typeList[i]));
        b->setData(QVariant::fromValue((int) typeList[i]));
        b->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
        b->setFont(wizardFont);
        layout->addWidget(b, i/3, i%3);

        connect(b, SIGNAL(clicked()), this, SLOT(typeButton_clicked()));
    }

}

void FlightWizard::init_page3()
{
    // *** Pilot and copilot selector
    QList<Person> personList = cache.getPeopleSortedByFrequency();
    QList<QVariant> vPersonList;
    foreach (Person p, personList)
        vPersonList << QVariant::fromValue(p);

    SkCompleter* pilotCompleter = new SkCompleter(ui->pilotEdit, cache, vPersonList, &personMatches, &personToStringWhenSelected);
    SkCompleter* copilotCompleter = new SkCompleter(ui->copilotEdit, cache, vPersonList, &personMatches, &personToStringWhenSelected);

    ui->pilotEdit->setCompleter(pilotCompleter);
    ui->copilotEdit->setCompleter(copilotCompleter);

    // *** Launch methods
    QList<LaunchMethod> launchMethods = cache.getLaunchMethods().getList();
    ui->launchMethodComboBox->addItem(notr ("-"), invalidId);
    for (int i=0; i<launchMethods.size(); ++i)
        ui->launchMethodComboBox->addItem (launchMethods.at(i).nameWithShortcut(), launchMethods.at(i).getId());

    // If there is exactly one launch method, select it
    if (launchMethods.size() == 1)
        ui->launchMethodComboBox->setCurrentIndex(1);

    dbId preselectedLaunchMethod = Settings::instance().preselectedLaunchMethod;
    if (idValid (preselectedLaunchMethod))
        ui->launchMethodComboBox->setCurrentItemByItemData(preselectedLaunchMethod);
}

void FlightWizard::showing_page3() {
    ui->copilotLabel->setText(firstToUpper(Flight::typeCopilotDescription(selectedType)));

    QString flighttypeRemark = Flight::flighttypeRemark(selectedType);
    if (!flighttypeRemark.isEmpty()) {
        ui->chargeLabel->setText(flighttypeRemark);
        ui->chargeLabel->setStyleSheet(notr("background-color: blue; color:white;"));
        ui->chargeLabel->show();
    } else {
        ui->chargeLabel->hide();
    }

    ui->flighttypeLabel->setText(notr("<b>") + tr("Flight type: ") + notr("</b>") + Flight::typeText(selectedType, true));
}

void FlightWizard::accept()
{

    Flight flight = determineFlight();
    if (sender() == ui->acceptAndTakeoffButton) {
        flight.departNow(true);
    }
    bool success = dbManager.createObject(flight, this);

    if (!success)
    {
        QMessageBox::critical (
            this,
            tr ("Save error"),
            tr ("An error occured while writing the flight to the datbase")
            );
    }

    QDialog::accept();
}

Flight FlightWizard::determineFlight()
{
    Flight flight;
    if (!Settings::instance().anonymousMode) {
        flight.setPilotId(ui->pilotEdit->getSelectedItem().value<Person>().getId());
        if (selectedType == FlightBase::typeTraining1) {
            flight.setSupervisorId(ui->copilotEdit->getSelectedItem().value<Person>().getId());
        } else {
            flight.setCopilotId(ui->copilotEdit->getSelectedItem().value<Person>().getId());
        }
        flight.setNumCrew(1);
        flight.setNumPax(0);
        flight.setType(selectedType);
    } else {
        flight.setNumCrew(ui->numCrewInput->value());
        flight.setNumPax(ui->numPaxInput->value());
        flight.setType(Flight::Type::typeNormal);
    }
    flight.setPlaneId(ui->planeEdit->getSelectedItem().value<Plane>().getId());
    flight.setLaunchMethodId(ui->launchMethodComboBox->currentItemData().toInt());
    flight.setDepartureLocation(Settings::instance().location);
    flight.setLandingLocation(Settings::instance().location);
    return flight;
}

void FlightWizard::nextButton_clicked()
{
    int idx = ui->stack->currentIndex();
    if (idx == PlanePage) {
        ui->stack->setCurrentIndex(Settings::instance().anonymousMode ? AnonymousPage : TypePage);
    } else {
        ui->stack->setCurrentIndex(idx+1);
    }

    if (ui->stack->currentIndex() == PilotsPage) {
        showing_page3();
    }

    adaptButtons();
    adaptFocus();
    adaptVisibility();
    updateNextButtonState();
}

void FlightWizard::prevButton_clicked()
{
    int idx = ui->stack->currentIndex();
    if (idx == AnonymousPage) {
        ui->stack->setCurrentIndex(PlanePage);
    } else {
        ui->stack->setCurrentIndex(idx-1);
    }

    if (ui->stack->currentIndex() == PilotsPage) {
        showing_page3();
    }

    adaptButtons();
    adaptFocus();
    adaptVisibility();
    updateNextButtonState();
}

void FlightWizard::planeButton_clicked()
{
    DataButton* btn = (DataButton*) sender();
    ui->planeEdit->setSelectedItem(btn->getData());
    nextButton_clicked();
}

void FlightWizard::typeButton_clicked()
{
    DataButton* btn = (DataButton*) sender();
    selectedType = (Flight::Type) btn->getData().toInt();
    nextButton_clicked();
}

void FlightWizard::adaptFocus() {
    if (ui->stack->currentIndex() == AnonymousPage) {
        ui->numCrewInput->selectAll();
    }
}

void FlightWizard::adaptButtons()
{
    bool lastPage = ui->stack->currentIndex() == PilotsPage || ui->stack->currentIndex() == AnonymousPage;
    ui->prevButton->setVisible(ui->stack->currentIndex() != PlanePage);
    ui->nextButton->setVisible(!lastPage);
    ui->acceptButton->setVisible(lastPage);
    ui->acceptAndTakeoffButton->setVisible(lastPage);
    ui->nextButton->setDefault(!lastPage);
    ui->acceptButton->setDefault(lastPage);
}

void FlightWizard::adaptVisibility()
{
    ui->copilotLabel->setVisible(Flight::typeCopilotRecorded(selectedType) || Flight::typeSupervisorRecorded(selectedType));
    ui->copilotEdit->setVisible(Flight::typeCopilotRecorded(selectedType) || Flight::typeSupervisorRecorded(selectedType));
}

void FlightWizard::updateNextButtonState()
{
    int index = ui->stack->currentIndex();

    if (index == PlanePage)
    {
        ui->nextButton->setEnabled(ui->planeEdit->isItemSelected());
    }
    else if (index == PilotsPage)
    {
        bool e = ui->pilotEdit->isItemSelected() &&
                 (ui->copilotEdit->isItemSelected() || !(Flight::typeAlwaysHasCopilot(selectedType) && Flight::typeCopilotRecorded(selectedType))) &&
                idValid(ui->launchMethodComboBox->currentItemData().toInt());
        ui->nextButton->setEnabled(e);
        ui->acceptAndTakeoffButton->setEnabled(e);
        ui->acceptButton->setEnabled(e);
    }
    else if (index == TypePage)
    {
        ui->nextButton->setEnabled(selectedType != Flight::typeNone);
    }
}

std::optional<QString> FlightWizard::planeMatches(QVariant &v, QString completionPrefix) {
    Plane p = v.value<Plane>();
    foreach (QString token, completionPrefix.split(" ", Qt::SkipEmptyParts)) {
        if (p.registration.contains(token, Qt::CaseInsensitive) ||
            p.callsign.contains(token, Qt::CaseInsensitive) ||
            p.type.contains(token, Qt::CaseInsensitive)) {
            return planeToString(v);
        }
    }
    return {};
}

QString FlightWizard::planeToString(QVariant& v)
{
    Plane p = v.value<Plane>();
    return QString(notr("%1 %2 %3 %4"))
        .arg(p.callsign, -3)
        .arg(p.registration, -7)
        .arg(p.type, -20)
        .arg(p.club);
}

QString FlightWizard::planeToStringWhenSelected(QVariant& v)
{
    Plane p = v.value<Plane>();
    if (p.callsign.isEmpty()) {
        return QString(notr("%1, %2, %3")).arg(p.registration, p.type, p.club);
    } else {
        return QString(notr("%2 (%1), %3, %4")).arg(p.callsign, p.registration, p.type, p.club);
    }
}


std::optional<QString> FlightWizard::personMatches(QVariant &v, QString completionPrefix)
{
    Person p = v.value<Person>();

    foreach (QString token, completionPrefix.split(" ", Qt::SkipEmptyParts)) {
        if (p.firstName.contains(token, Qt::CaseInsensitive) ||
            p.lastName.contains(token, Qt::CaseInsensitive) ||
            p.nickname.contains(token, Qt::CaseInsensitive)) {

            QStringList nicks = p.nicknamesAsList();
            auto matchedNick = std::find_if(nicks.begin(), nicks.end(), [&](QString x) { return x.contains(token, Qt::CaseInsensitive);});

            if (matchedNick == nicks.end()) {
                return p.fullNameWithNick();
            } else {
                return QString(notr("%1 \"%3\" %2")).arg(p.firstName, p.lastName, *matchedNick);
            }
        }
    }
    return {};

}

QString FlightWizard::personToStringWhenSelected(QVariant &v)
{
    return v.value<Person>().fullNameWithNick();
}
