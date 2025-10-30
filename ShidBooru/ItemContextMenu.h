#ifndef ITEMCONTEXTMENU_H
#define ITEMCONTEXTMENU_H

#include <QMenu>
#include <QFileDialog>
#include <QGuiApplication>
#include <QClipboard>
#include <QUrl>
#include <QMimeData>
#include <QMessageBox>
#include "BooruItemType.h"
#include "HelperFunctions.h"

class ItemContextMenu : public QMenu
{
public:
    ItemContextMenu(QWidget* parent, QPoint pos, BooruTypeItem *item);
};

#endif // ITEMCONTEXTMENU_H
