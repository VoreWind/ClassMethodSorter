#ifndef CLASSBREAKER_H
#define CLASSBREAKER_H

#include <QList>
#include <QString>
#include <QVector>

struct ParsedClass {
  bool is_public_section_shown = true;

  QString class_header;
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
  static QList<ParsedClass> FindClassBlocksInString(QString &block);
  static int FindClassTokenPosition(const QString &block);
  static void SortClassSections(ParsedClass &parsed_class);

  static QString AssembleClassBack(ParsedClass parsed_class,
                                   QString &initial_string);

 private:
  static QList<int> BuildSectionPositionList(const QString &class_block);
  static QRegExp SectionFinderRegExp();
  static QString CleanClassFromMacros(const QString &class_string);
  static QString FindClassHeader(const QString &block, int token_position);
  static QString FindClassName(const QString &block, int token_position);
  static QString FindHeader(const QString &block, int token_position);
  static QString FindStructHeader(const QString &block, int token_position);
  static QVector<QString> SplitClassBlockToSections(const QString &class_block);
  static bool DoesBlockContainUnparseableCode(QString &block);
  static bool IsClassBlockStatringWithSectionToken(const QString &block);

  static QString InsertSectionToClass(const QString &class_string,
                                      bool is_class);

  static QString AssembleBlockFromSections(QVector<QString> sections,
                                           bool is_public_section_shown = true);

  static const int kSectionsAmount = 7;

  static const QStringList kSectionNames;

  static int FindCloseCurvyBracePositions(QString &class_block,
                                          int token_position,
                                          QString &block);
};

#endif  // CLASSBREAKER_H
