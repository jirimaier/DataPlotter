#ifndef MYFRAMEWITHRESIZESIGNAL_H
#define MYFRAMEWITHRESIZESIGNAL_H

#include <QWidget>
#include <QFrame>
#include <QResizeEvent>

class MyFrameWithResizeSignal : public QFrame
{
    Q_OBJECT
public:
    explicit MyFrameWithResizeSignal(QWidget *parent = nullptr);

signals:
    void resized(QSize size);

    // QWidget interface
protected:
    void resizeEvent(QResizeEvent *event);
};

#endif // MYFRAMEWITHRESIZESIGNAL_H
