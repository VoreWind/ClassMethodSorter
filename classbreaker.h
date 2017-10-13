#ifndef CLASSBREAKER_H
#define CLASSBREAKER_H

#include <QList>
#include <QString>
#include <QVector>

struct ParsedClass {
  QString class_name;
  QString class_header;
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
  static QString AssembleBlockBack(ParsedClass parsed_class,
                                   QString& initial_string);

 private:
  static QList<int> BuildSectionPositionList(const QString& class_block);
  static QString FindClassName(const QString& block, int token_position);
  static QRegExp SectionFinderRegExp();
  static QVector<QString> SplitClassBlockToSections(const QString& class_block);

  static QString AssembleBlockFromSections(QVector<QString> sections);

  static const QStringList kSectionNames;
  static const int kSectionsAmount = 7;
  static QString FindClassHeader(const QString& block, int token_position);
};

#endif  // CLASSBREAKER_H
