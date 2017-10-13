#ifndef CLASSBREAKER_H
#define CLASSBREAKER_H

#include <QList>
#include <QString>
#include <QVector>

struct ParsedClass {
  QString class_name;
  QVector<QString> split_class_body;
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

 private:
  static QList<int> BuildSectionPositionList(const QString& class_block);
  static QString FindClassName(const QString& block, int token_position);
  static QRegExp SectionFinderRegExp();
  static QVector<QString> SplitClassBlockToSections(const QString& class_block);

  static const QStringList kSectionNames;
  static const int kSectionsAmount = 7;
};

#endif  // CLASSBREAKER_H
