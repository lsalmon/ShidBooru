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
#include <QClipboard>
#include "BooruItemType.h"

namespace Ui {
class ItemEditor;
}

class ItemEditor : public QDialog
{
    Q_OBJECT

public:
    explicit ItemEditor(QWidget *parent = nullptr,
                        BooruTypeItem *_item = nullptr);
    QStringList GetUpdatedTags();
    ~ItemEditor();

private slots:
    void AddTag();
    void RemoveSelectedTag();

private:
    BooruTypeItem *item;
    QStringListModel default_tag_model;
    Ui::ItemEditor *ui;

protected:
    void mousePressEvent(QMouseEvent* event);
};

#endif // ITEMEDITOR_H
