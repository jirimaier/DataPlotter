#include "buffer.h"
#include "enums.h"
#include <QByteArrayMatcher>
#include <QDebug>
#include <QMainWindow>
#include <QObject>

void Buffer::timeout() {
  list.append(QPair<bool, QByteArray>(false, buffer));
  buffer.clear();
  emit newEntry();
}

Buffer::Buffer() {
  head.setPattern("<cmd>");
  tail.setPattern("<\\cmd>");
  timeoutTimer = new QTimer(this);
  connect(timeoutTimer, SIGNAL(timeout()), this, SLOT(timeout()));
  timeoutTimer->setSingleShot(true);
}

void Buffer::add(QByteArray data) {
  buffer.append(data);
  while (true) {
    timeoutTimer->stop();
    int begin = head.indexIn(buffer);
    int end = tail.indexIn(buffer);
    if (begin == -1 && end == -1) {
      timeoutTimer->start(BUFFER_LINE_TIMEOUT);
      break;
    }
    if (end != -1 && end < begin) {
      buffer.remove(0, end + tail.pattern().length());
      continue;
    }
    if (begin > 0) {
      list.append(QPair<bool, QByteArray>(false, buffer.left(begin)));
      emit newEntry();
      buffer.remove(0, begin);
    }
    if (begin == 0 && end == -1)
      break;
    if (begin == 0 && end > begin) {
      list.append(QPair<bool, QByteArray>(true, buffer.mid(begin + head.pattern().length(), end - begin - head.pattern().length())));
      emit newEntry();
      buffer.remove(0, end + tail.pattern().length());
      continue;
    }
  }
}

QPair<bool, QByteArray> Buffer::next() {
  QPair<bool, QByteArray> result;
  if (!list.isEmpty()) {
    result = list.at(0);
    list.removeFirst();
  }
  return result;
}
