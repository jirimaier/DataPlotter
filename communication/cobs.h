#ifndef COBS_H
#define COBS_H

#include <QByteArray>

struct COBS
{
public:
    static QByteArray encode(QByteArray data);
    static QByteArray decode(QByteArray data);
private:
    COBS(){}
};

#endif // COBS_H
