#include <QtGui>
#include "slidersgroup.h"
#include "window.h"

Window::Window()
{
    upDown = 0;
    frontBack = 0;
    leftRight = 0;

    horizontalSliders = new SlidersGroup(Qt::Horizontal, tr(""));
    horizontalSliders->tag = 0;
    verticalSliders = new SlidersGroup(Qt::Vertical, tr(""));
    verticalSliders->tag = 0;

    horizontalSliders2 = new SlidersGroup(Qt::Horizontal, tr(""));
    horizontalSliders2->tag = 1;
    verticalSliders2 = new SlidersGroup(Qt::Vertical, tr(""));
    verticalSliders2->tag = 1;

    horizontalSliders3 = new SlidersGroup(Qt::Horizontal, tr(""));
    horizontalSliders3->tag = 2;
    verticalSliders3 = new SlidersGroup(Qt::Vertical, tr(""));
    verticalSliders3->tag = 2;

    stackedWidget = new QStackedWidget;
    stackedWidget->setStyleSheet("background-color: #3498db;"
                                 "width: 60px;"
                                 );
    stackedWidget->addWidget(verticalSliders);
    stackedWidget->addWidget(horizontalSliders);
    stackedWidget2 = new QStackedWidget;
    stackedWidget2->setStyleSheet("background-color: #3498db;"
                                  "width: 60px;"
                                  );
    stackedWidget2->addWidget(verticalSliders2);
    stackedWidget2->addWidget(horizontalSliders2);
    stackedWidget3 = new QStackedWidget;
    stackedWidget3->setStyleSheet("background-color: #3498db;"
                                  "height: 40px;");
    stackedWidget3->addWidget(horizontalSliders3);
    stackedWidget3->addWidget(verticalSliders3);

    createControls(tr(""));

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

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(stackedWidget, 0, 0,Qt::AlignLeft);
    layout->addWidget(controlsGroup, 0, 1,Qt::AlignCenter);
    layout->addWidget(stackedWidget2, 0, 2,Qt::AlignRight);
    layout->addWidget(stackedWidget3, 1, 1,Qt::AlignBottom);
    setLayout(layout);

    minimumSpinBox->setValue(0);
    maximumSpinBox->setValue(127);
    minimumSpinBox2->setValue(-63);
    maximumSpinBox2->setValue(63);
    valueSpinBox->setValue(0);
    valueSpinBox2->setValue(0);
    valueSpinBox3->setValue(0);

    setWindowTitle(tr("Sliders"));
}

void Window::writeValues()
{
    QFile file("/dev/arduino_comms");
    //    QFile file("/users/mmd1080/Desktop/535project/sample2.txt");
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);
    out << "m," << this->leftRight << "," << this->frontBack << "," << this->upDown ;
    file.close();
}

void Window::updateUpDown(int value)
{
    this->upDown = value;
    writeValues();
}

void Window::updateFrontBack(int value)
{
    this->frontBack = value;
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

    QFont font;
    font.setPointSize(20);
    font.setBold(true);

    minimumLabel = new QLabel(tr("Minimum value:"));
    maximumLabel = new QLabel(tr("Maximum value:"));

    valueLabel = new QLabel;
    QString myString = QString("<font color='white'>Up/Down:</font>");
    valueLabel->setText(myString);
    valueLabel->setStyleSheet("background-color: #3498db;");
    valueLabel->setFont( font);
    valueLabel2 = new QLabel;
    QString myString2 = QString("<font color='white'>Front/Back:</font>");
    valueLabel2->setText(myString2);
    valueLabel2->setStyleSheet("background-color: #3498db");
    valueLabel2->setFont( font);
    valueLabel3 = new QLabel;
    QString myString3 = QString("<font color='white'>Left/Right:</font>");
    valueLabel3->setText(myString3);
    valueLabel3->setStyleSheet("background-color: #3498db");
    valueLabel3->setFont( font);

    invertedAppearance = new QCheckBox(tr("Inverted appearance"));
    invertedKeyBindings = new QCheckBox(tr("Inverted key bindings"));

    minimumSpinBox = new QSpinBox;
    minimumSpinBox->setRange(0, 127);
    minimumSpinBox->setSingleStep(1);

    maximumSpinBox = new QSpinBox;
    maximumSpinBox->setRange(0, 127);
    maximumSpinBox->setSingleStep(1);

    minimumSpinBox2 = new QSpinBox;
    minimumSpinBox2->setRange(-63, 63);
    minimumSpinBox2->setSingleStep(1);

    maximumSpinBox2 = new QSpinBox;
    maximumSpinBox2->setRange(-63, 63);
    maximumSpinBox2->setSingleStep(1);

    QPalette* palette = new QPalette();
    palette->setColor(QPalette::Text,Qt::white);

int height = 35;
int width = 60;

    valueSpinBox = new QSpinBox;
    valueSpinBox->setStyleSheet("background-color: #3498db");
    valueSpinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
    valueSpinBox->setRange(0, 127);
    valueSpinBox->setSingleStep(1);
    valueSpinBox->setFixedHeight(height);
    valueSpinBox->setFixedWidth(width);
    valueSpinBox->setFont(font);
    valueSpinBox->setPalette(*palette);

    valueSpinBox2 = new QSpinBox;
    valueSpinBox2->setStyleSheet("background-color: #3498db");
    valueSpinBox2->setButtonSymbols(QAbstractSpinBox::NoButtons);
    valueSpinBox2->setRange(-63, 63);
    valueSpinBox2->setSingleStep(1);
    valueSpinBox2->setFixedHeight(height);
    valueSpinBox2->setFixedWidth(width);
    valueSpinBox2->setFont(font);
    valueSpinBox2->setPalette(*palette);

    valueSpinBox3 = new QSpinBox;
    valueSpinBox3->setStyleSheet("background-color: #3498db");
    valueSpinBox3->setButtonSymbols(QAbstractSpinBox::NoButtons);
    valueSpinBox3->setRange(-63, 63);
    valueSpinBox3->setSingleStep(1);
    valueSpinBox3->setFixedHeight(height);
    valueSpinBox3->setFixedWidth(width);
    valueSpinBox3->setFont(font);
    valueSpinBox3->setPalette(*palette);

    orientationCombo = new QComboBox;
    orientationCombo->addItem(tr("Vertical slider-like widgets"));
    orientationCombo->addItem(tr("Horizontal slider-like widgets"));

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
    connect(invertedAppearance, SIGNAL(toggled(bool)),
            horizontalSliders, SLOT(invertAppearance(bool)));
    connect(invertedAppearance, SIGNAL(toggled(bool)),
            verticalSliders, SLOT(invertAppearance(bool)));
    connect(invertedKeyBindings, SIGNAL(toggled(bool)),
            horizontalSliders, SLOT(invertKeyBindings(bool)));
    connect(invertedKeyBindings, SIGNAL(toggled(bool)),
            verticalSliders, SLOT(invertKeyBindings(bool)));

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
    connect(invertedAppearance, SIGNAL(toggled(bool)),
            horizontalSliders2, SLOT(invertAppearance(bool)));
    connect(invertedAppearance, SIGNAL(toggled(bool)),
            verticalSliders2, SLOT(invertAppearance(bool)));
    connect(invertedKeyBindings, SIGNAL(toggled(bool)),
            horizontalSliders2, SLOT(invertKeyBindings(bool)));
    connect(invertedKeyBindings, SIGNAL(toggled(bool)),
            verticalSliders2, SLOT(invertKeyBindings(bool)));

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
    connect(invertedAppearance, SIGNAL(toggled(bool)),
            horizontalSliders3, SLOT(invertAppearance(bool)));
    connect(invertedAppearance, SIGNAL(toggled(bool)),
            verticalSliders3, SLOT(invertAppearance(bool)));
    connect(invertedKeyBindings, SIGNAL(toggled(bool)),
            horizontalSliders3, SLOT(invertKeyBindings(bool)));
    connect(invertedKeyBindings, SIGNAL(toggled(bool)),
            verticalSliders3, SLOT(invertKeyBindings(bool)));

    QGridLayout *controlsLayout = new QGridLayout;
    controlsLayout->addWidget(valueLabel, 0, 0);
    controlsLayout->addWidget(valueLabel2, 1, 0);
    controlsLayout->addWidget(valueLabel3, 2, 0);
    controlsLayout->addWidget(valueSpinBox, 0, 1);
    controlsLayout->addWidget(valueSpinBox2, 1, 1);
    controlsLayout->addWidget(valueSpinBox3, 2, 1);
    controlsGroup->setLayout(controlsLayout);
}
