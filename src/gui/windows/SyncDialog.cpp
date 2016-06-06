#include "SyncDialog.h"
#include "ui_SyncDialog.h"
#include "src/i18n/notr.h"

SyncDialog::SyncDialog(QWidget *parent) :
    QDialog(parent),ui(new Ui::SyncDialog)
{
    ui->setupUi(this);
    this->setWindowFlags((this->windowFlags() | Qt::CustomizeWindowHint) & ~Qt::WindowCloseButtonHint);

    QStringList errorListHeaders;
    errorListHeaders << tr("Pilot") << tr("Copilot") << tr("Date") << tr("Departure") << tr("Landing") << tr("Error");
    ui->errorTreeWidget->setHeaderLabels(errorListHeaders);
    ui->errorTreeWidget->setVisible(false);
}

SyncDialog::~SyncDialog()
{
    delete ui;
}

void SyncDialog::completed(bool errors, QString msg, QList<QTreeWidgetItem*> errorItems)
{
    ui->closeButton->setEnabled(true);
    ui->cancelButton->setEnabled(false);
    ui->statusLabel->setText(msg);
    if (!errors)
    {
        ui->label->setText(tr("Upload successful."));
        ui->statusLabel->setStyleSheet(notr("color:green; font-weight:bold;"));
        ui->progressBar->setValue(ui->progressBar->maximum());
    } else
    {
        ui->label->setText(tr("An error occurred during upload."));
        ui->statusLabel->setStyleSheet(notr("color:red; font-weight:bold;"));
        ui->progressBar->setValue(0);
    }

    ui->errorTreeWidget->clear();
    ui->errorTreeWidget->setVisible(!errorItems.empty());
    ui->errorTreeWidget->addTopLevelItems(errorItems);
    for (int i = 0; i < 6; i++)
    {
        ui->errorTreeWidget->resizeColumnToContents(i);
    }
}

void SyncDialog::completed(bool errors, QString msg)
{
    completed(errors, msg, QList<QTreeWidgetItem*>());
}

void SyncDialog::setProgress(int val, QString msg)
{
    ui->statusLabel->setText(msg);
    ui->progressBar->setValue(val);
}

void SyncDialog::cancelButtonClicked()
{
    ui->cancelButton->setEnabled(false);
    emit cancelled();
}

void SyncDialog::setCancelable(bool cancelable)
{
    ui->cancelButton->setEnabled(cancelable);
}
