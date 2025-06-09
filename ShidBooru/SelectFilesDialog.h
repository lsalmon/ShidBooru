#ifndef SELECTFILESDIALOG_H
#define SELECTFILESDIALOG_H

#include <QDialog>
#include <QFileDialog>
#include <QAbstractButton>
#include <QDebug>
#include <QMessageBox>

namespace Ui {
class SelectFilesDialog;
}

class SelectFilesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SelectFilesDialog(QWidget *parent = nullptr);
    ~SelectFilesDialog();
    QDir selectedDir;

private slots:
    void SelectFiles(bool checked);

private:
    Ui::SelectFilesDialog *ui;
};

#endif // SELECTFILESDIALOG_H
