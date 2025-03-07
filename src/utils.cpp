#include "utils.h"
#include "global.h"
#include "qglobal.h"
#include "qregularexpression.h"

double floorToNiceValue(double value) {
  if (value > 0) {
    auto power = floor(log10(value));
    double one = pow(10, power);
    double two = 2.0 * one;
    double five = 5.0 * one;
    double ten = 10.0 * one;
    if (qFuzzyCompare(value, ten))
      return ten;
    if (value > five || qFuzzyCompare(value, five))
      return five;
    if (value > two || qFuzzyCompare(value, two))
      return two;
    return one;
  } else if (value < 0)
    return (-floorToNiceValue(-value));
  else
    return 0;
}

double ceilToNiceValue(double value) {
  if (value > 0) {
    auto power = floor(log10(value));
    double one = pow(10, power);
    double two = 2.0 * one;
    double five = 5.0 * one;
    double ten = 10.0 * one;
    if (qFuzzyCompare(value, one))
      return one;
    if (value < two || qFuzzyCompare(value, two))
      return two;
    if (value < five || qFuzzyCompare(value, five))
      return five;
    return ten;
  } else if (value < 0)
    return (-ceilToNiceValue(-value));
  else
    return 0;
}

int getAnalogChId(int number, ChannelType::enumChannelType type) {
  if (type == ChannelType::analog)
    return (number - 1);
  if (type == ChannelType::math)
    return (number + ANALOG_COUNT - 1);
  return 0;
}

int getLogicChannelID(int group, int bit) { return (ANALOG_COUNT + MATH_COUNT + (group)*LOGIC_BITS + bit); }

QString getChName(int chid) {
  if (ChID_TO_LOGIC_GROUP(chid) == LOGIC_GROUPS - 1)
    return (QObject::tr("Logic bit %1").arg(ChID_TO_LOGIC_GROUP_BIT(chid)));
  if (chid >= ANALOG_COUNT + MATH_COUNT)
    return (QObject::tr("Logic %1 bit %2").arg(ChID_TO_LOGIC_GROUP(chid) + 1).arg(ChID_TO_LOGIC_GROUP_BIT(chid)));
  if (chid >= ANALOG_COUNT)
    return (QObject::tr("Math %1").arg(chid - ANALOG_COUNT + 1));
  return (QObject::tr("Ch %1").arg(chid + 1));
}

int intLog10(double x) {
  if (qFuzzyIsNull(x)) // - nekonečno
    return -1000;
  if (qIsInf(x) || qIsInf(-x))
    return 100;
  if (qIsNaN(x))
    return 0;
  x = std::abs(x);
  double result = log10(x);

  // Exact power of 10 can be floored to one lower because of numeric inacccuracy
  // Check if not rounded result is close to rounded
  double fr = floor(result);
  double rr = round(result);
  if (qFuzzyCompare(rr, result))
    return rr;
  else
    return fr;
}

QString toSignificantDigits(double x, int prec, bool trimZeroes) {
  Q_ASSERT(prec > 0);

  if (qFuzzyIsNull(x)) {
    if (trimZeroes || prec == 1)
      return "0";
    QByteArray zeroes = "0.";
    for (int i = 1; i < prec; i++)
      zeroes.append('0');
    return zeroes;
  }

  // Vlivem zaokrouhlní může hodnota být trochu menší než měla být,
  // například 1.000000 se může změnit na 0.99999999, proto je číslo mírně
  // zvětšeno, aby se zajistilo, že rád nevfijde o jedna mensí než měl být.
  int log10ofX = intLog10(x);

  if (log10ofX >= prec - 1) {
    return QString::number((int)round(x));
  } else {
    // Převedu na text jako celé číslo
    QString result = QString::number((int)round(x * (pow(10, prec - log10ofX - 1))));

    // Na příslušné místo vloží desetinnou tečku
    int decimalPoint = result.length() - prec + log10ofX + 1;
    if (decimalPoint > 0) {
      result.insert(decimalPoint, '.');
      if (trimZeroes) {
        // Odřízne nuly z desetinných míst
        for (int i = result.length() - 1; i >= decimalPoint; i--) {
          if (result.at(i) == '0' || result.at(i) == '.')
            result.remove(i, 1);
          else
            break;
        }
      }
    } else {
      // Pokud číslo nemá celou část, je před něj přidána nula s desettinou tečkou
      // Případně další nuly za tečkou
      for (; decimalPoint < 0; decimalPoint++)
        result.push_front('0');
      result.push_front('.');
      result.push_front('0');
    }
    return result;
  }
}

QString valueTypeToString(ValueType val) {
  if (val.isBinary) {
    QString description;
    description = QString::number(val.bytes * 8) + "-bit ";
    switch (val.type) {
    case ValueType::Type::invalid:
      return "Invalid data";
    case ValueType::Type::incomplete:
      return "Incomplete data";
    case ValueType::Type::integer:
      description += "signed integer";
      break;
    case ValueType::Type::unsignedint:
      description += "unsigned integer";
      break;
    case ValueType::Type::floatingpoint:
      description += "floating point";
      break;
    default:
      break;
    }
    if (val.bigEndian)
      description.append(" (big endian)");
    else
      description.append(" (little endian)");
    return description;
  } else
    return ("Decimal");
}

UnitOfMeasure::UnitOfMeasure(QString rawUnit) {

  QString prefixChars = "munkMG";

  if (rawUnit.isEmpty())
    mode = noPrefix;
  else if (rawUnit.startsWith("-")) {
    mode = usePrefix;
    text = rawUnit.mid(1);
  } else if (rawUnit.startsWith("!")) {
    mode = noPrefix;
    text = rawUnit.mid(1);
  } else if (rawUnit == "index") {
    mode = index;
  } else if (rawUnit.toLower().trimmed() == QString("time") || rawUnit.toLower().trimmed() == QObject::tr("time")) {
    mode = time;
    text = "s";
    static QRegularExpression regex("\\(([^)]+)\\)");
    QRegularExpressionMatch match = regex.match(rawUnit);
    if (match.hasMatch())
      special = match.captured(1);
  } else if (rawUnit.length() >= 2 && (rawUnit.left(2) == "dB" || prefixChars.contains(rawUnit.at(0)))) {
    mode = noPrefix;
    text = rawUnit;
  } else {
    text = rawUnit;
    mode = usePrefix;
  }
}

ValueType readValuePrefix(QByteArray &buffer, int &detectedPrefixLength) {
  ValueType valType;
  if (buffer.length() < 2)
    return valType; // Incomplete
  if (!isdigit(buffer.at(1))) {
    detectedPrefixLength = 3;
    if (buffer.length() < 3)
      return valType; // Incomplete
    switch (buffer.at(0)) {
    case 'T':
      valType.multiplier = 1e12;
      break;
    case 'G':
      valType.multiplier = 1e9;
      break;
    case 'M':
      valType.multiplier = 1e6;
      break;
    case 'k':
      valType.multiplier = 1e3;
      break;
    case 'h':
      valType.multiplier = 1e2;
      break;
    case 'D':
      valType.multiplier = 1e1;
      break;
    case 'd':
      valType.multiplier = 1e-1;
      break;
    case 'c':
      valType.multiplier = 1e-2;
      break;
    case 'm':
      valType.multiplier = 1e-3;
      break;
    case 'u':
      valType.multiplier = 1e-6;
      break;
    case 'n':
      valType.multiplier = 1e-9;
      break;
    case 'p':
      valType.multiplier = 1e-12;
      break;
    case 'f':
      valType.multiplier = 1e-15;
      break;
    case 'a':
      valType.multiplier = 1e-18;
      break;
    default:
      valType.type = ValueType::invalid;
      return valType; // Invalid
    }
  } else
    detectedPrefixLength = 2;

#define unitPosition 0
#define typePosition detectedPrefixLength - 2
#define bytesPosition detectedPrefixLength - 1

  switch (tolower(buffer.at(typePosition))) {
  case 'u':
    valType.type = ValueType::unsignedint;
    valType.bytes = buffer.at(bytesPosition) - '0';
    if (valType.bytes != 1 && valType.bytes != 2 && valType.bytes != 3 && valType.bytes != 4)
      valType.type = ValueType::invalid;
    break;
  case 'i':
    valType.type = ValueType::integer;
    valType.bytes = buffer.at(bytesPosition) - '0';
    if (valType.bytes != 1 && valType.bytes != 2 && valType.bytes != 4)
      valType.type = ValueType::invalid;
    break;
  case 'f':
    valType.type = ValueType::floatingpoint;
    valType.bytes = buffer.at(bytesPosition) - '0';
    if (valType.bytes != 4 && valType.bytes != 8)
      valType.type = ValueType::invalid;
    break;
  default:
    valType.type = ValueType::invalid;
    return valType; // Invalid
  }
  valType.bigEndian = (buffer.at(typePosition) == toupper(buffer.at(typePosition)));
  return valType;
}

QString floatToNiceString(double d, int significantDigits, bool justify, bool justifyUnit, bool noDecimalsIfInteger, UnitOfMeasure unit) {
  QString text = "";
  QString postfix = "";

  if (qIsInf(d)) {
    text = justifyUnit ? "\xe2\x88\x9e  " : "\xe2\x88\x9e "; // Infinity
    goto final_justify;
  } else if (qIsNaN(d)) {
    text = justifyUnit ? "---  " : "--- ";
    goto final_justify;
  } else {

    if (unit.mode == UnitOfMeasure::noPrefix) {
    noPrefix:
      text = toSignificantDigits(d, significantDigits, noDecimalsIfInteger) + (justifyUnit ? "  " : " ");
      goto final_justify;
    }

    if (unit.mode == UnitOfMeasure::index) {
      text = QString::number(floor(d), 'f', 0);
      goto final_justify;
    }

    int order = intLog10(d);

    if (qFuzzyIsNull(d)) {
      d = 0;
      postfix = justify ? "  " : " ";
    } else if (order >= 21)
      goto noPrefix;
    else if (order >= 18) {
      postfix = " E";
      d /= 1e18;
    } else if (order >= 15) {
      postfix = " P";
      d /= 1e15;
    } else if (order >= 12) {
      postfix = " T";
      d /= 1e12;
    } else if (order >= 9) {
      postfix = " G";
      d /= 1e9;
    } else if (order >= 6) {
      postfix = " M";
      d /= 1e6;
    } else if (order >= 3) {
      postfix = " k";
      d /= 1e3;
    } else if (order >= 0) {
      goto noPrefix;
    } else if (order >= -3) {
      postfix = " m";
      d /= 1e-3;
    } else if (order >= -6) {
      postfix = " " + QString::fromUtf8("\xc2\xb5"); // mikro
      d /= 1e-6;
    } else if (order >= -9) {
      postfix = " n";
      d /= 1e-9;
    } else if (order >= -12) {
      postfix = " p";
      d /= 1e-12;
    } else if (order >= -15) {
      postfix = " f";
      d /= 1e-15;
    } else {
      goto noPrefix;
    }
    text = toSignificantDigits(d, significantDigits, noDecimalsIfInteger);
  }

  text.append(postfix);
final_justify:
  if (justify) {
    return text.rightJustified(significantDigits + ((text.right(1) == " " && !justifyUnit) ? 3 : 4)) + unit.text;
  } else
    return text + unit.text;
}

bool operator==(const QSerialPortInfo &lhs, const QSerialPortInfo &rhs) { return lhs.portName() == rhs.portName() && lhs.serialNumber() == rhs.serialNumber() && lhs.description() == rhs.description(); }

int nextPow2(int number) {
  for (int i = 0;; i++)
    if (pow(2, i) >= number)
      return (pow(2, i));
}
