#ifndef PURECBREAKER_H
#define PURECBREAKER_H

#include <QMap>
#include <QString>

class PureCBreaker {
 public:
  static QString FindRelevantCode(QString &header_code);
  static QString SortHeader(const QString &header_code);

 private:
  enum Blocks {
    kIncludes,
    kExternC,
    kUnsortableDefines,
    kMacros,
    kDefineCostants,
    kTypedefs,
    kTypedefEnums,
    kEnums,
    kTypedefStructs,
    kStructs,
    kFunctions,
    kExternVariables,
    kOtherVariables
  };

  static bool IsBlockTypedefEnum(const QString &block);
  static bool IsBlockEnum(const QString &block);
  static bool IsBlockFunction(const QString &block);
  static bool IsBlockExternVariable(const QString &block);
  static bool IsBlockOtherVariable(const QString &block);
  static bool IsBlockTypedef(const QString &block);

  static QStringList ExtractIncludesFromRelevantCode(QString &relevant_code);
  static int FindLowestIncludeInIrrelevantCode(const QString &irrelevant_code);

  static QMap<Blocks, bool (*)(const QString &)> PopulateAssistant();

  static void ExtractIfdefMacrosFromCode(QString &relevant_code,
                                         QVector<QStringList> &groups);
  static void ExtractMacrosFromCode(QString &relevant_code,
                                    QVector<QStringList> &groups);
  static void ExtractStructuresFromCode(QString &relevant_code,
                                        QVector<QStringList> &groups);
  static QStringList SplitCodeToMethods(QString &relevant_code);
  static int MethodParamsAmount(const QString &method);
  static void SortGroups(QVector<QStringList> &groups);

  static bool SortingForPureC(const QString &left_method,
                              const QString &right_method);

  static void AssembleHeaderBack(QString &header_code,
                                 QVector<QStringList> groups);

  static void PlaceMethodsIntoGroups(const QStringList &methods,
                                     QVector<QStringList> &groups);

  static void AddStringIntoListOfLists(int list_index,
                                       const QString &string,
                                       QVector<QStringList> &groups);
  static const int kBlocksAmount = 12;
  static const QMap<Blocks, bool (*)(const QString &)> kSortingAssistant;

  static int FindCloseCurvyBracePositions(int token_position, QString &block);
};

#endif  // PURECBREAKER_H
