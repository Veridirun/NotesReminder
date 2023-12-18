#include "helpnotes.h"
#include "ui_helpnotes.h"

HelpNotes::HelpNotes(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HelpNotes)
{
    ui->setupUi(this);
    loadHtmlFile("B:/ProgramData/QTProjects/ToDo-Reminder/Help/help.html");
    connect(ui->textBrowser, &QTextBrowser::anchorClicked, this, &HelpNotes::handleAnchorClicked);
}

void HelpNotes::loadHtmlFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qWarning("Файл справки не найден!");
    }
    ui->textBrowser->setHtml(QString::fromUtf8(file.readAll()));
}

void HelpNotes::handleAnchorClicked(const QUrl &url)
{
    // При щелчке на ссылке обрабатываем URL и загружаем соответствующий файл
    QString filePath = url.toString();
    loadHtmlFile(filePath);
}

HelpNotes::~HelpNotes()
{
    delete ui;
}
