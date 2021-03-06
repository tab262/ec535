#include <QtGui>
#include "slidersgroup.h"
#include "window.h"

Window::Window()
{
    upDown = 0;
    //i.e. Throttle
    frontBack = 0;
    //i.e. Pitch
    leftRight = 0;
    //i.e. Yaw

    /*
    Why SlidersGroups?

    In the original implementation, I experimented with multiple sliders per widget.
    In this final version, there is only one slider per widget.
    Also note that SlidersGroups set up both horizontal and vertical sliders in case they are needed.
    The SlidersGroup class could be eliminated in a future implementation.
    */

    //Throttle Widget:
    horizontalSliders = new SlidersGroup(Qt::Horizontal, tr(""));
    verticalSliders = new SlidersGroup(Qt::Vertical, tr(""));

    stackedWidget = new QStackedWidget;
    stackedWidget->setStyleSheet("background-color: #3498db;"
                                 "width: 60px;"
                                 );
    stackedWidget->addWidget(verticalSliders);
    stackedWidget->addWidget(horizontalSliders);

    //Pitch Widget:
    horizontalSliders2 = new SlidersGroup(Qt::Horizontal, tr(""));
    verticalSliders2 = new SlidersGroup(Qt::Vertical, tr(""));

    stackedWidget2 = new QStackedWidget;
    stackedWidget2->setStyleSheet("background-color: #3498db;"
                                  "width: 60px;"
                                  );
    stackedWidget2->addWidget(verticalSliders2);
    stackedWidget2->addWidget(horizontalSliders2);

    //Yaw Widget:
    horizontalSliders3 = new SlidersGroup(Qt::Horizontal, tr(""));
    verticalSliders3 = new SlidersGroup(Qt::Vertical, tr(""));

    stackedWidget3 = new QStackedWidget;
    stackedWidget3->setStyleSheet("background-color: #3498db;"
                                  "height: 40px;");
    stackedWidget3->addWidget(horizontalSliders3);
    stackedWidget3->addWidget(verticalSliders3);

    //Reset Button Widget:
    resetButton = new QPushButton(tr("Reset"));
    QFont font;
    font.setPointSize(20);
    font.setBold(true);
    resetButton->setFont(font);
    resetButton->setStyleSheet("color: white;"
                               "background-color: #4aa3df");

    resetButtonWidget = new QStackedWidget;
    resetButtonWidget->setStyleSheet("background-color: #3498db;"
                                     "width: 60px;"
                                     );
    resetButtonWidget->addWidget(resetButton);

    //Control Values Widget (QGroupBox controlsGroup):
    createControls(tr(""));

    //Set up functionality:
    connect(resetButton, SIGNAL(pressed()), this, SLOT(reset()));

    //Adjusting a slider adjusts the valueSpinBox and calls that value's update function
    connect(horizontalSliders, SIGNAL(valueChanged(int)),
            verticalSliders, SLOT(setValue(int)));
    connect(verticalSliders, SIGNAL(valueChanged(int)),
            valueSpinBox, SLOT(setValue(int)));
    connect(valueSpinBox, SIGNAL(valueChanged(int)),
            horizontalSliders, SLOT(setValue(int)));
    connect(valueSpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(updateUpDown(int)));

    connect(horizontalSliders2, SIGNAL(valueChanged(int)),
            verticalSliders2, SLOT(setValue(int)));
    connect(verticalSliders2, SIGNAL(valueChanged(int)),
            valueSpinBox2, SLOT(setValue(int)));
    connect(valueSpinBox2, SIGNAL(valueChanged(int)),
            horizontalSliders2, SLOT(setValue(int)));
    connect(valueSpinBox2, SIGNAL(valueChanged(int)),
            this, SLOT(updateFrontBack(int)));

    connect(horizontalSliders3, SIGNAL(valueChanged(int)),
            verticalSliders3, SLOT(setValue(int)));
    connect(verticalSliders3, SIGNAL(valueChanged(int)),
            valueSpinBox3, SLOT(setValue(int)));
    connect(valueSpinBox3, SIGNAL(valueChanged(int)),
            horizontalSliders3, SLOT(setValue(int)));
    connect(valueSpinBox3, SIGNAL(valueChanged(int)),
            this, SLOT(updateLeftRight(int)));

    //Layout of the five widgets:
    QGridLayout *layout = new QGridLayout;
    //Top row: Throttle, controlsGroup, Pitch
    layout->addWidget(stackedWidget, 0, 0,Qt::AlignLeft);
    layout->addWidget(controlsGroup, 0, 1,Qt::AlignCenter);
    layout->addWidget(stackedWidget2, 0, 2,Qt::AlignRight);
    //Bottom row: resetButton, Yaw
    layout->addWidget(stackedWidget3, 1, 1,1,2,Qt::AlignBottom);
    layout->addWidget(resetButtonWidget, 1, 0,Qt::AlignBottom);
    setLayout(layout);

    //Initialize values:
    minimumSpinBox->setValue(0);
    maximumSpinBox->setValue(127);
    minimumSpinBox2->setValue(-63);
    maximumSpinBox2->setValue(63);
    valueSpinBox->setValue(0);
    valueSpinBox2->setValue(0);
    valueSpinBox3->setValue(0);
}

void Window::writeValues()
{
    QFile file("/dev/arduino_comms");
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);
    out << "m," << this->leftRight << "," << this->frontBack << "," << this->upDown ;
    //The kernel only receives an update from the controller when any of the value are updated.
    //All updates from controller are manual (m) mode updates.
    file.close();
}

void Window::reset()
{
    int value = 0;
    valueSpinBox->setValue(value);
    valueSpinBox2->setValue(value);
    valueSpinBox3->setValue(value);
    this->upDown = value;
    this->frontBack = value;
    this->leftRight = value;
    writeValues();
}

void Window::updateUpDown(int value)
{
    this->upDown = value;
    writeValues();
}

void Window::updateFrontBack(int value)
{
    this->frontBack = -value;
    //negative values actually translate to forward movement.
    writeValues();
}

void Window::updateLeftRight(int value)
{
    this->leftRight = value;
    writeValues();
}

void Window::createControls(const QString &title)
{
    controlsGroup = new QGroupBox(title);
    controlsGroup->setStyleSheet("background-color: #3498db;");
    orientationCombo = new QComboBox;
    orientationCombo->addItem(tr("Vertical slider-like widgets"));
    orientationCombo->addItem(tr("Horizontal slider-like widgets"));

    //There are two possible ranges which will be used:
    //Range: 0-127
    minimumSpinBox = new QSpinBox;
    minimumSpinBox->setRange(0, 127);
    minimumSpinBox->setSingleStep(1);

    maximumSpinBox = new QSpinBox;
    maximumSpinBox->setRange(0, 127);
    maximumSpinBox->setSingleStep(1);

    //Range: (-63)-63
    minimumSpinBox2 = new QSpinBox;
    minimumSpinBox2->setRange(-63, 63);
    minimumSpinBox2->setSingleStep(1);

    maximumSpinBox2 = new QSpinBox;
    maximumSpinBox2->setRange(-63, 63);
    maximumSpinBox2->setSingleStep(1);

    //The font size and style, as well as height/width, will be shared by all labels:
    QFont font;
    font.setPointSize(20);
    font.setBold(true);

    QPalette* palette = new QPalette();
    palette->setColor(QPalette::Text,Qt::white);

    int height = 35;
    int width = 60;

    //Throttle label:
    valueLabel = new QLabel;
    QString myString = QString("<font color='white'>Throttle:</font>");//was up/down
    valueLabel->setText(myString);
    valueLabel->setStyleSheet("background-color: #3498db;");
    valueLabel->setFont( font);

    valueSpinBox = new QSpinBox;
    valueSpinBox->setStyleSheet("background-color: #3498db");
    valueSpinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
    valueSpinBox->setRange(0, 127);
    valueSpinBox->setSingleStep(1);
    valueSpinBox->setFixedHeight(height);
    valueSpinBox->setFixedWidth(width);
    valueSpinBox->setFont(font);
    valueSpinBox->setPalette(*palette);

    connect(orientationCombo, SIGNAL(activated(int)),
            stackedWidget, SLOT(setCurrentIndex(int)));
    connect(minimumSpinBox, SIGNAL(valueChanged(int)),
            horizontalSliders, SLOT(setMinimum(int)));
    connect(minimumSpinBox, SIGNAL(valueChanged(int)),
            verticalSliders, SLOT(setMinimum(int)));
    connect(maximumSpinBox, SIGNAL(valueChanged(int)),
            horizontalSliders, SLOT(setMaximum(int)));
    connect(maximumSpinBox, SIGNAL(valueChanged(int)),
            verticalSliders, SLOT(setMaximum(int)));

    //Pitch label:
    valueLabel2 = new QLabel;
    QString myString2 = QString("<font color='white'>Pitch:</font>");//was front/back
    valueLabel2->setText(myString2);
    valueLabel2->setStyleSheet("background-color: #3498db");
    valueLabel2->setFont( font);

    valueSpinBox2 = new QSpinBox;
    valueSpinBox2->setStyleSheet("background-color: #3498db");
    valueSpinBox2->setButtonSymbols(QAbstractSpinBox::NoButtons);
    valueSpinBox2->setRange(-63, 63);
    valueSpinBox2->setSingleStep(1);
    valueSpinBox2->setFixedHeight(height);
    valueSpinBox2->setFixedWidth(width);
    valueSpinBox2->setFont(font);
    valueSpinBox2->setPalette(*palette);

    connect(orientationCombo, SIGNAL(activated(int)),
            stackedWidget2, SLOT(setCurrentIndex(int)));
    connect(minimumSpinBox2, SIGNAL(valueChanged(int)),
            horizontalSliders2, SLOT(setMinimum(int)));
    connect(minimumSpinBox2, SIGNAL(valueChanged(int)),
            verticalSliders2, SLOT(setMinimum(int)));
    connect(maximumSpinBox2, SIGNAL(valueChanged(int)),
            horizontalSliders2, SLOT(setMaximum(int)));
    connect(maximumSpinBox2, SIGNAL(valueChanged(int)),
            verticalSliders2, SLOT(setMaximum(int)));

    //Yaw label:
    valueLabel3 = new QLabel;
    QString myString3 = QString("<font color='white'>Yaw:</font>");//was left/right
    valueLabel3->setText(myString3);
    valueLabel3->setStyleSheet("background-color: #3498db");
    valueLabel3->setFont( font);

    valueSpinBox3 = new QSpinBox;
    valueSpinBox3->setStyleSheet("background-color: #3498db");
    valueSpinBox3->setButtonSymbols(QAbstractSpinBox::NoButtons);
    valueSpinBox3->setRange(-63, 63);
    valueSpinBox3->setSingleStep(1);
    valueSpinBox3->setFixedHeight(height);
    valueSpinBox3->setFixedWidth(width);
    valueSpinBox3->setFont(font);
    valueSpinBox3->setPalette(*palette);

    connect(orientationCombo, SIGNAL(activated(int)),
            stackedWidget3, SLOT(setCurrentIndex(int)));
    connect(minimumSpinBox2, SIGNAL(valueChanged(int)),
            horizontalSliders3, SLOT(setMinimum(int)));
    connect(minimumSpinBox2, SIGNAL(valueChanged(int)),
            verticalSliders3, SLOT(setMinimum(int)));
    connect(maximumSpinBox2, SIGNAL(valueChanged(int)),
            horizontalSliders3, SLOT(setMaximum(int)));
    connect(maximumSpinBox2, SIGNAL(valueChanged(int)),
            verticalSliders3, SLOT(setMaximum(int)));

    //Layout Value labels:
    QGridLayout *controlsLayout = new QGridLayout;
    //Top row: Throttle
    controlsLayout->addWidget(valueLabel, 0, 0);
    controlsLayout->addWidget(valueSpinBox, 0, 1);
    //Middle row: Pitch
    controlsLayout->addWidget(valueLabel2, 1, 0);
    controlsLayout->addWidget(valueSpinBox2, 1, 1);
    //Bottom row: Yaw
    controlsLayout->addWidget(valueLabel3, 2, 0);
    controlsLayout->addWidget(valueSpinBox3, 2, 1);
    controlsGroup->setLayout(controlsLayout);
}
