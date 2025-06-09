#include "SelectFilesDialog.h"
#include "ui_selectfilesdialog.h"

SelectFilesDialog::SelectFilesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SelectFilesDialog)
{
    ui->setupUi(this);
    ui->currentPath->setText(QDir().absolutePath());
    selectedDir = QDir(QDir().absolutePath());
    connect(ui->folderExplorer, &QPushButton::clicked, this, &SelectFilesDialog::SelectFiles);
}

SelectFilesDialog::~SelectFilesDialog()
{
    delete ui;
}

void SelectFilesDialog::SelectFiles(bool checked)
{
    qDebug() << "Select files";
    QString dir = QFileDialog::getExistingDirectory();
    if(dir.isEmpty())
    {
        QMessageBox::information(this, "No Directory Selected", "Select directory again");
        return;
    }
    else
    {
        ui->currentPath->setText(dir);
    }

    selectedDir = QDir(dir);
    if(selectedDir.isEmpty())
    {
        QMessageBox::information(this, "Empty Directory", "No files will be available");
    }
}
