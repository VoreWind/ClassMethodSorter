#include "headerguardfixer.h"

#include <QDebug>
#include <QRegExp>

QString HeaderGuardFixer::FixHeaderGuardsInText(const QString &file_text,
                                                const QString &file_name) {
  QString return_string;

  qDebug() << FindHeaderGuard(file_text);
  return return_string;
}

QString HeaderGuardFixer::FindHeaderGuard(const QString &file_text) {
  QString header_guard_prefix = "#ifndef ";
  int header_guard_position = file_text.indexOf(header_guard_prefix);
  int header_guard_start_position =
      header_guard_position + header_guard_prefix.count();
  int space_position =
      file_text.indexOf(QRegExp(" |\n"), header_guard_start_position);

  return file_text.mid(header_guard_start_position,
                       space_position - header_guard_start_position);
}
