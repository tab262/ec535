#include <QtGui>
#include <QRect>
#include "slidersgroup.h"

SlidersGroup::SlidersGroup(Qt::Orientation orientation, const QString &title,
                           QWidget *parent)
    : QGroupBox(title, parent)
{
    slider = new QSlider(orientation);

    if (orientation == Qt::Horizontal)
        //This will be used for the yaw slider
        slider->setStyleSheet(
                    "QSlider::handle:orientation {"
                    "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #dde4e6, stop:1 #ecf0f1);"
                    "border: 1px solid #5c5c5c;"
                    "width: 45px;"
                    "height: 10px;"
                    "margin: -2px 0;"
                    "border-radius: 15px;}"

                    "QSlider::groove:orientation {"
                    "border: 1px solid #999999;"
                    "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #4aa3df, stop:1 #5faee3);"
                    "height: 35px;"
                    //"width: 90px;"
                    "margin: 0 0 0 0;}"
                    );
    else
        //This will be used for throttle & pitch sliders
        slider->setStyleSheet(
                    "QSlider::handle:orientation {"
                    "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #dde4e6, stop:1 #ecf0f1);"
                    "border: 1px solid #5c5c5c;"
                    //"width: 60px;"
                    "height: 60;"
                    "margin: -2px 0;"
                    "border-radius: 20px;}"

                    "QSlider::groove:orientation {"
                    "border: 1px solid #999999;"
                    "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #4aa3df, stop:1 #5faee3);"
                    "height: 150px;"
                    //"width: 60px;"
                    "margin: 0 0 0 0;}"
                    );

    slider->setFocusPolicy(Qt::StrongFocus);
    slider->setTickPosition(QSlider::TicksBothSides);
    slider->setTickInterval(10);
    slider->setSingleStep(1);

    connect(slider, SIGNAL(valueChanged(int)), this, SIGNAL(valueChanged(int)));

    QBoxLayout::Direction direction;

    if (orientation == Qt::Horizontal)
        direction = QBoxLayout::TopToBottom;
    else
        direction = QBoxLayout::LeftToRight;

    QBoxLayout *slidersLayout = new QBoxLayout(direction);
    slidersLayout->addWidget(slider);
    setLayout(slidersLayout);
}

void SlidersGroup::setValue(int value)
{
    slider->setValue(value);
}

void SlidersGroup::setMinimum(int value)
{
    slider->setMinimum(value);
}

void SlidersGroup::setMaximum(int value)
{
    slider->setMaximum(value);
}
