#ifndef SYNCDIALOG_H
#define SYNCDIALOG_H

#include <QDialog>

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
    void completed(QString msg, bool errors);
    void setProgress(int val, QString msg);
    void cancelButtonClicked();
    void setCancelable(bool cancelable);

private:
    Ui::SyncDialog* ui;
};

#endif // SYNCDIALOG_H
