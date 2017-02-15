#ifndef FUNCTIONSXML_H
#define FUNCTIONSXML_H
#include <QtXml>
#include <QString>



QDomElement contact(QDomDocument &domDoc,
                         const QString& strFile, const QString& fileSize,const QString& fileDate);
#endif // FUNCTIONSXML_H
