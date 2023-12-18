#include "mainwindow.h"
#include <QApplication>
#include <QListWidget>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QApplication::setOrganizationName("KGaponov");
    QApplication::setApplicationName("Notes Notifier");

    QFile styleFile(":/styles.css");
    styleFile.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(styleFile.readAll());
    a.setStyleSheet(styleSheet);

    MainWindow w;
    w.show();

    return a.exec();
}
