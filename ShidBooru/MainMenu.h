#ifndef MAINMENU_H
#define MAINMENU_H

#include <QMainWindow>
#include <QAbstractButton>
#include "SelectFilesDialog.h"
#include "BooruMenu.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainMenu; }
QT_END_NAMESPACE

class MainMenu : public QMainWindow
{
    Q_OBJECT

public:
    MainMenu(QWidget *parent = nullptr);
    ~MainMenu();

private slots:
    void onCreateBooruButtonClicked(bool checked = false);

private:
    Ui::MainMenu *ui;
    //BooruMenu *newmenu;
    void closeEvent(QCloseEvent *event);
};
#endif // MAINMENU_H
