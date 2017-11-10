#ifndef SORTER_H
#define SORTER_H

#include <QString>
#include <QVector>

class QStringList;
class Sorter {

public:
  static QString TruncateCommentsFromElement(const QString &element);
  static QStringList SplitSectionIntoElements(const QString &code_section);
  static int ElementStringAmount(const QString &element);

  static QString CleanString(const QString &string,
                             const QString &clutter_token = "\n\n\n",
                             const QString &clered_token = "\n\n");
  Sorter();
};

#endif // SORTER_H
