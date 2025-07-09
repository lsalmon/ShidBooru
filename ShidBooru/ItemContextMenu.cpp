#include "ItemContextMenu.h"
#include <QDebug>

ItemContextMenu::ItemContextMenu(QWidget* parent, QPoint pos, BooruTypeItem *item_data)
{
    QMenu menu(parent);
    QAction *copy = menu.addAction("Copy picture to clipboard");
    QAction *save = menu.addAction("Save content of picture to file");

    QAction *selected = menu.exec(pos);

    if(selected == copy)
    {
        QClipboard *clipboard = QGuiApplication::clipboard();
        if(item_data->type == GIF)
        {
            QMimeData *mimeData = new QMimeData();
            mimeData->setData("image/gif", item_data->gif);
            clipboard->setMimeData(mimeData);
        }
        else
        {
            clipboard->setPixmap(item_data->picture);
        }
    }
    else if(selected == save)
    {
        QString file_path;

        if(item_data->type == GIF)
        {
            file_path = QFileDialog::getSaveFileName(this, "Save gif", QDir::homePath(), "GIF Images (*.gif)");
        }
        else
        {
            file_path = QFileDialog::getSaveFileName(this, "Save picture", QDir::homePath(), "Images (*.png *.jpg *.bmp)");
        }
        if(!file_path.isEmpty())
        {
            if(item_data->type == GIF)
            {
                QFile save_file(file_path);
                if(save_file.open(QIODevice::WriteOnly))
                {
                    save_file.write(item_data->gif);
                    save_file.close();
                }
            }
            else
            {
                if(!item_data->picture.save(file_path+'.'+item_data->extension, item_data->extension.toUpper().toUtf8().constData()))
                {
                    QMessageBox warning_item_missing;
                    warning_item_missing.setIcon(QMessageBox::Warning);
                    warning_item_missing.setText("Failed to save "+file_path+'.'+item_data->extension);
                    warning_item_missing.exec();
                }
            }
        }
    }
}
