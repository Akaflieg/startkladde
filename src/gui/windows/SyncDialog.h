#ifndef SYNCDIALOG_H
#define SYNCDIALOG_H

#include <QDialog>
#include <QTreeWidgetItem>

namespace Ui {
class SyncDialog;
}


class SyncDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit SyncDialog(QWidget *parent = 0);
    ~SyncDialog();

signals:
    void cancelled();

public slots:
    void completed(bool errors, QString msg, QList<QTreeWidgetItem*> errorItems);
    void completed(bool errors, QString msg);
    void setProgress(int val, QString msg);
    void cancelButtonClicked();
    void setCancelable(bool cancelable);

private:
    Ui::SyncDialog* ui;
};

#endif // SYNCDIALOG_H
