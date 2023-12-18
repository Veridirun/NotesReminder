#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QAbstractItemView>

void MainWindow::About(){
    QMessageBox msg;
    msg.setWindowTitle("О программе");
    msg.setText("Редактор заметок с уведомлениями\nВыполнил студент группы ИП-113 Гапонов Кирилл");
    msg.exec();
}

void MainWindow::loadFromFile(){
    if(ui->listWidget->currentItem()){
        QString filename = QFileDialog::getOpenFileName(nullptr, "Загрузить файл", QDir::currentPath(), "*.txt");
        QFile file(filename);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);

            ui->noteHeader->setPlainText(in.readLine());
            ui->noteText->setPlainText(in.readAll());
            file.close();
        } else {
            qDebug() << "Не удалось открыть файл для чтения.";
        }
    }
}

void MainWindow::saveToFile(){
    if(ui->listWidget->currentItem()){
        QString filename = QFileDialog::getOpenFileName(nullptr, "Сохранить файл", QDir::currentPath(), "*.txt");
        QFile file(filename);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);

            out<<ui->noteHeader->toPlainText()<<'\n';
            out<<ui->noteText->toPlainText();

            file.close();
        } else {
            qDebug() << "Не удалось открыть файл для записи.";
        }
    }
}

void MainWindow::rememberSearched(){
    QString filePath = QCoreApplication::applicationDirPath() + "/lastSearched.txt";
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << ui->noteSearch->toPlainText();
            file.close();
        } else {
            qDebug() << "Ошибка при открытии файла:" << file.errorString();
        }
}

void MainWindow::readSearched(){
    QString filePath = QCoreApplication::applicationDirPath() + "/lastSearched.txt";
    QFile file(filePath);

       if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
           QTextStream in(&file);

           // Чтение первой строки
           searchPrompt = in.readLine();
           ui->noteSearch->setPlainText(searchPrompt);

           updateListWidget();

           file.close();
       } else {
           qDebug() << "Не удалось открыть файл для чтения.";
       }

}

void MainWindow::playNotificationSound() {
    QMediaPlayer *player = new QMediaPlayer;
    QUrl soundUrl = QUrl("qrc:/audio/alarm1.mp3");
    player->setMedia(soundUrl);
    player->play();
}

void MainWindow::checkDateTime() {

    QDateTime currentDateTime = QDateTime::currentDateTime();
    QDateTime targetDateTime;

    model->setTable("Notes");
    model->select();
    for(int row = 0; row < model->rowCount(); ++row){
        bool isActive = model->record(row).value("isActive").toBool();
        QString header = model->record(row).value("Header").toString();
        QString text = model->record(row).value("Text").toString();
        if(isActive){
            QString dateString = model->record(row).value("Date").toString();
            targetDateTime = QDateTime::fromString(dateString, "yyyy-MM-dd HH:mm:ss.zzz");
        } else{
            continue;
        }

        if (currentDateTime >= targetDateTime) {

            playNotificationSound();
            QMessageBox msgBox;
            msgBox.setIcon(QMessageBox::Information);
            msgBox.setWindowTitle(header);
            msgBox.setMinimumSize(250, 0);
            msgBox.setText(text);
            msgBox.exec();

            //QSqlRecord record = model->record();
            model->setData(model->index(row, model->fieldIndex("isActive")), 0);
            if (model->submitAll()) {
                qDebug() << "Новая запись добавлена в базу данных.";
            } else {
                qDebug() << "Ошибка при добавлении новой записи:" << model->lastError().text();
            }
        }
    }
}

void MainWindow::toggleDate(){
    if(ui->checkBox->isChecked()){
        ui->dateTimeEdit->setEnabled(true);
    } else {
        ui->dateTimeEdit->setEnabled(false);
    }
}

void MainWindow::updateListWidget(){
    ui->listWidget->clear();
    model->setTable("Notes");
    model->select();
    for(int row = 0; row < model->rowCount(); ++row){
        searchPrompt = ui->noteSearch->toPlainText();

        QString header = model->record(row).value("Header").toString();
        if(header.length()>18){
            header.truncate(15);
            header += "...";
        }

        if(!header.contains(searchPrompt,Qt::CaseInsensitive)){
            continue;
        }

        int number = model->record(row).value("Number").toInt();
        QListWidgetItem *item = new QListWidgetItem(header);
        item->setData(Qt::UserRole, number);
        ui->listWidget->addItem(item);
    }
    ui->noteText->setEnabled(false);
    ui->noteHeader->setEnabled(false);
    ui->checkBox->setEnabled(false);
    ui->dateTimeEdit->setEnabled(false);
    ui->saveToFileMenu->setEnabled(false);
    ui->loadFromFileMenu->setEnabled(false);
}

void MainWindow::addNewRecord(){
    QSqlRecord record = model->record();
    QString header = "Новая заметка";
    QString text = "";
    //QDateTime *notificationDate = new QDateTime();
    QDateTime notificationDate;
    notificationDate = QDateTime::fromString("2000-01-01 00:00:00.000", "yyyy-MM-dd HH:mm:ss.zzz");
    record.setValue("Header", header);
    record.setValue("Text", text);
    record.setValue("Number", ++lastNoteNumber);
    record.setValue("isActive", false);
    record.setValue("Date", notificationDate.currentDateTime());
    model->insertRecord(-1,record);
    if (model->submitAll()) {
        qDebug() << "Новая запись добавлена в базу данных.";
        updateListWidget();  // Обновить QListWidget
    } else {
        qDebug() << "Ошибка при добавлении новой записи:" << model->lastError().text();
    }
}

void MainWindow::removeRecordFromDatabase(){
    if (ui->listWidget->currentItem()) {
        int number = ui->listWidget->currentItem()->data(Qt::UserRole).value<int>();
        QSqlQuery query;
        query.prepare("DELETE FROM Notes WHERE Number = :number");
        query.bindValue(":number", number);

        if (query.exec()) {
            qDebug() << "Запись успешно удалена из базы данных.";
            updateListWidget();
        } else {
            qDebug() << "Ошибка при удалении записи из базы данных:" << query.lastError().text();
        }
    }
}

void MainWindow::saveChangesToDatabase() {
    if (ui->listWidget->currentItem()) {
        QString header = ui->noteHeader->toPlainText();
        QString text = ui->noteText->toPlainText();
        QDateTime dateTime = ui->dateTimeEdit->dateTime();
        QString formattedDateTime = dateTime.toString("yyyy-MM-dd HH:mm:ss.zzz");
        bool isActive = ui->checkBox->isChecked();
        if(isActive){
            if(dateTime < QDateTime::currentDateTime()){
                QMessageBox msgBox;
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.setWindowTitle("Ошибка!");
                msgBox.setMinimumSize(250, 0);
                msgBox.setText("Нельзя поставить уведомление раньше, чем текущие дата и время.\n");
                msgBox.exec();
                isActive = false;
                ui->checkBox->setChecked(false);
                //return;
            }
        }

        // Обновление записи в базе данных
        int number = ui->listWidget->currentItem()->data(Qt::UserRole).value<int>();
        model->setFilter(QString("Number = %1").arg(number));
        model->select();
        if (model->rowCount() == 1) {
            int row = 0;

            model->setData(model->index(row, model->fieldIndex("Header")), header);
            model->setData(model->index(row, model->fieldIndex("Text")), text);
            model->setData(model->index(row, model->fieldIndex("Date")), formattedDateTime);
            if(isActive){
                model->setData(model->index(row, model->fieldIndex("isActive")), 1);
            } else {
                model->setData(model->index(row, model->fieldIndex("isActive")), 0);
            }

            if (!model->submitAll()) {
                qDebug() << "Failed to submit changes to the database.";
            }
        } else {
            qDebug() << "Record not found for Number:" << number;
        }
        //Обновление списка заметок
        updateListWidget();
    }
}

void MainWindow::saveSettings(){
    settings->beginGroup("form");
    settings->setValue("title", windowTitle());
    settings->setValue("windowSize", size());
    settings->setValue("position", pos());
    settings->endGroup();
}
void MainWindow::loadSettings(){
    settings->beginGroup("form");
    setWindowTitle(settings->value("title", "Notes Notifier").toString());
    resize(settings->value("windowSize", QSize(800, 600)).toSize());
    move(settings->value("position", QPoint(100, 100)).toPoint());
    settings->endGroup();
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    settings = new QSettings(QCoreApplication::applicationDirPath() + "/settings.ini", QSettings::IniFormat, this);
    loadSettings();

    lastNoteNumber=0;

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("B:/ProgramData/QTProjects/ToDo-Reminder/DBNotes.db");
    if(db.open()){
        qDebug() << "Success opening DB";
    } else {
        qDebug() << "Failed opening DB";
    }
    model = new QSqlTableModel(this);
    model->setTable("Notes");
    model->select();

    //Считываем прошлый поиск
    readSearched();
    //Проверяем какие заметки уже просрочены
    for(int row = 0; row < model->rowCount(); ++row){
        QString header = model->record(row).value("Header").toString();
        int currentNoteNumber = model->record(row).value("Number").toInt();
        bool isActive = model->record(row).value("isActive").toBool();
        QString dateString = model->record(row).value("Date").toString();
        QDateTime dateTime = QDateTime::fromString(dateString, "yyyy-MM-dd HH:mm:ss.zzz");

        //Выставляем самый большой ID заметки
        if(lastNoteNumber<currentNoteNumber)
            lastNoteNumber = currentNoteNumber;

        if(isActive){
            if(dateTime < QDateTime::currentDateTime()){
                qDebug()<<"меняем isActive для заметки с названием "<<header;
                model->setData(model->index(row, model->fieldIndex("isActive")), 0);
                if (model->submitAll()) {
                    qDebug() << "Изменения успешно сохранены в базе данных.";
                } else {
                    qDebug() << "Ошибка при сохранении изменений в базе данных:" << model->lastError().text();
                }
            }
        }

//        QListWidgetItem *item = new QListWidgetItem(header);
//        item->setData(Qt::UserRole, number);
//        ui->listWidget->addItem(item);
    }
    updateListWidget();

    //Подставление данных записи в виджеты при выборе заметки
    connect(ui->listWidget, &QListWidget::currentItemChanged, [=](QListWidgetItem *current){
        //qDebug()<<"ItemChanged";
        if(current){

            int number = current->data(Qt::UserRole).value<int>();
            model->setFilter(QString("Number = %1").arg(number));
            model->select();
            if (model->rowCount() == 1) {
                int row = 0;
                QString header = model->record(row).value("Header").toString();
                QString text = model->record(row).value("Text").toString();
                QString dateString = model->record(row).value("Date").toString();
                bool isActive = model->record(row).value("isActive").toBool();
                QDateTime dateTime;
                if(!dateString.isEmpty()){
                    dateTime = QDateTime::fromString(dateString, "yyyy-MM-dd HH:mm:ss.zzz");
                    ui->dateTimeEdit->setDateTime(dateTime);
                } else {
                    dateTime = QDateTime::fromString("2000-01-01 00:00:00.000", "yyyy-MM-dd HH:mm:ss.zzz");
                    ui->dateTimeEdit->setDateTime(dateTime);
                }

                ui->checkBox->setChecked(isActive);
                toggleDate();
                ui->noteText->setPlainText(text);
                ui->noteHeader->setPlainText(header);
                ui->noteText->setEnabled(true);
                ui->noteHeader->setEnabled(true);
                ui->checkBox->setEnabled(true);
                ui->saveToFileMenu->setEnabled(true);
                ui->loadFromFileMenu->setEnabled(true);

            } else {
                qDebug() << "Record not found for Number:" << number;
            }
        }
    });

    connect(ui->saveButton, &QPushButton::clicked, this, &MainWindow::saveChangesToDatabase);
    connect(ui->addButton, &QPushButton::clicked, this, &MainWindow::addNewRecord);
    connect(ui->noteSearch, &QTextEdit::textChanged, this, &MainWindow::updateListWidget);
    connect(ui->deleteButton, &QPushButton::clicked, this, &MainWindow::removeRecordFromDatabase);
    connect(ui->checkBox, &QCheckBox::toggled, this, &MainWindow::toggleDate);
    connect(ui->loadFromFileMenu, &QAction::triggered,this, &MainWindow::loadFromFile);
    connect(ui->saveToFileMenu, &QAction::triggered,this, &MainWindow::saveToFile);

    help = new HelpNotes();
    connect(ui->referenceMenu, SIGNAL(triggered()), help, SLOT(show()));

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::checkDateTime);
    timer->start(1000);
    updateListWidget();
}

MainWindow::~MainWindow()
{
    saveSettings();
    rememberSearched();
    saveChangesToDatabase();
    delete ui;
}
