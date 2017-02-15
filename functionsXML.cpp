#include "functionsXML.h"

QDomElement makeElement(      QDomDocument domDoc,
const QString&      strName,
const QString&      strAttr = QString::null,
const QString&      strText = QString::null)
{
    QDomElement domElement = domDoc.createElement(strName);

    if (!strAttr.isEmpty()) {
        QDomAttr domAttr = domDoc.createAttribute("number");
        domAttr.setValue(strAttr);
        domElement.setAttributeNode(domAttr);
    }

    if (!strText.isEmpty()) {
        QDomText domText = domDoc.createTextNode(strText);
        domElement.appendChild(domText);
    }
    return domElement;
}

// ----------------------------------------------------------------------
QDomElement contact(      QDomDocument &domDoc,
                    const QString&      strFile, const QString& fileSize,const QString& fileDate)
{
    static int nNumber = 1;

    QDomElement domElement = makeElement(domDoc,
                                         "file",
                                         QString().setNum(nNumber)
                                        );
    domElement.appendChild(makeElement(domDoc, "path", "", strFile));
    domElement.appendChild(makeElement(domDoc,"size(bytes)", "", fileSize));
    domElement.appendChild(makeElement(domDoc,"data", "", fileDate));


    nNumber++;

    return domElement;
}
