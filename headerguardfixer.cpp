#include "headerguardfixer.h"

#include <QDebug>
#include <QRegExp>

QString HeaderGuardFixer::FixHeaderGuardsInText(const QString &file_text,
                                                const QString &file_name) {
  QString return_string = file_text;

  QString header_guard = FindHeaderGuard(file_text);
  QString header_guard_from_file = MakeHeaderGuardFromFileName(file_name);

  if (return_string.count(header_guard) > 3) {
    return "";
  } else if (return_string.count(header_guard) < 3) {
    if (file_text.contains("#define " + header_guard)) {
      QString endif_string = "#endif";
      int last_endif_position = file_text.lastIndexOf(endif_string);
      return_string.insert(last_endif_position + endif_string.count(),
                           "  // " + header_guard_from_file);
    }
  }

  return_string.replace(header_guard, header_guard_from_file);
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

QString HeaderGuardFixer::MakeHeaderGuardFromFileName(
    const QString &file_name) {
  QString updated_file_name = file_name;
  updated_file_name.replace(".", "_");
  updated_file_name = updated_file_name.toUpper();

  return updated_file_name;
}
