#include "filefinder.h"
#include "ui_filefinder.h"
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QGridLayout>
#include <QString>
#include <iostream>
#include <algorithm>
#include <QtGlobal>
#include <QDebug>
#include <QDateTimeEdit>
#include <QString>
 #include <qDebug>


using namespace std;
LoadingWorker::LoadingWorker():iter(0),isPaused(false),lowSizeFile(0),highSizeFile(2147483647),
 lowDateTimeFileCreated(QDate::currentDate().addYears(-30)),highDateTimeFileCreated(QDateTime::currentDateTime())
{

}

//
void LoadingWorker::findFiles(QDir dir)
{
    if(doStop)
        return;

    // http://qt-project.org/doc/qt-4.8/qstringlist.html#replaceInStrings
    // http://en.wikipedia.org/wiki/Regular_expression#Standards
    emit setDirPath(dir.absolutePath());


    QStringList tmp; //= dir.entryList( Filters, QDir::Files);
    QFileInfoList list = dir.entryInfoList(Filters, QDir::Files);
    for (int i = 0; i < list.size(); ++i)
    {
        QFileInfo fileInfo = list.at(i);
        QString temp;
        QString tempNum;
        temp.append(fileInfo.fileName());
        temp.append(" *\tSIZE= * ");
        tempNum.setNum(fileInfo.size());
        temp.append(tempNum);
        temp.append(" *BYTES  ");
        temp.append("*\tDATE= * ");
        temp.append(fileInfo.created().toString());
        if(fileInfo.size()>= lowSizeFile &&fileInfo.size()<= highSizeFile && fileInfo.created() >=lowDateTimeFileCreated && fileInfo.created() <=highDateTimeFileCreated )
        tmp.append(temp);
    }




    tmp.replaceInStrings(QRegExp("^(.*)"), dir.absolutePath()+QString("/\\1") );
    listFiles.append(tmp);
// cout <<" listSize = "<<listFiles.size();
    if (listFiles.size() > n)
    {
        iter = listFiles.size()/n;
        QStringList templist;
        cout <<iter<<" listSize = "<<listFiles.size();
        for(int i = 0; i < iter; ++i )
        {
            itrBegin = listFiles.begin();
            itrEnd = listFiles.begin()+n;
            copy(itrBegin,itrEnd, back_inserter(templist));
            listFiles.erase(itrBegin,itrEnd);
            emit nextWorkPart(templist);
            templist.clear();


        }
    }
    while(isPaused && !doStop)
    {
        QApplication::processEvents();
        QThread::msleep(10);
    }
    QStringList listDir = dir.entryList(QDir::Dirs);
    foreach (QString subdir, listDir)
    {
        if (subdir == "." || subdir == "..")
        {
            continue;
        }
        if( !doStop )
            findFiles(QDir(dir.absoluteFilePath(subdir)));


        else
        {

            emit workFinished();
            listFiles.clear();
        }

    }
    QApplication::processEvents();


}

void LoadingWorker::doWork(QString path, QString masks)
{
    emit workStarted();
    doStop=false;
    emit currentValue(0);
    QDir initial_dir(path);
    Filters = masks.split(" ");
    files.clear();
    findFiles(initial_dir);
    // Send reminder

    emit nextWorkPart(listFiles);
    listFiles.clear();
    //stopped=true;
    emit workFinished();
}
void LoadingWorker::stop()
{
    doStop = true;
    qDebug() << "Stopping";
}

void LoadingWorker::pause()
{
    isPaused = true;
    qDebug() << "\"Pausing\"";
}

void LoadingWorker::continueWork()
{
    isPaused = false;
    qDebug() << "Continuing";
}

void LoadingWorker::setLowSizeFile(int low)
{
 if(low <= highSizeFile )
 lowSizeFile = low;
 else
 {
  lowSizeFile = highSizeFile;
  highSizeFile = low;
 }
}
void LoadingWorker::setHighSizeFile(int high)
{
    if(high >= lowSizeFile )
    highSizeFile = high;
    else
    {
     highSizeFile = lowSizeFile;
     lowSizeFile = high;
    }
 //
}

void LoadingWorker::setLowDateCreatedFile(QDateTime low)
{
    if(low <= highDateTimeFileCreated )
    lowDateTimeFileCreated=low;
    else
    {
     lowDateTimeFileCreated = highDateTimeFileCreated;
     highDateTimeFileCreated = low;
    }
}
void LoadingWorker::setHighDateCreatedFile(QDateTime high)
{
    if(high >= lowDateTimeFileCreated)
    highDateTimeFileCreated=high;
    else
    {
     highDateTimeFileCreated=lowDateTimeFileCreated;
     lowDateTimeFileCreated = high;
    }
}


QString FileFinder::pausing_text(tr("&pause"));
QString FileFinder::continuing_text(tr("&continue"));

void FileFinder::closeEvent(QCloseEvent *event)
{
    if(thread.isRunning() )
    {
        QMetaObject::invokeMethod(worker, "stop", Qt::QueuedConnection );
        thread.quit();
    }
    thread.wait();
    event->accept();
}

void FileFinder::resetNotification()
{
    ui->pauseButton->setText( pausing_text );
    ui->pauseButton->setEnabled(false);
    ui->stopButton->setEnabled(false);
}

FileFinder::FileFinder() :
    ui(new Ui::FileFinder)
{
    ui->setupUi(this);
    ui->m_ptxtDir->setText(QDir::current().absolutePath());
    ui->m_ptxtMask->setEnabled(false);
    ui->dirPath->clear();
    ui->label_5->clear();
    ui->label_6->clear();
    ui->pauseButton->setText(continuing_text);
    ui->stopButton->setText("&stop");
    ui->lowSizeSpinBox->setEnabled(false);
    ui->lowSizeSpinBox->setValue(0);
    ui->lowSizeSpinBox->setRange(0, 2147483647);
    ui->lowSizeSpinBox->setSuffix("B");
    ui->highSizeSpinBox->setEnabled(false);
    ui->highSizeSpinBox->setRange(0, 2147483647);
    ui->highSizeSpinBox->setValue(2147483647);
    ui->highSizeSpinBox->setSuffix("B");
    ui->lowDateTimeEdit->setEnabled(false);
    ui->highDateTimeEdit->setEnabled(false);
    ui->highDateTimeEdit->setDateTime(QDateTime::currentDateTime());
    worker = new LoadingWorker();
    worker->moveToThread(&thread);
    resetNotification();
    doc = new QDomDocument("findFiles");
    //doc.createAttribute("findFiles");
    QObject::connect(&thread, &QThread::finished, worker, &QObject::deleteLater );

    QObject::connect(this, SIGNAL(sendWork(QString, QString)),
                     worker, SLOT(doWork(QString, QString)));

    QObject::connect(worker, SIGNAL(workFinished()),
                     this, SLOT(onWorkFinished()) );

    QObject::connect(worker, SIGNAL(nextWorkPart(const QStringList&)),
                     this, SLOT(onNextWorkPart(const QStringList&) ) );
    QObject::connect(worker, SIGNAL(setDirPath(const QString)),this, SLOT(initDirPath(const QString) ) );
    QObject::connect(this, SIGNAL(currentValue(int)),ui->lcd,SLOT(display(int)));
    QObject::connect(worker, SIGNAL(currentValue(int)),ui->lcd,SLOT(display(int)));
    QObject::connect(this, SIGNAL(stop()),worker,SLOT(stop()));
    QObject::connect(worker, SIGNAL(workStarted()), this, SLOT(workerStarted()));
    QObject::connect(ui->lowSizeSpinBox, SIGNAL(valueChanged ( int)),worker, SLOT(setLowSizeFile(int)));
    QObject::connect(ui->highSizeSpinBox, SIGNAL(valueChanged ( int)),worker, SLOT(setHighSizeFile(int)));
    QObject::connect(ui->lowDateTimeEdit, SIGNAL(dateTimeChanged ( QDateTime)),worker, SLOT(setLowDateCreatedFile(QDateTime)));
    QObject::connect(ui->highDateTimeEdit, SIGNAL(dateTimeChanged ( QDateTime)),worker, SLOT(setHighDateCreatedFile(QDateTime)));

    thread.start();

}

FileFinder::~FileFinder()
{
    thread.quit();
    thread.wait();
    delete doc;
    delete ui;
}
void FileFinder::workerStarted()
{
    ui->m_ptxtDir->setEnabled(false);
    ui->pauseButton->setText(pausing_text);
    ui->pauseButton->setEnabled(true);
    ui->stopButton->setEnabled(true);
}




void FileFinder::on_pcmdDir_clicked()
{
    QString str = QFileDialog::getExistingDirectory(0,
                  "Select a Directory",
                  ui->m_ptxtDir->text()
                                                   );
    if (!str.isEmpty())
    {
        ui->m_ptxtDir->setText(str);
    }
}

void FileFinder::on_pcmdFind_clicked()
{
    count = 0;
    ui->pcmdFind->setEnabled(false);
    ui->m_ptxtDir->setEnabled(false);
    ui->m_ptxtResult->clear();


    domElement = doc->createElement("findFiles");
    doc->appendChild(domElement);
    emit sendWork( ui->m_ptxtDir->text(),ui-> m_ptxtMask->text() );

}

void FileFinder::onNextWorkPart(const QStringList &strlist)
{

    foreach (QString subdir, strlist)
    {
      storeToXML(subdir);
      subdir.remove("*");
        ui->m_ptxtResult->append(subdir);
        count++;
        //QDomElement cont = contact(*doc, subdir);

        //domElement.appendChild(cont);
        emit currentValue(count);
    }
    if (count ==0)
    {
        QMessageBox quitMsg;
             quitMsg.setWindowTitle("Files have not found!");
             quitMsg.setText("No one file with adjusted criterion of the search has not found!\n");
             if (quitMsg.exec() == QMessageBox::Ok)
                 return;
    }
    cout <<" listSize = "<<strlist.size();

}

void FileFinder::storeToXML(const QString str)
{
    QString dat;
 QStringList fileList = str.split("*");
 for(int i = 5; i < fileList.size(); ++i )
     dat.append(fileList[i]+" ");
 /*qDebug()<<fileList[0];
 qDebug()<<fileList[2];
 qDebug()<<dat;*/
 QDomElement cont = contact(*doc, fileList[0],fileList[2],dat);
 domElement.appendChild(cont);
}

void FileFinder::onWorkFinished()
{
    ui->m_ptxtDir->setEnabled(true);
    ui->pcmdFind->setEnabled(true);
    ui->pauseButton->setEnabled(false);
    ui->stopButton->setEnabled(false);
    QFile file("filelist.xml");
        if(file.open(QIODevice::WriteOnly)) {
            QTextStream(&file) << doc->toString();
            file.close();
        }
}

void FileFinder::on_stopButton_clicked()
{
    emit stop();
}
void FileFinder::initDirPath(const QString dirPath)
{
    QString path("Current dir: ");
    path.append(dirPath);
    ui->dirPath->setText(path);
}





void FileFinder::on_pauseButton_clicked()
{
    if(ui->pauseButton->text()==pausing_text )
    {
        QMetaObject::invokeMethod(worker, "pause", Qt::QueuedConnection );
        ui->pauseButton->setText( continuing_text );
    }
    else
    {
        QMetaObject::invokeMethod(worker, "continueWork", Qt::QueuedConnection );
        ui->pauseButton->setText( pausing_text );
    }


}


void FileFinder::on_checkBoxFindSize_clicked()
{
    if (ui->checkBoxFindSize->isChecked())
    {
      ui->lowSizeSpinBox->setEnabled(true);
      ui->highSizeSpinBox->setEnabled(true);
     } else
    {
      ui->lowSizeSpinBox->setEnabled(false);
      ui->highSizeSpinBox->setEnabled(false);
    }
}

void FileFinder::on_checkBoxFindDate_clicked()
{
    if (ui->checkBoxFindDate->isChecked())
    {
      ui->lowDateTimeEdit->setEnabled(true);
      ui->highDateTimeEdit->setEnabled(true);
     } else
    {
      ui->lowDateTimeEdit->setEnabled(false);
      ui->highDateTimeEdit->setEnabled(false);
    }

}

void FileFinder::on_checkBoxFindType_clicked()
{
    if (ui->checkBoxFindType->isChecked())
     ui->label_5->setText("<B><FONT COLOR=RED>Enter the names through the space.</FONT></B>");
    else
    {
    ui->label_5->clear();
    ui->m_ptxtMask->clear();
    }
    if (ui->checkBoxFindName->isChecked() || ui->checkBoxFindType->isChecked())
    {
      ui->m_ptxtMask->setEnabled(true);
    } else
  ui->m_ptxtMask->setEnabled(false);

}

void FileFinder::on_checkBoxFindName_clicked()
{
    if (ui->checkBoxFindType->isChecked() || ui->checkBoxFindName->isChecked())
    {
      ui->m_ptxtMask->setEnabled(true);
    } else
  ui->m_ptxtMask->setEnabled(false);

     if (ui->checkBoxFindName->isChecked())
     {
         QString str("*.");
         ui->label_6->setText("<B><FONT COLOR=RED>Enter the types through the space.</FONT></B>");
         ui->m_ptxtMask->setText(str);
      } else
       {
        ui->label_6->clear();
        ui->m_ptxtMask->clear();
     }

}
//*.cpp *.h
