#include "MainMenu.h"
#include "BooruItemType.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    qRegisterMetaType<BooruTypeItem>("BooruTypeItem");
    QApplication a(argc, argv);
    MainMenu w;
    w.show();
    return a.exec();
}
