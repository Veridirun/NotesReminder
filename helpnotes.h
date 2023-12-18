#ifndef HELPNOTES_H
#define HELPNOTES_H

#include <QWidget>
#include <QFile>

namespace Ui {
class HelpNotes;
}

class HelpNotes : public QWidget
{
    Q_OBJECT

public:
    explicit HelpNotes(QWidget *parent = nullptr);
    ~HelpNotes();
private slots:
    void handleAnchorClicked(const QUrl &url);
private:
    Ui::HelpNotes *ui;
    void loadHtmlFile(const QString &filePath);
};

#endif // HELPNOTES_H
