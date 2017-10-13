#ifndef SECTIONSORTER_H
#define SECTIONSORTER_H

#include <QString>
#include <QVector>

class QString;
class QStringList;

class SectionSorter {
 public:
  SectionSorter(QString class_name);
  static bool SortingForMethods(const QString &left_method,
                                const QString &right_method);
  static QString TruncateCommentsFromMethod(const QString &method);
  static int MethodParamsAmount(QString truncated_method);

  static int MethodStringAmount(QString truncated_method);
  void SortMethodsInGroups();
  QString AssembleSortedString();
  void PlaceMethodsIntoGroups(const QStringList &methods);

 private:
  enum Blocks {
    kUsingDirectives,
    kTypedefs,
    kEnums,
    kInlineStructs,
    kInlineClasses,
    kStaticMethods,
    kStaticFactoryMethods,
    kConstructors,
    kDestructors,
    kAssignmentOperators,
    kNonConstantOperators,
    kVirtualNonConstantMethods,
    kNonConstantMethods,
    kConstantOperators,
    kVirtualConstantMethods,
    kConstantMethods,

    kStaticNonConstantMembers,
    kStaticConstantMembers,
    kNonConstantMembers,
    kConstantMembers,

    kFriendClasses,
    kFriendOperators,
    kFriendMethods,
  };
  QStringList SplitSectionIntoMethods(const QString &code_section);
  void AddStringIntoListOfLists(int list_index, const QString &string);

  static const int kMethodGroupsAmount = 23;

  QVector<QStringList> method_groups_;
  QString class_name_;
};

#endif  // SECTIONSORTER_H
