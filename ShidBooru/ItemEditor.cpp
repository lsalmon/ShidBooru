#include "ItemEditor.h"
#include "ui_ItemEditor.h"
#include "QSqlQueryHelper.h"
#include <QDebug>

static QSqlDatabase db;

ItemEditor::ItemEditor(QWidget *parent,
                       BooruTypeItem *_item) :
    QDialog(parent),
    item(_item),
    ui(new Ui::ItemEditor)
{
    db = QSqlDatabase::database();
    ui->setupUi(this);

    if(item->type != MOVIE)
    {
        ui->picture->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        ui->picture->setScaledContents(true);
    }

    if(item->type == GIF)
    {
        ui->playButton->hide();
        ui->pauseButton->hide();
        ui->positionSlider->hide();
        ui->videoWidget->hide();
        ui->volumeSlider->hide();
        ui->picture->show();

        QFile file(item->path);
        if(file.open(QIODevice::ReadOnly))
        {
            gif = file.readAll();
            buf = new QBuffer(&gif);
            buf->open(QIODevice::ReadOnly);
            gif_movie = new QMovie(buf, QByteArray(), this);
            ui->picture->setMovie(gif_movie);
            ui->picture->resize(gif_movie->scaledSize());
            gif_movie->start();
        }
    }
    else if(item->type == MOVIE)
    {
        ui->picture->hide();
        ui->playButton->show();
        ui->pauseButton->show();
        ui->positionSlider->show();
        ui->volumeSlider->show();
        ui->videoWidget->show();

        player = new QMediaPlayer(this);
        player->setVideoOutput(ui->videoWidget);
        // Default volume is 50%
        player->setVolume(ui->volumeSlider->value());
        // Get duration of media
        connect(player, &QMediaPlayer::durationChanged, this, [&](qint64 duration){
            ui->positionSlider->setMaximum(duration);
        });
        player->setMedia(QUrl::fromLocalFile(item->path));
        connect(ui->playButton, &QAbstractButton::clicked, this, &ItemEditor::PlayPressed);
        connect(ui->pauseButton, &QAbstractButton::clicked, this, &ItemEditor::PausePressed);
        connect(ui->volumeSlider, &QSlider::valueChanged, this, &ItemEditor::VolumeSliderValueUpdated);
        connect(player, &QMediaPlayer::positionChanged, this, &ItemEditor::PositionSliderUpdate);
        connect(ui->positionSlider, &QSlider::sliderMoved, this, &ItemEditor::PositionSliderSeek);
    }
    else
    {
        ui->playButton->hide();
        ui->pauseButton->hide();
        ui->positionSlider->hide();
        ui->videoWidget->hide();
        ui->volumeSlider->hide();
        ui->picture->show();

        QImage image(item->path);
        QPixmap picture = QPixmap::fromImage(image);

        ui->picture->setPixmap(picture);
        ui->picture->resize(picture.size());
    }

    // Set tags to a model inside the dialog box and update the QStringList of the model
    ui->tagListView->setModel(&default_tag_model);

    QStringList tags_list;
    getTagsFromItemQuery(item->sql_id, tags_list);
    this->default_tag_model.setStringList(tags_list);

    connect(ui->addButton, &QPushButton::clicked, this, &ItemEditor::AddTag);
    connect(ui->removeButton, &QPushButton::clicked, this, &ItemEditor::RemoveSelectedTag);
}

void ItemEditor::PlayPressed()
{
    player->play();
}

void ItemEditor::PausePressed()
{
    player->pause();
}

void ItemEditor::VolumeSliderValueUpdated(int value)
{
    // setMuted or volume to 0 does not mute, what is going on ?
    player->setMuted(value == 0);
    player->setVolume(value);
}

void ItemEditor::PositionSliderUpdate(qint64 position)
{
    if(!ui->positionSlider->isSliderDown())
    {
        ui->positionSlider->setValue(position);
    }
}

void ItemEditor::PositionSliderSeek(int value)
{
    player->setPosition(value);
}

void ItemEditor::AddTag()
{
    QString tag = this->ui->tagLineEdit->text().trimmed();
    // Check if LineEdit not empty and model exists
    if(!tag.isEmpty() && this->ui->tagListView->model() != nullptr)
    {
        QStringList tag_input_list = tag.split(" ", Qt::SkipEmptyParts);
        for(const QString &tag  : tag_input_list)
        {
            // Remove empty strings
            if(!tag.isEmpty())
            {
                // Check if tag not already in list
                QStringList tag_list = this->default_tag_model.stringList();
                if(tag_list.indexOf(tag) != -1) {
                    QMessageBox::warning(this, tr("ShidBooru"), tr("Tag already exists"));
                } else {
                    int id_tag = checkDuplicateTagQuery(tag);
                    if(id_tag < 0) {
                        id_tag = addTagQuery(tag).toInt();
                        qDebug() << "New tag id " << id_tag;
                    } else {
                        id_tag = getIDFromTagQuery(tag);
                        qDebug() << "Got existing tag id " << id_tag;
                    }
                    int id_link = checkDuplicateLinkQuery(QVariant(id_tag), item->sql_id);
                    if(id_link < 0) {
                        qDebug() << "New link id (tag -> item) " << QString(id_tag) << " -> " << item->sql_id.toString();
                        addLinkQuery(item->sql_id, QVariant(id_tag));
                    }
                    // Update list locally
                    tag_list.append(tag);
                    this->default_tag_model.setStringList(tag_list);
                }
                this->ui->tagLineEdit->clear();
            }
        }
    }
}

void ItemEditor::RemoveSelectedTag()
{
    QModelIndex tag_index = this->ui->tagListView->currentIndex();
    if(tag_index.isValid())
    {
        QStringList tag_list = this->default_tag_model.stringList();
        QString tag = tag_list.at(tag_index.row());
        int tag_id = getIDFromTagQuery(tag);
        if(tag_id < 0)
        {
            qDebug() << "Cannot get tag id for tag "+tag;
            return ;
        }

        // Remove link
        if(!removeLinkQuery(QVariant(item->sql_id), QVariant(tag_id)))
        {
            qDebug() << "Cannot remove link to "+tag;
            return ;
        }

        // If link was the last to use the tag, also remove tag
        QVector<BooruTypeItem> items;
        getItemsFromSingleTagQuery(tag, items);

        if(items.empty()) {
            qDebug() << "Remove tag "+tag+" completely";
            removeTagQuery(tag);
        }

        // Update list locally
        tag_list.removeAt(tag_index.row());
        this->default_tag_model.setStringList(tag_list);
    }
}

void ItemEditor::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::RightButton)
    {
        ItemContextMenu menu(this, event->globalPos(), item);
    }
}

// For crashes when clicking the cross
void ItemEditor::closeEvent(QCloseEvent* event)
{
    if(item->type == MOVIE)
    {
        player->stop();
    }
}

ItemEditor::~ItemEditor()
{
    if(item->type == GIF || item->type == STILL_IMG)
    {
        ui->picture->clear();
    }
    delete ui;
}
