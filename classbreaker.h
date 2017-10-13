#ifndef CLASSBREAKER_H
#define CLASSBREAKER_H

#include <QList>
#include <QString>

struct ParsedClass {
  QString class_name;
  QString class_body;
  QList<ParsedClass> inner_classes;
};

class ClassBreaker {
 public:
  enum Sections {
    kPublic,
    kSignals,
    kPublicSlots,
    kProtectedSlots,
    kProtected,
    kPrivateSlots,
    kPrivate
  };

  static QList<ParsedClass> FindClassBlocksInString(QString& block);
  static QString FindClassName(const QString& block, int token_position);
  static QRegExp SectionFinderRegExp();

  static const QStringList section_names;
};

#endif  // CLASSBREAKER_H
