#include <QApplication>
#include <iostream>
#include "plugin-interface.h"
#include "test/fft_test.h"

using namespace std;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    FFTTest f;
    f.init();

    return app.exec();
}

