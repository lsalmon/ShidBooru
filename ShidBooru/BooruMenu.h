#ifndef BOORUMENU_H
#define BOORUMENU_H

#include <QFrame>
#include <QDir>
#include <QDebug>
#include <QMainWindow>
#include <QListWidget>
#include <QStandardItemModel>
#include <QStringListModel>
#include "BooruItemType.h"
#include "ItemEditor.h"

namespace Ui {
class BooruMenu;
}

class BooruMenu : public QMainWindow
{
    Q_OBJECT

public:
    explicit BooruMenu(QWidget *parent = nullptr, QDir _filesDir = QDir::current());
    ~BooruMenu();

private slots:
    void viewClickedItemTag(const QModelIndex& idx);
    void viewDoubleClickedItem(const QModelIndex& idx);
    void getUpdatedTagList(int state);

private:
    bool LoadFile(QFileInfo info);
    bool eventFilter(QObject *obj, QEvent *event);
    ItemEditor* editor;
    Ui::BooruMenu *ui;
    QDir filesDir;
    QStandardItemModel model;
    QStringListModel tagModel;
};

#endif // BOORUMENU_H
