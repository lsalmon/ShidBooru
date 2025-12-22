#include "MainMenu.h"
#include "ui_MainMenu.h"
#include <QDebug>

MainMenu::MainMenu(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainMenu)
{
    ui->setupUi(this);

    connect(ui->CreateBooruButton, &QPushButton::clicked, this, &MainMenu::onCreateBooruButtonClicked);
    connect(ui->LoadBooruButton, &QPushButton::clicked, this, &MainMenu::onLoadBooruButtonClicked);
}

MainMenu::~MainMenu()
{
    delete ui;
}

void MainMenu::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    qDebug() << "Close event";
    //QMainWindow::closeEvent(event);
}

void MainMenu::onCreateBooruButtonClicked(bool checked)
{
    Q_UNUSED(checked);

    SelectFilesDialog dialbox(this);
    if(dialbox.exec() == QDialog::Accepted)
    {
        BooruMenu* newmenu = new BooruMenu(this, dialbox.selected, CREATE);
        newmenu->setAttribute(Qt::WA_QuitOnClose);
        newmenu->show();
    }
}

void MainMenu::onLoadBooruButtonClicked(bool checked)
{
    Q_UNUSED(checked);

    // Use QStringList for compatibility with ctor
    // TODO : overload the ctor ?
    QStringList file_path = QStringList();
    file_path.prepend(QFileDialog::getOpenFileName(this, "Load database", QDir::homePath(), "SQLite Database (*.sqlite)"));
    if(!file_path[0].isEmpty() && !file_path[0].isNull())
    {
        BooruMenu* newmenu = new BooruMenu(this, file_path, LOAD);
        newmenu->setAttribute(Qt::WA_QuitOnClose);
        newmenu->show();
    }
    else
    {
        DisplayInfoMessage("Loading cancelled by user");
    }
}
