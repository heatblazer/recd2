#ifndef FFT_TEST_H
#define FFT_TEST_H

#include <QtWidgets/QWidget>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>

// fails with union
struct Layout
{
    QHBoxLayout hlay;
    QVBoxLayout vlay;
};


struct Screen : public QWidget
{
    explicit Screen(QWidget* parent = nullptr);
    virtual ~Screen();

};

class FFTTest : public QWidget
{
public:
    explicit FFTTest(QWidget* parent = nullptr);
    virtual ~FFTTest();
    void init();

private:
    QFileDialog m_fbrowser;
    QPushButton m_select;
    QPushButton m_doFFT;
    QWidget* m_screen;
    QString m_loadedFile;
    QByteArray m_fdata;
    Layout m_widgets;
};



#endif // FFT_TEST_H
