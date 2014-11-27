#ifndef DIALOGS_H_
#define DIALOGS_H_

#include <QMessageBox>

void showWarning (const QString &title, const QString &text, QWidget *parent);

bool confirmProblem (QWidget *, const QString);
bool confirmProblem (QWidget *, const QString, const QString);
bool yesNoQuestion (QWidget *parent, QString title, QString question);
QMessageBox::StandardButton yesNoQuestionStandardButton (QWidget *parent, QString title, QString question);
QMessageBox::StandardButton yesNoCancelQuestion (QWidget *parent, QString title, QString question);

bool verifyPassword (QWidget *parent, const QString &password, const QString &message);


#endif
