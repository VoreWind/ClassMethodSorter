#ifndef MEMBERSORTER_H
#define MEMBERSORTER_H

#include <QString>
#include <QVector>

#include <sorter.h>

class QStringList;

class MemberSorter : public Sorter {

public:
  MemberSorter();
  QString SortMembers(QStringList &split_section);

private:
  enum MemberTypes {
    kReferences,
    kReferenceWrappers,
    kReferenceWrapperContainers,
    kPointers,
    kPointerContainers,
    kSmartPointers,
    kSmartPointersContainers,
    kValues,
    kContainersUsedLikeValues, // Strings and whatnot
    kValueContainers,
    kFunctionalObjects,
    kFunctionalObjectsContainers
  };

  static bool SortingForMembers(const QString &left_member,
                                const QString &right_member);
  QString AssembleSortedString();
  void AddStringIntoListOfLists(int list_index, const QString &string);
  void PlaceMembersIntoGroups(const QStringList &members);
  void SortMembersInGroups();
  static const int kMemberGroupsAmount = 13;
  QVector<QStringList> member_groups_;
};

#endif // MEMBERSORTER_H
