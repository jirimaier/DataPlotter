#ifndef FILESENDER_H
#define FILESENDER_H

#include <QObject>

class FileSender : public QObject
{
    Q_OBJECT
public:
    explicit FileSender(QObject *parent = nullptr);

private:
    QByteArray buffer;
    int m_count;
    bool m_hasEnding;
    bool m_fillEnd;
    char m_ending;

public slots:
    void newFile(const QByteArray &file);
    QByteArray nextPart(QByteArray countText, QByteArray args);

signals:
    void transmit(QByteArray message);

};

#endif // FILESENDER_H
