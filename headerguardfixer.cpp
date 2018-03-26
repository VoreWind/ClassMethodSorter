#include "headerguardfixer.h"

#include <QDebug>
#include <QRegExp>

QString HeaderFileCleaner::FixHeaderGuardsInText(const QString &file_text,
                                                 const QString &file_name) {
  if (file_text.isEmpty()) {
    QString full_header_guard = BuildHeaderGuardForEmptyFile(file_name);
    return full_header_guard;
  }

  QString return_string = file_text;

  QString header_guard = FindHeaderGuard(file_text);
  QString header_guard_from_file = MakeHeaderGuardFromFileName(file_name);

  if (return_string.count(header_guard) > 3 ||
      !file_text.contains("#define " + header_guard)) {
    return "";
  }

  return_string.replace(header_guard, header_guard_from_file);

  QString endif_string = "#endif";
  int last_endif_position = return_string.lastIndexOf(endif_string);
  return_string.truncate(last_endif_position + endif_string.count());
  return_string.append("  // " + header_guard_from_file + "\n");

  return return_string;
}

QString HeaderFileCleaner::FindHeaderGuard(const QString &file_text) {
  QString header_guard_prefix = "#ifndef ";
  int header_guard_position = file_text.indexOf(header_guard_prefix);
  int header_guard_start_position =
      header_guard_position + header_guard_prefix.count();
  int space_position =
      file_text.indexOf(QRegExp(" |\n"), header_guard_start_position);

  return file_text.mid(header_guard_start_position,
                       space_position - header_guard_start_position);
}

QString
HeaderFileCleaner::MakeHeaderGuardFromFileName(const QString &file_name) {
  QString updated_file_name = file_name;
  updated_file_name.replace(".", "_");
  updated_file_name = updated_file_name.toUpper();

  return updated_file_name;
}

QString
HeaderFileCleaner::BuildHeaderGuardForEmptyFile(const QString &file_name) {
  QString header_guard_name = MakeHeaderGuardFromFileName(file_name);
  return "#ifndef " + header_guard_name + "\n#define " + header_guard_name +
         "\n\n#endif  // " + header_guard_name;
}
