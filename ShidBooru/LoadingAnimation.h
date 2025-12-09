#ifndef LOADINGANIMATION_H
#define LOADINGANIMATION_H

#include <QDialog>
#include "ui_LoadingAnimation.h"

namespace Ui {
class LoadingAnimation;
}

class LoadingAnimation : public QDialog
{
    Q_OBJECT

public:
    explicit LoadingAnimation(QWidget *parent = nullptr) :
        QDialog(parent),
        ui(new Ui::LoadingAnimation)
    {
        Q_UNUSED(parent);
        ui->setupUi(this);
    }
    ~LoadingAnimation()
    {
        delete ui;
    }
private:
    Ui::LoadingAnimation *ui;
};

#endif // LOADINGANIMATION_H
