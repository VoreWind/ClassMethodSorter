#ifndef PURECBREAKER_H
#define PURECBREAKER_H

#include <QString>

class PureCBreaker {
 public:
  static QString FindRelevantCode(QString &header_code);
  static QString SortHeader(const QString &header_code);

 private:
  enum Blocks {
    kMacros,
    kDefineCostants,

    kTypedefs,
    kEnums,
    kInlineStructs,
    kStaticMethods,
    kNonConstantMethods,
    kConstantMethods,
    kStaticNonConstantMembers,
    kStaticConstantMembers,
    kNonConstantMembers,
    kConstantMembers,
  };
  static const int kBlocksAmount = 12;

  static void AddStringIntoListOfLists(int list_index,
                                       const QString &string,
                                       QVector<QStringList> &groups);
  static QStringList RemoveMacrosFromCode(QString &relevant_code);
  static QStringList RemoveStructuresFromCode(QString &relevant_code);
  static QStringList SplitCodeToMethods(QString &relevant_code);
  static QVector<QStringList> PlaceMethodsIntoGroups(
      const QStringList &macros, const QStringList &methods);
  static void SortGroups(QVector<QStringList> &groups);
  static QString AssembleHeaderBack(const QString &relevant_code,
                                    QVector<QStringList> groups);
};

#endif  // PURECBREAKER_H
