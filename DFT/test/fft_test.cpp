#include "fft_test.h"
#include <QFile>

Screen::Screen(QWidget *parent)
{

}

Screen::~Screen()
{

}


FFTTest::FFTTest(QWidget *parent)
    : QWidget(parent),
      m_screen(nullptr)
{
}

FFTTest::~FFTTest()
{

}

void FFTTest::init()
{
    m_screen = new Screen(this);

    m_doFFT.setText("do fft");
    m_select.setText("choose file");
    m_screen->setMaximumSize(500, 400);
    m_screen->setMinimumSize(500, 400);


    m_widgets.vlay.addWidget(&m_select);
    m_widgets.vlay.addWidget(&m_doFFT);
    m_widgets.hlay.addWidget(m_screen);
    m_widgets.hlay.addLayout(&m_widgets.vlay);

    this->setLayout(&m_widgets.hlay);
    setMinimumSize(640, 480);
    setMaximumSize(640, 480);

    connect(&m_select, &QPushButton::clicked,
            [=](bool)
    {
        m_fbrowser.show();
    });

    connect(&m_doFFT, &QPushButton::clicked,
            [=](bool)
    {
       // break here
        FFTTest* ffff = this; // jsut for debug break point for this
        // clojure does not suppor viewing ... lazy but bad for debugging
        // and disconnection
        // do the stuff here
    });

    connect(&m_fbrowser, &QFileDialog::fileSelected,
            [=](const QString& ref)
    {
        m_loadedFile = ref;// set the name to open
        QFile f(m_loadedFile);
        if (!f.open(QIODevice::ReadOnly)) {
            m_fbrowser.hide(); /// bla bla failed to open file
        } else {
            m_fdata = f.readAll();
            f.close();
        }

    });

    show();
}

