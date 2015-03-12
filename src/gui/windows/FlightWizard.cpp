#include "FlightWizard.h"
#include "ui_FlightWizard.h"
#include <QStandardItemModel>
#include <QCompleter>
#include <QModelIndex>
#include <qdebug.h>
#include <QGridLayout>
#include "src/gui/PlaneCompleter.h"
#include "src/db/DbManager.h"
#include "src/db/cache/Cache.h"

FlightWizard::FlightWizard(QWidget *parent, DbManager& dbManager) :
    QDialog(parent), ui(new Ui::FlightWizard), dbManager(dbManager), cache(dbManager.getCache())
{
    ui->setupUi(this);

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

    PlaneCompleter* completer = new PlaneCompleter(this, cache);
    ui->planeEdit->setCompleter(completer);

    QList<Plane> planesByUsage = cache.getPlanesSortedByUsage();
    QGridLayout* layout = new QGridLayout(ui->speedDial);
    ui->speedDial->setLayout(layout);
    for (int i = 0; i < 9; i++) {
        DataButton* l = new DataButton(ui->speedDial);
        l->setData(QVariant::fromValue(planesByUsage[i]));
        l->setText(planesByUsage[i].toNiceString());
        l->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));
        QFont f = l->font();
        f.setPointSize(14);
        l->setFont(f);
        l->setDefault(false);
        l->setAutoDefault(false);
        layout->addWidget(l, i/3, i%3);
        connect(l, SIGNAL(clicked()), this, SLOT(planeButton_clicked()));
    }

}

void FlightWizard::nextButton_clicked()
{
    int idx = ui->stack->currentIndex();
    ui->stack->setCurrentIndex(idx+1);
    adaptButtons();
}

void FlightWizard::prevButton_clicked()
{
    int idx = ui->stack->currentIndex();
    ui->stack->setCurrentIndex(idx-1);
    adaptButtons();
}

void FlightWizard::planeButton_clicked()
{
    DataButton* btn = (DataButton*) sender();
    ui->planeEdit->setSelectedItem(btn->getData().value<Plane>());
}

void FlightWizard::adaptButtons() {
    int lastIndex = 1;

    ui->prevButton->setVisible(ui->stack->currentIndex() != 0);
    ui->nextButton->setVisible(ui->stack->currentIndex() != lastIndex);
    ui->acceptButton->setVisible(ui->stack->currentIndex() == lastIndex);
    ui->acceptAndTakeoffButton->setVisible(ui->stack->currentIndex() == lastIndex);
    ui->nextButton->setDefault(ui->stack->currentIndex() != lastIndex);
    ui->acceptButton->setDefault(ui->stack->currentIndex() == lastIndex);
}
