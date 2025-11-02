#include "SelectFilesDialog.h"
#include "ui_SelectFilesDialog.h"

SelectFilesDialog::SelectFilesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SelectFilesDialog)
{
    ui->setupUi(this);
    ui->currentPath->setText(QDir().absolutePath());
    selected = QDir().absolutePath();
    connect(ui->fileSystemExplorer, &QPushButton::clicked, this, &SelectFilesDialog::SelectDirectory);
    connect(ui->comboBox, &QComboBox::currentTextChanged, this, &SelectFilesDialog::ComboChanged);
    connect(ui->currentPath, &QLineEdit::editingFinished, this, &SelectFilesDialog::UserManuallyAddedPath);
}

SelectFilesDialog::~SelectFilesDialog()
{
    delete ui;
}

void SelectFilesDialog::ComboChanged(const QString &text)
{
    qDebug() << "Combo selection" << text;
    if(text == "File")
    {
        disconnect(ui->fileSystemExplorer, &QPushButton::clicked, this, &SelectFilesDialog::SelectDirectory);
        connect(ui->fileSystemExplorer, &QPushButton::clicked, this, &SelectFilesDialog::SelectFile);
        ui->fileSystemExplorer->setText("Select file");
    }
    else if(text == "Directory")
    {
        disconnect(ui->fileSystemExplorer, &QPushButton::clicked, this, &SelectFilesDialog::SelectFile);
        connect(ui->fileSystemExplorer, &QPushButton::clicked, this, &SelectFilesDialog::SelectDirectory);
        ui->fileSystemExplorer->setText("Select folder");
    }
}

void SelectFilesDialog::SelectFile(bool checked)
{
    Q_UNUSED(checked);
    qDebug() << "Select file";
    QString file = QFileDialog::getOpenFileName(this, tr("Select a file"), QDir().absolutePath());
    if(file.isEmpty())
    {
        QMessageBox::information(this, "No File Selected", "Select file again");
        return;
    }
    else
    {
        ui->currentPath->setText(file);
    }

    selected = file;
}

void SelectFilesDialog::SelectDirectory(bool checked)
{
    Q_UNUSED(checked);
    qDebug() << "Select dir";
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select a directory"), QDir().absolutePath());
    if(dir.isEmpty())
    {
        QMessageBox::information(this, "No Directory Selected", "Select directory again");
        return;
    }
    else
    {
        ui->currentPath->setText(dir);
    }

    QDir selectedDir = QDir(dir);
    if(selectedDir.isEmpty())
    {
        QMessageBox::information(this, "Empty Directory", "No files will be available");
    }

    selected = dir;
}

// Avoid the enter key pressing the "OK" button
void SelectFilesDialog::keyPressEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return)
    {
        return;
    }
    QDialog::keyPressEvent(e);
}

void SelectFilesDialog::UserManuallyAddedPath()
{
    selected = ui->currentPath->text();
    QFileInfo checkPath(selected);
    if(checkPath.exists() && checkPath.isFile())
    {
        if(ui->fileSystemExplorer->text() == "Select folder")
        {
            qDebug() << "Combo changed to file";
            this->ComboChanged("File");
        }
    }
    else if(checkPath.exists() && checkPath.isDir())
    {
        if(ui->fileSystemExplorer->text() == "Select file")
        {
            qDebug() << "Combo changed to dir";
            this->ComboChanged("Directory");
        }
    }
    else
    {
        QMessageBox::warning(this, "Not A File/Dir Path", "Input path again");
        ui->currentPath->setText(QDir().absolutePath());
        selected = QDir().absolutePath();
    }
}