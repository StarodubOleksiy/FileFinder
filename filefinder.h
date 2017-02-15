#ifndef FILEFINDER_H
#define FILEFINDER_H

#include <QMainWindow>
#include <QWidget>
#include <QLineEdit>
#include <QDir>
#include <QTextEdit>
#include <QFileDialog>
#include <QThread>
#include <QProgressBar>
#include <QTimer>
#include <QApplication>
#include <QLCDNumber>
#include <QWidget>
#include <QResizeEvent>
#include <QDateTime>
#include "functionsXML.h"




class LoadingWorker : public QObject
{
    Q_OBJECT

    QStringList files;
    QStringList listFiles;
    QStringList Filters;
    QObject* m_pobjReceiver;
    QStringList::iterator itrBegin;
    QStringList::iterator itrEnd;
    int iter;
    int lastiter;
    bool doStop;
    bool isPaused;
    int lowSizeFile;
    int highSizeFile;
    static const int n = 10;
    QDateTime lowDateTimeFileCreated;
    QDateTime highDateTimeFileCreated;

public:
        LoadingWorker();/*:iter(0),isPaused(false)
            {

            }*/
        bool getPaused()
        {
         return isPaused;
        }
    void findFiles(QDir dir);
signals:
     void workStarted();
    //!!!! Object LoadingWorker should be alive while we using the result!
    void nextWorkPart(const QStringList &strlist);
    void workFinished();
  void setDirPath(const QString dirPath);
  void currentValue(int);



public slots:
    void setLowSizeFile(int low);
    void setHighSizeFile(int high);
    void setLowDateCreatedFile(QDateTime low);
    void setHighDateCreatedFile(QDateTime high);
    void doWork(QString path, QString masks);
    void stop();
    void pause();
    void continueWork();



};


namespace Ui {
class FileFinder;
}

class FileFinder : public  QMainWindow
{
    Q_OBJECT

public:
    explicit FileFinder(/*QWidget *parent = 0*/);
    void storeToXML(const QString str);
    ~FileFinder();
protected:
    void closeEvent(QCloseEvent *event);

private:
    Ui::FileFinder *ui;
    int count;
    LoadingWorker *worker;
    QThread thread;
    //QString setPause,resume;
   // LoadingWorker *worker;
    static QString pausing_text;
    static QString continuing_text;
    QDomElement  domElement;
    QDomDocument *doc;
    void resetNotification();
public slots:
 void onNextWorkPart(const QStringList &strlist);
 void onWorkFinished();
 void initDirPath(const QString dirPath);
 void workerStarted();
 signals:
 void sendWork(QString path, QString masks);
 void currentValue(int);
 void stop();
 void pause();
private slots:
 void on_pcmdDir_clicked();
 void on_pcmdFind_clicked();
 void on_stopButton_clicked();
 void on_pauseButton_clicked();
 void on_checkBoxFindSize_clicked();
 void on_checkBoxFindDate_clicked();
 void on_checkBoxFindType_clicked();
 void on_checkBoxFindName_clicked();
};

#endif // FILEFINDER_H
