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
                                     const QString &right_member) {
  QString left_truncated_member = TruncateCommentsFromElement(left_member);
  QString right_truncated_member = TruncateCommentsFromElement(right_member);

  int right_method_string_count = ElementStringAmount(right_member);
  int left_method_string_count = ElementStringAmount(left_member);

  if (right_method_string_count == 1 && left_method_string_count == 1) {
    return left_member < right_member;
  }

  if (right_method_string_count == 1 || left_method_string_count == 1) {
    return left_method_string_count < right_method_string_count;
  }

  if (right_method_string_count != left_method_string_count) {
    return left_method_string_count < right_method_string_count;
  }

  int right_truncated_method_string_count =
      ElementStringAmount(right_truncated_member);
  int left_truncated_method_string_count =
      ElementStringAmount(left_truncated_member);

  if (right_truncated_method_string_count !=
      left_truncated_method_string_count) {
    return left_truncated_method_string_count <
           right_truncated_method_string_count;
  }

  return left_truncated_member < right_truncated_member;
}

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

void MemberSorter::SortMethodsInGroups() {
  for (int i = 0; i < kMemberGroupsAmount; ++i) {
    std::sort(member_groups_[i].begin(), member_groups_[i].end(),
              this->SortingForMembers);
  }
}
