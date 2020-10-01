#include "receivedoutputhandler.h"

ReceivedOutputHandler::ReceivedOutputHandler(QObject *parent) : QObject(parent) {}

void ReceivedOutputHandler::input(QByteArray message) {
  if (outputLevel == 0)
    return;
  if (outputLevel == 1) {
    emit output(message.left(30));
  }
  if (outputLevel == 2) {
    QString stringMessage = QString(message);
    if (stringMessage.length() == message.length()) {
      stringMessage.replace(QChar('\r'), "<font color=navy>[CR]</font>");
      stringMessage.replace(QChar('\n'), "<font color=navy>[LF]</font>");
      stringMessage.replace(QChar('\t'), "<font color=navy>[TAB]</font>");
      stringMessage.replace(QChar(27), "<font color=navy>[ESC]</font>");
    } else {
      QString suffix = "";
      if (message.endsWith(TIMEOUT_SYMBOL)) {
        message = message.left(message.length() - TIMEOUT_SYMBOL_LENGTH);
        suffix = TIMEOUT_SYMBOL;
      }
      if (message.endsWith(CMD_END)) {
        message = message.left(message.length() - CMD_END_LENGTH);
        suffix = CMD_END;
      }

// Oddělení bajtů mezerami nefunguje v starším Qt (Win XP)
#if QT_VERSION >= 0x050900
      stringMessage = message.toHex(' ') + suffix;
#else
      stringMessage.clear();
      foreach (unsigned char b, message)
        stringMessage.append(QString::number(b, 16) + " ");
      stringMessage = stringMessage.trimmed();
#endif

      stringMessage = "<font color=navy>" + stringMessage + "</font>";
    }
    stringMessage.replace(CMD_BEGIN, QString("<font color=orange>%1<cmd></font>").arg(QString(CMD_BEGIN).replace('<', "&lt;").replace('>', "&gt;")));
    stringMessage.replace(CMD_END, QString("<font color=orange>%1<cmd></font>").arg(QString(CMD_END).replace('<', "&lt;").replace('>', "&gt;")));
    stringMessage.replace(TIMEOUT_SYMBOL, QString("<font color=orange>%1<cmd></font>").arg(QString(TIMEOUT_SYMBOL).replace('<', "&lt;").replace('>', "&gt;")));
    emit output(stringMessage);
  }
}
