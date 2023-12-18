#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSql>
#include <QDebug>
#include <QTableView>
#include <QTableWidget>
#include <QListWidget>
#include <QTextEdit>
#include <QSqlTableModel>
#include <QAbstractButton>
#include <QMessageBox>
#include <QMediaPlayer>
#include <QUrl>
#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QTextDocumentWriter>
#include <QSettings>
#include <vector>

#include "helpnotes.h"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

private slots:
    void saveChangesToDatabase();
    void addNewRecord();
    void updateListWidget();
    void removeRecordFromDatabase();
    void toggleDate();

public:
    void saveSettings();
    void loadSettings();
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void checkDateTime();
    void playNotificationSound();
    void rememberSearched();
    void readSearched();
    void loadFromFile();
    void saveToFile();
    void About();

    Ui::MainWindow *ui;
    QSqlTableModel *model;
    QString searchPrompt;
    int lastNoteNumber;
    HelpNotes *help;
    QSettings *settings;
};

#endif // MAINWINDOW_H
