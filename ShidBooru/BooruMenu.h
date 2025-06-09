#ifndef BOORUMENU_H
#define BOORUMENU_H

#include <QFrame>
#include <QDir>
#include <QDebug>
#include <QListWidget>
#include <QStandardItemModel>
#include <QStringListModel>
#include "ItemEditorDelegate.h"

namespace Ui {
class BooruMenu;
}

class BooruMenu : public QFrame
{
    Q_OBJECT

public:
    explicit BooruMenu(QWidget *parent = nullptr, QDir _filesDir = QDir::current(), ItemEditorDelegate *_delegate = nullptr);
    ~BooruMenu();

private slots:
    void viewDoubleClickedItem(const QModelIndex& idx);

private:
    bool LoadFile(QFileInfo info);
    Ui::BooruMenu *ui;
    QDir filesDir;
    ItemEditorDelegate *delegate;
    QStandardItemModel model;
    QStringListModel tagModel;
};

#endif // BOORUMENU_H
