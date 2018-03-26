#ifndef HEADERGUARDFIXER_H
#define HEADERGUARDFIXER_H

#include <QString>

class HeaderFileCleaner {

public:
  static QString FixHeaderGuardsInText(const QString &file_text,
                                       const QString &file_name);

private:
  static QString FindHeaderGuard(const QString &file_text);
  static QString MakeHeaderGuardFromFileName(const QString &file_name);
  static QString BuildHeaderGuardForEmptyFile(const QString &file_name);
};

#endif // HEADERGUARDFIXER_H
