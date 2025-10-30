#include "ItemContextMenu.h"
#include <QDebug>

ItemContextMenu::ItemContextMenu(QWidget* parent, QPoint pos, BooruTypeItem *item_data)
{
    QMenu menu(parent);
    QAction *copy;
    if(item_data->type == GIF)
    {
        copy = menu.addAction("Copy url of gif to clipboard");
    }
    else if(item_data->type == MOVIE)
    {
        copy = menu.addAction("Copy url of movie to clipboard");
    }
    else
    {
        copy = menu.addAction("Copy picture to clipboard");
    }
    QAction *save = menu.addAction("Save content to file");

    QAction *selected = menu.exec(pos);

    if(selected == copy)
    {
        QClipboard *clipboard = QGuiApplication::clipboard();
        if(item_data->type == GIF || item_data->type == MOVIE)
        {
            QFile file(item_data->path);
            if(file.open(QIODevice::ReadOnly))
            {
                QMimeData *mimeData = new QMimeData();
                QList<QUrl> url;
                url.append(QUrl::fromLocalFile(item_data->path));

                mimeData->setUrls(url);
                clipboard->setMimeData(mimeData, QClipboard::Clipboard);
            }
            else
            {
                DisplayWarningMessage("Failed to extract url for "+item_data->path);
            }
        }
        else
        {
            QImage image(item_data->path);
            QPixmap picture = QPixmap::fromImage(image);
            if(picture.isNull())
            {
                DisplayWarningMessage("Failed to extract picture for "+item_data->path);
            }
            else
            {
                clipboard->setPixmap(picture);
            }
        }
    }
    else if(selected == save)
    {
        QString file_path;

        if(item_data->type == GIF)
        {
            file_path = QFileDialog::getSaveFileName(this, "Save gif", QDir::homePath(), "GIF Images (*.gif)");
        }
        else if(item_data->type == MOVIE)
        {
            file_path = QFileDialog::getSaveFileName(this, "Save movie", QDir::homePath(), "Movies (*.mp4 *.webm)");
        }
        else
        {
            file_path = QFileDialog::getSaveFileName(this, "Save picture", QDir::homePath(), "Images (*.png *.jpg *.jpeg *.webp)");
        }
        if(!file_path.isEmpty() && !file_path.isNull())
        {
            if(item_data->type == GIF || item_data->type == MOVIE)
            {
                QFile save_file(file_path);
                if(save_file.open(QIODevice::WriteOnly))
                {
                    QByteArray raw_data;
                    QFile file(item_data->path);
                    if(file.open(QIODevice::ReadOnly))
                    {
                        raw_data = file.readAll();
                        save_file.write(raw_data);
                    }
                    else
                    {
                        DisplayWarningMessage("Failed to save "+file_path+'.'+item_data->extension);
                    }
                    save_file.close();
                }
            }
            else
            {
                QImage image(item_data->path);
                QPixmap picture = QPixmap::fromImage(image);
                if(!picture.save(file_path+'.'+item_data->extension, item_data->extension.toUpper().toUtf8().constData()))
                {
                    DisplayWarningMessage("Failed to save "+file_path+'.'+item_data->extension);
                }
            }
        }
        else
        {
            DisplayInfoMessage("Save cancelled by user");
        }
    }
}
