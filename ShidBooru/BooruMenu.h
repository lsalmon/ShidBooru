#ifndef BOORUMENU_H
#define BOORUMENU_H

#include <QFrame>
#include <QDir>
#include <QDebug>
#include <QAction>
#include <QMainWindow>
#include <QListWidget>
#include <QStandardItemModel>
#include <QStringListModel>
#include <QInputDialog>
#include <QMessageBox>
#include "BooruItemType.h"
#include "ItemEditor.h"
#include "TagFilterProxyModel.h"
#include "SearchTagDialog.h"
#include "SelectFilesDialog.h"
#include "ItemContextMenu.h"

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
    void findImage(void);
    void addImage(void);
    void removeImage(void);
    void searchImage(QString tags);
    void resetSearchImage(void);
    void searchImageFinished(bool res);

private:
    void BrowseFiles(QDir dir);
    bool LoadFile(QFileInfo info);
    bool eventFilter(QObject *obj, QEvent *event);
    ItemEditor* editor;
    Ui::BooruMenu *ui;
    QDir filesDir;
    QStandardItemModel model;
    TagFilterProxyModel *proxyModel;
    QStringListModel tagModel;
    bool searchInProgress = false;
};

#endif // BOORUMENU_H
