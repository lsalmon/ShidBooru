#ifndef BOORUMENU_H
#define BOORUMENU_H

#include <QPointer>
#include <QFrame>
#include <QDir>
#include <QDebug>
#include <QAction>
#include <QMainWindow>
#include <QListWidget>
#include <QStandardItemModel>
#include <QStringListModel>
#include <QInputDialog>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QVector>
#include <QSet>
#include <QQueue>
#include <QtConcurrent>
#include <QThread>
#include "BooruItemType.h"
#include "ItemEditor.h"
#include "TagFilterProxyModel.h"
#include "SearchTagDialog.h"
#include "SelectFilesDialog.h"
#include "ItemContextMenu.h"
#include "HelperFunctions.h"

namespace Ui {
class BooruMenu;
}

class BooruMenu : public QMainWindow
{
    Q_OBJECT

public:
    explicit BooruMenu(QWidget *parent = nullptr, QString _file_or_db_path = "", BooruInitType type = CREATE);
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
    void exportToBooruFile(void);

private:
    void BooruMenuUISetup(void);
    void BrowseFiles(QDir dir);
    static bool LoadFile(QFileInfo info, int item_id);
    bool eventFilter(QObject *obj, QEvent *event);
    void ClearItemTag(void);
    void AddItemTags(QStringList tags, QStringList &tags_list, const QVariant &id_item);
    void RemoveItemTags(QStringList tags, QStringList &tags_list, const QVariant &id_item);
    void SyncItemTags(const QVariant &id_item, QSet<QString> new_tag_set, QSet<QString> old_tag_set);
    void loadExistingBooru(void);
    void readBooruSQLFile(QQueue<BooruTypeItem> &items);
    static void importBooruFromFile(QQueue<BooruTypeItem> *items, QMutex *items_mutex);
    void searchQueryParser(QStringList tag_list, QVector<BooruTypeItem> &items);
    void currentChanged(const QModelIndex &current, const QModelIndex &previous);
    ItemEditor* editor;
    Ui::BooruMenu *ui;
    QString file_or_db_path;
    QStringListModel tagModel;
    TagFilterProxyModel *proxyModel;
    bool searchInProgress = false;
};

#endif // BOORUMENU_H
