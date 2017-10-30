#include "purecbreaker.h"

#include <sectionsorter.h>

QString PureCBreaker::FindRelevantCode(const QString &header_code) {
  const QString opening_tag = "#ifdef __cplusplus\nextern \"C\" {\n#endif";
  const QString closing_tag = "#ifdef __cplusplus\n}  // extern \"C\"\n#endif";

  int opening_tag_position =
      header_code.indexOf(opening_tag) + opening_tag.count();
  int closing_tag_position =
      closing_tag.indexOf(opening_tag) + opening_tag.count();

  if (opening_tag_position == -1 || closing_tag_position == -1) {
    return "";
  } else {
    return header_code.mid(opening_tag_position,
                           closing_tag_position - opening_tag_position);
  }
}

QString PureCBreaker::SortHeader(const QString &header_code) {
  QString relevant_code = FindRelevantCode(header_code);
  QStringList macros = RemoveMacrosFromCode(relevant_code);
  QStringList methods = SplitCodeToMethods(relevant_code);
  QVector<QStringList> groups = PlaceMethodsIntoGroups(macros, methods);
  SortGroups(groups);
  return AssembleHeaderBack(relevant_code, groups);
}

QStringList PureCBreaker::RemoveMacrosFromCode(QString &relevant_code) {}

QStringList PureCBreaker::SplitCodeToMethods(QString &relevant_code) {}

QVector<QStringList> PureCBreaker::PlaceMethodsIntoGroups(
    const QStringList &macros, const QStringList &methods) {}

void PureCBreaker::SortGroups(QVector<QStringList> &groups) {}

QString PureCBreaker::AssembleHeaderBack(const QString &relevant_code,
                                         QVector<QStringList> groups) {}
