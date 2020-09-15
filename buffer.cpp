#include "buffer.h"

void Buffer::timeout() {
  queue.enqueue(QPair<bool, QByteArray>(false, buffer));
  buffer.clear();
  emit newEntry();
}

Buffer::Buffer() {
  head.setPattern(CMD_BEGIN);
  tail.setPattern(CMD_END);
  timeoutTimer = new QTimer(this);
  connect(timeoutTimer, SIGNAL(timeout()), this, SLOT(timeout()));
  timeoutTimer->setSingleShot(true);
}

void Buffer::add(QByteArray data) {
  buffer.append(data);
  // Timeout po 10 cyklech
  for (quint8 i = 0; i < 10; i++) {
    timeoutTimer->stop();
    int begin = head.indexIn(buffer);
    int end = tail.indexIn(buffer);
    if (begin == -1 && end == -1) {
      timeoutTimer->start(BUFFER_LINE_TIMEOUT);
      return;
    }
    if (end != -1 && (end < begin || begin == -1)) {
      buffer.remove(0, end + CMD_END_LENGHT);
      continue;
    }
    if (begin > 0) {
      queue.enqueue(QPair<bool, QByteArray>(false, buffer.left(begin)));
      emit newEntry();
      buffer.remove(0, begin);
      continue;
    }
    if (begin == 0 && end == -1)
      return;
    if (begin == 0 && end > begin) {
      queue.enqueue(QPair<bool, QByteArray>(true, buffer.mid(begin + head.pattern().length(), end - begin - head.pattern().length())));
      emit newEntry();
      buffer.remove(0, end + CMD_END_LENGHT);
      continue;
    }
  }
  timeout();
}

QPair<bool, QByteArray> Buffer::next() {
  if (!queue.isEmpty())
    return queue.dequeue();
  else
    return QPair<bool, QByteArray>(false, "");
}
