#include "myframewithresizesignal.h"

MyFrameWithResizeSignal::MyFrameWithResizeSignal(QWidget *parent)
    : QFrame{parent}
{

}

void MyFrameWithResizeSignal::resizeEvent(QResizeEvent *event)
{
    emit resized(event->size());
}
