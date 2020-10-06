#ifndef SERIALWORKER_H
#define SERIALWORKER_H

#include "settings.h"
#include <QByteArrayMatcher>
#include <QObject>
#include <QSerialPort>
#include <QThread>
#include <QTimer>

class SerialWorker : public QObject {
  Q_OBJECT
public:
  explicit SerialWorker(QObject *parent = nullptr);
  ~SerialWorker();
  /// Inicializuje objekt. Nutno zavolat až po přemístění do vlákna!
  void init();

signals:
  /// Pošle informaci jestli je port připojen
  void connectionResult(bool, QString);
  /// Předá celý úsek dat k dalšímu zpracování
  void newLine(QByteArray, int);
  /// Oznámý dokončení zápisu dat do portu
  void finishedWriting();
  /// Předá aktuální obsah bufferu
  void bufferDebug(QByteArray);

private:
  QTimer *lineTimeouter;
  int lineTimeout = 100;
  QSerialPort *serial;
  QByteArray *buffer;
  QByteArrayMatcher *commandBeginningMatcher;
  QByteArrayMatcher *commandEndMatcher;

private slots:
  void read();
  void lineTimedOut();
  void errorOccurred();

public slots:
  /// Změní timeout pro ukončením řádku
  void changeLineTimeout(int value) { lineTimeout = value; }
  /// Pokusí se otevřít port
  void begin(QString portName, int baudRate);
  /// Zavře port
  void end();
  /// Zapíše data do portu
  void write(QByteArray data);
  /// Vyžádá si poslání obsahu bufferu
  void requestedBufferDebug();
};

#endif // SERIALWORKER_H
