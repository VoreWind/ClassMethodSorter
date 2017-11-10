#ifndef SECTIONSORTER_H
#define SECTIONSORTER_H

#include <QString>
#include <QVector>

#include <sorter.h>

class QString;
class QStringList;

class SectionSorter : public Sorter {

public:
  SectionSorter(QString class_name);
  QString SortSection(QString &section);

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
  static int MethodParamsAmount(const QString &method);

  static bool SortingForMethods(const QString &left_method,
                                const QString &right_method);
  QString AssembleSortedString();
  void AddStringIntoListOfLists(int list_index, const QString &string);
  void PlaceMethodsIntoGroups(const QStringList &methods);
  void SortMethodsInGroups();
  static const int kMethodGroupsAmount = 23;

  QString class_name_;
  QVector<QStringList> method_groups_;
};

#endif // SECTIONSORTER_H
