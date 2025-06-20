#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("Leditor");
    QCoreApplication::setApplicationName("Leditor");
    QCoreApplication::setApplicationVersion("1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::applicationName());
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("file", "The file to open.");
    parser.process(app);

    MainWindow mainWin;
    
    // If a file was specified, open it
    if (!parser.positionalArguments().isEmpty()) {
        mainWin.openFile(parser.positionalArguments().first());
    }
    
    mainWin.show();

    return app.exec();
} 