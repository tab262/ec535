#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>

class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QSpinBox;
class QStackedWidget;
class QPushButton;
class SlidersGroup;

class Window : public QWidget
{
    Q_OBJECT

public:
    Window();

public slots:
    void updateUpDown(int value);
    void updateFrontBack(int value);
    void updateLeftRight(int value);
    void reset();

private:
    void createControls(const QString &title);
    void writeValues();

    int upDown;
    int frontBack;
    int leftRight;

    SlidersGroup *horizontalSliders;
    SlidersGroup *verticalSliders;
    QStackedWidget *stackedWidget;

    SlidersGroup *horizontalSliders2;
    SlidersGroup *verticalSliders2;
    QStackedWidget *stackedWidget2;

    SlidersGroup *horizontalSliders3;
    SlidersGroup *verticalSliders3;
    QStackedWidget *stackedWidget3;

    QPushButton * resetButton;
    QStackedWidget *resetButtonWidget;

    QGroupBox *controlsGroup;
    QComboBox *orientationCombo;
    QSpinBox *maximumSpinBox;
    QSpinBox *minimumSpinBox;
    QSpinBox *maximumSpinBox2;
    QSpinBox *minimumSpinBox2;

    QLabel *valueLabel;
    QSpinBox *valueSpinBox;

    QLabel *valueLabel2;
    QSpinBox *valueSpinBox2;

    QLabel *valueLabel3;
    QSpinBox *valueSpinBox3;
};

#endif
