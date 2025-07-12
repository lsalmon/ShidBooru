#ifndef SELECTFILESDIALOG_H
#define SELECTFILESDIALOG_H

#include <QDialog>
#include <QFileDialog>
#include <QAbstractButton>
#include <QDebug>
#include <QMessageBox>
#include <QComboBox>

namespace Ui {
class SelectFilesDialog;
}

class SelectFilesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SelectFilesDialog(QWidget *parent = nullptr);
    ~SelectFilesDialog();
    QString selected;

private slots:
    void SelectDirectory(bool checked);
    void SelectFile(bool checked);
    void ComboChanged(const QString &text);

private:
    Ui::SelectFilesDialog *ui;
};

#endif // SELECTFILESDIALOG_H
