#ifndef QMLTERMINALINTERFACE_H
#define QMLTERMINALINTERFACE_H

#include <QObject>
#include <QPalette>
#include <QQuickItem>

class QmlTerminalInterface : public QObject {
  Q_OBJECT
public:
  explicit QmlTerminalInterface(QObject *parent = nullptr);
  ~QmlTerminalInterface() {}

  Q_INVOKABLE void transmitToSerial(QVariant data, QString type = "s");
  Q_INVOKABLE void sendToParser(QVariant data);

  void directInput(QByteArray data);

public:
  bool darkThemeIsUsed() const;
  void setDarkThemeIsUsed(bool newDarkThemeIsUsed);

  const QString &tabBackground() const;
  void setTabBackground(const QString &newTabBackground);

signals:
  void receivedFromSerial(QByteArray data);
  void dataTransmitted(QByteArray data);
  void dataSentToParser(QByteArray data);
  void darkThemeIsUsedChanged();
  void themePaletteChanged();
  void basicTextSizeChanged();
  void tabBackgroundChanged();
  void deviceMessagesChanged();
  void deviceMessagesWarningOrInfoChanged();

private:
  bool m_darkThemeIsUsed;
  QPalette m_themePalette;
  QString m_tabBackground;

  Q_PROPERTY(bool darkThemeIsUsed READ darkThemeIsUsed NOTIFY darkThemeIsUsedChanged)
  Q_PROPERTY(QString tabBackground READ tabBackground NOTIFY tabBackgroundChanged)
};

#endif // QMLTERMINALINTERFACE_H
