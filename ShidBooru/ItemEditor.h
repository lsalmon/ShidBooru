#ifndef ITEMEDITOR_H
#define ITEMEDITOR_H

#include <QDialog>
#include <QListWidget>
#include <QStandardItemModel>
#include <QStringListModel>
#include <QDebug>
#include <QMessageBox>
#include <QMouseEvent>
#include <QMenu>
#include <QAction>
#include <QBuffer>
#include <QMovie>
#include <QScreen>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include "BooruItemType.h"
#include "ItemContextMenu.h"

namespace Ui {
class ItemEditor;
}

class ItemEditor : public QDialog
{
    Q_OBJECT

public:
    explicit ItemEditor(QWidget *parent = nullptr,
                        BooruTypeItem *_item = nullptr);
    ~ItemEditor();

private slots:
    void AddTag();
    void RemoveSelectedTag();

private:
    BooruTypeItem *item;
    QStringListModel default_tag_model;
    Ui::ItemEditor *ui;
    QByteArray gif;
    QBuffer *buf;
    QMovie *gif_movie;

protected:
    void mousePressEvent(QMouseEvent* event);
};

#endif // ITEMEDITOR_H
