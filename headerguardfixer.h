#ifndef HEADERGUARDFIXER_H
#define HEADERGUARDFIXER_H

#include <QString>

class HeaderGuardFixer {
 public:
  static QString FixHeaderGuardsInText(const QString& file_text,
                                       const QString& file_name);

 private:
  static QString FindHeaderGuard(const QString& file_text);
  static bool IsHeaderGuardValid(const QString& file_name,
                                 const QString& header_guard);
};

#endif  // HEADERGUARDFIXER_H
