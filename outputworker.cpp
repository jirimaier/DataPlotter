#include "outputworker.h"

OutputWorker::OutputWorker(QObject *parent) : QObject(parent) {
  beginMark = QString("<font color=darkgreen>%1</font>").arg(QString(CMD_BEGIN).replace('<', "&lt;").replace('>', "&gt;"));
  endMark = QString("<font color=darkgreen>%1</font>").arg(QString(CMD_END).replace('<', "&lt;").replace('>', "&gt;"));
  timeoutMark = QString("<font color=darkgreen>%1</font>").arg(QString(TIMEOUT_SYMBOL).replace('<', "&lt;").replace('>', "&gt;"));
  qDebug() << "OutputWorker created from " << QThread::currentThreadId();
}

OutputWorker::~OutputWorker() { qDebug() << "OutputWorker deleted from " << QThread::currentThreadId(); }

void OutputWorker::input(QByteArray message, int type) {
  if (type == DataLineType::debugMessage)
    emit output(QString("<font color=red>%1</font>").arg(QString(message)));
  else if (type == DataLineType::command) {
    if (outputLevel == OutputLevel::low)
      emit output(QString(message));
    if (outputLevel == OutputLevel::high)
      emit output(beginMark + QString(message) + endMark);
  } else if (type == DataLineType::dataEnded || type == DataLineType::dataTimeouted || type == DataLineType::dataImplicitEnded) {
    bool wasShortened = false;
    if (outputLevel == OutputLevel::low) {
      if (message.length() > OUTPUT_SHORT_LINE_MAX_LENGTH) {
        message = message.left(OUTPUT_SHORT_LINE_MAX_LENGTH);
        wasShortened = true;
      }
    }
    if (outputLevel == OutputLevel::low || outputLevel == OutputLevel::high) {
      QString stringMessage = QString(message);
      if (stringMessage.length() == message.length()) {
        stringMessage.replace(QChar('\r'), "<font color=navy>[CR]</font>");
        stringMessage.replace(QChar('\n'), "<font color=navy>[LF]</font>");
        stringMessage.replace(QChar('\t'), "<font color=navy>[TAB]</font>");
        stringMessage.replace(QChar(27), "<font color=navy>[ESC]</font>");
      } else {

// Oddělení bajtů mezerami nefunguje v starším Qt (Win XP)
#if QT_VERSION >= 0x050900
        stringMessage = message.toHex(' ');
#else
        stringMessage.clear();
        foreach (unsigned char b, message)
          stringMessage.append(QString::number(b, 16) + " ");
        stringMessage = stringMessage.trimmed();
#endif
        stringMessage = QString("<font color=navy>%1</font>").arg(stringMessage);
      }
      if (wasShortened)
        stringMessage.append(QString("<font color=orange>...</font>"));
      if (outputLevel == OutputLevel::high) {
        if (type == DataLineType::dataTimeouted)
          stringMessage.append(timeoutMark);
        if (type == DataLineType::dataEnded)
          stringMessage.append(endMark);
      }
      emit output(stringMessage);
    }
  }
}
