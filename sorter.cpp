#include "sorter.h"

#include <QDebug>
#include <QStringList>

Sorter::Sorter() {}

int Sorter::ElementStringAmount(const QString &element) {
  return element.count("\n");
}

QString Sorter::TruncateCommentsFromElement(const QString &element) {
  QStringList split_method = element.split("\n");
  QString truncated_method;
  for (auto method_line : split_method) {
    if (!method_line.contains(QRegExp("^ *\\/\\/"))) {
      truncated_method += method_line + "\n";
    }
  }
  return truncated_method;
}

QStringList Sorter::SplitSectionIntoElements(const QString &code_section) {
  QRegExp splitter("(;\n)|(;.*\\.\n)");
  splitter.setMinimal(true);
  QStringList list;
  for (int i = 0; i < code_section.count(splitter); ++i) {
    list << code_section.section(splitter, i, i,
                                 QString::SectionIncludeTrailingSep);
  }
  return list;
}

QString Sorter::CleanString(const QString &string,
                            const QString &clutter_token,
                            const QString &clered_token) {
  QString clean_string = string;
  while (clean_string.contains(clutter_token)) {
    clean_string.replace(clutter_token, clered_token);
  }
  return clean_string;
}
