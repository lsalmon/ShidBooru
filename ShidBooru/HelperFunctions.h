#ifndef HELPERFUNCTIONS_H
#define HELPERFUNCTIONS_H

#include <QMessageBox>

typedef enum BooruInitType {
    CREATE,
    LOAD
} BooruInitType;

static inline void DisplayMessage(QString message, QMessageBox::Icon icon) {
    QMessageBox msg;
    msg.setIcon(icon);
    msg.setText(message);
    msg.exec();
}
static inline void DisplayWarningMessage(QString message) {
    DisplayMessage(message, QMessageBox::Warning);
}
static inline void DisplayInfoMessage(QString message) {
    DisplayMessage(message, QMessageBox::Information);
}

#endif // HELPERFUNCTIONS_H
