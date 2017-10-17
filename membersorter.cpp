#include "membersorter.h"

#include <QDebug>
#include <QStringList>

MemberSorter::MemberSorter() {}

QString MemberSorter::SortMembers(QString &section) {
  auto methods = SplitSectionIntoElements(section);
  PlaceMembersIntoGroups(methods);
  SortMethodsInGroups();
  return AssembleSortedString();
}

bool MemberSorter::SortingForMembers(const QString &left_member,
                                     const QString &right_member) {}

QString MemberSorter::AssembleSortedString() {
  QString return_string;
  for (int i = 0; i < kMemberGroupsAmount; ++i) {
    QStringList methods = member_groups_.at(i);
    for (auto method : methods) {
      if (method.count("\n") > 1) {
        method.prepend("\n");
      }
      return_string += method;
    }
    return_string.append("\n");
  }

  return CleanString(return_string);
}

void MemberSorter::AddStringIntoListOfLists(int list_index,
                                            const QString &string) {
  member_groups_[list_index].push_back(string);
}

void MemberSorter::PlaceMembersIntoGroups(const QStringList &members) {}

void MemberSorter::SortMethodsInGroups() {}
