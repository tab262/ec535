#ifndef SLIDERSGROUP_H
#define SLIDERSGROUP_H

#include <QGroupBox>

class QDial;
class QScrollBar;
class QSlider;

class SlidersGroup : public QGroupBox
{
    Q_OBJECT

public:
    SlidersGroup(Qt::Orientation orientation, const QString &title,
                 QWidget *parent = 0);
    int tag;

signals:
    void valueChanged(int value);

public slots:
    void setValue(int value);
    void setMinimum(int value);
    void setMaximum(int value);
    void invertAppearance(bool invert);
    void invertKeyBindings(bool invert);

private:
    QSlider *slider;
    QSlider *slider2;
    QDial *dial;
};

#endif
