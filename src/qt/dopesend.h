#ifndef DOPESEND_H
#define DOPESEND_H

#include <QObject>
#include "httpsocket.h"

class dopesend : public QObject
{
    Q_OBJECT
public:
    explicit dopesend(QObject *parent = 0);
    QString fromAddress;
    QString destinationAddress;
    QString amount;
    QString getCloakedAddress(); //returns the bited address assuming object variables set correctly.
    bool useProxy;
    QString proxyAddress;
    int proxyPort;
signals:

public slots:

private:
    httpsocket *socket;
};

#endif // DOPESEND_H
