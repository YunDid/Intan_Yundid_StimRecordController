#include "DeviceManager.h"
#include "startupdialog.h"

#include <QApplication>
#include <QSplashScreen>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QSplashScreen *splash = new QSplashScreen();
    splash->setPixmap(QPixmap(":/images/splash.png"));

#ifdef __APPLE__
    app.setStyle(QStyleFactory::create("Fusion"));
    splash->setWindowFlags(Qt::WindowStaysOnBottomHint); // move splash screen to back in Mac OS, otherwise it covers startup dialog window
#elif __linux__
    splash->setWindowFlags(Qt::WindowStaysOnBottomHint); // not sure if we need this for Linux, but let's be safe
#endif

    splash->show();
    splash->showMessage(QObject::tr("Starting Intan Technologies Stimulation / Recording Controller v1.07..."), Qt::AlignCenter | Qt::AlignBottom, Qt::black);

    int sampleRateIndex, stimStepIndex;
    StartUpDialog *startUpDialog = new StartUpDialog(&sampleRateIndex, &stimStepIndex);
    startUpDialog->exec();

    DeviceManager w(sampleRateIndex, stimStepIndex);
    w.connectToDevice();
//    w.show();

//    splash->finish(&w);
    delete splash;
    return a.exec();
}
