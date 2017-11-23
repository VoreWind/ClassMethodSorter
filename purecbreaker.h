#ifndef PURECBREAKER_H
#define PURECBREAKER_H

#include <QMap>
#include <QString>

class PureCBreaker {
 public:
  static QString SortHeader(const QString &header_code);

 private:
  enum Blocks {
    kUnsortableCode,
    kTypedefs,
    kTypedefEnums,
    kEnums,
    kTypedefStructs,
    kStructs,
    kTypedefUnions,
    kUnions,
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

  static QString ExtractUnsortableBottomFromCode(QString &code);
  static QString ExtractExternCBlockFromCode(QString &code);
  static QStringList ExtractUnsortableTopFromCode(QString &code);
  static void RemoveSingleLineFromCode(QString &code, const QString &line);
  static QMap<Blocks, bool (*)(const QString &)> PopulateAssistant();

  static void ExtractStructuresFromCode(QString &relevant_code,
                                        QVector<QStringList> &groups);
  static void ExtractUnionsFromCode(QString &relevant_code,
                                    QVector<QStringList> &groups);
  static QStringList SplitCodeToMethods(QString &relevant_code);
  static int MethodParamsAmount(const QString &method);
  static void SortGroups(QVector<QStringList> &groups);

  static bool SortingForPureC(const QString &left_method,
                              const QString &right_method);

  static QString AssembleHeaderBack(QVector<QStringList> groups);

  static void PlaceMethodsIntoGroups(const QStringList &methods,
                                     QVector<QStringList> &groups);

  static void AddStringIntoListOfLists(int list_index,
                                       const QString &string,
                                       QVector<QStringList> &groups);
  static const int kBlocksAmount = 11;
  static const QMap<Blocks, bool (*)(const QString &)> kSortingAssistant;

  static int FindCloseCurvyBracePositions(int token_position, QString &block);
  static bool DoesContainerHaveBraces(int token_position, QString &block);
  static void ExtractContainerFromCode(QString &relevant_code,
                                       QVector<QStringList> &groups,
                                       const QString &container_name,
                                       Blocks container,
                                       Blocks typedef_container);
  static int ContainerBlockEnd(int starter_index, QString &relevant_code);
};

#endif  // PURECBREAKER_H
