#include "cobs.h"

QByteArray COBS::encode(QByteArray data) {
  QByteArray result;

  int srcIndex = 0;
  int dstIndex = 0;
  int lastZeroPos = 0;
  int lastZeroLength = 0;

  if (data.length() == 0) {
    return result;
  }

  result.append(1, static_cast<char>(0xFF)); // temporary
  dstIndex += 1;                             // First byte is overhead

  for (srcIndex = 0; srcIndex < data.length(); srcIndex++) {
    lastZeroLength += 1;

    if (lastZeroLength == 0xff) {
      result[lastZeroPos] = static_cast<char>(0xFF);
      lastZeroLength = 1;
      lastZeroPos = dstIndex;
      result.append(1, static_cast<char>(0xFF));
      dstIndex += 1;
    }

    if (static_cast<uint8_t>(data[srcIndex]) == 0) {
      result[lastZeroPos] = static_cast<char>(lastZeroLength);
      lastZeroLength = 0;
      lastZeroPos = dstIndex;
      result.append(1, static_cast<char>(0xFF));
      dstIndex += 1;
    } else {
      result.append(1, data[srcIndex]);
      dstIndex += 1;
    }
  }

  result[lastZeroPos] = static_cast<char>(lastZeroLength + 1);
  result.append(1, 0x00);

  return result;
}

QByteArray COBS::decode(QByteArray data) {
  QByteArray result;

  int srcIndex = 0;
  uint8_t zeroOffset = 0;
  uint8_t zeroValue = 0xFF;

  if (data.length() == 0) {
    return result;
  }

  while (srcIndex < data.length()) {
    if (zeroOffset != 0) {
      result.append(data.at(srcIndex));
      srcIndex += 1;
    } else {
      if (zeroValue != 0xFF) {
        result.append(1, 0x00);
      }
      zeroValue = static_cast<uint8_t>(data.at(srcIndex));
      zeroOffset = static_cast<uint8_t>(data.at(srcIndex));
      srcIndex += 1;
      if (zeroValue == 0x00) {
        break;
      }
    }
    zeroOffset -= 1;
  }

  result = result.mid(0, result.length());

  return result;
}
