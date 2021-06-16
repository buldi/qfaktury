#include "mainwindow.h"
#include "runguard.h"
#include "settings.h"

#include "debug_message.h"

#include <QMessageBox>
#include <QtCore/QCommandLineParser>
#include <QtCore/QResource>
#include <QtWidgets/QApplication>
#include <QtWidgets/QSplashScreen>
#include <QtWidgets/QStyle>

#include <QDebug>

int main(int argc, char **argv)
{
    StrDebug();

    RunGuard guard("main_run_protection");
    if (!guard.tryToRun())
        return 0;

    QApplication application(argc, argv);

    // creates instance of main window and move it in according to the max screen
    MainWindow mainWindow;
    mainWindow.setWindowState(Qt::WindowMaximized);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.setApplicationDescription("QFaktury");
    QCommandLineOption noSplashOption(
        "nosplash",
        QCoreApplication::translate("nosplash", "Blokuje widok splash przy starcie programu"));
    parser.addOptions({ noSplashOption });

    parser.process(application);

    // if argument for app in commandd line is --nosplash, uses no start window
    if (!parser.isSet(noSplashOption))
    {
        // sets start window during application load
        QSplashScreen splash(QPixmap(":/res/icons/splash.png"));

        splash.show();

        if (!splash.isVisible())
            qDebug() << "QSplash hadn't been visible";
        if (splash.isHidden())
            splash.show();

        application.processEvents();

        splash.showMessage("Ładowanie danych", Qt::AlignBaseline, Qt::white);

        // loads resources
        bool registered = QResource::registerResource("qfaktury.rcc");

        if (!registered)
        {
            QResource resource("qfaktury.rcc");
            if (!resource.isValid())
                qDebug() << "resources file " << resource.fileName() << " is not valid";
            if (resource.size() == 0)
                qDebug() << "resources file " << resource.fileName() << " is empty";
        }

        splash.showMessage("Ładowanie zasobów", Qt::AlignBaseline, Qt::white);

        splash.finish(&mainWindow);
    }

    // if last window is close, closes down application
    application.connect(&application, SIGNAL(lastWindowClosed()), &application, SLOT(quit()));
    mainWindow.connect(
        &mainWindow, &MainWindow::destroyed, &application, &QApplication::closeAllWindows);

    // sets icon, application name, organization name and style for app
    QIcon icon;
    icon.addPixmap(QPixmap(":/res/icons/qfaktury_48.png"), QIcon::Normal, QIcon::Off);
    application.setWindowIcon(icon);
    application.setApplicationName("QFaktury");
    application.setOrganizationName("https://github.com/juliagoda/qfaktury");
    application.setApplicationVersion(sett().getVersion(qAppName()));
    application.setStyle(sett().getStyle());

    if (parser.isSet("lang"))
        QLocale::setDefault(QLocale(parser.value("lang")));

    mainWindow.show();
    if (mainWindow.isActiveWindow())
        application.setActiveWindow(&mainWindow);
    if (!mainWindow.hasFocus())
        mainWindow.setFocus();

    //  // shows alert when main window is not shown properly
    application.alert(&mainWindow, 0);

    return application.exec();
}
