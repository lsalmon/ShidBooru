#include "MainMenu.h"
#include "ui_MainMenu.h"
#include <QDebug>

MainMenu::MainMenu(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainMenu)
{
    ui->setupUi(this);

    connect(ui->CreateBooruButton, &QPushButton::clicked, this, &MainMenu::onCreateBooruButtonClicked);
}

MainMenu::~MainMenu()
{
    delete ui;
}

void MainMenu::closeEvent(QCloseEvent *event)
{
    qDebug() << "Close event";
    //QMainWindow::closeEvent(event);
}

void MainMenu::onCreateBooruButtonClicked(bool checked)
{
    Q_UNUSED(checked);

    SelectFilesDialog dialbox(this);
    if(dialbox.exec() == QDialog::Accepted)
    {
        BooruMenu* newmenu = new BooruMenu(this, dialbox.selected);
        newmenu->setAttribute(Qt::WA_DeleteOnClose);
        newmenu->show();
    }
}
