#include "membersorter.h"

#include <QDebug>
#include <QStringList>

MemberSorter::MemberSorter() { member_groups_.resize(kMemberGroupsAmount); }

QString MemberSorter::SortMembers(QStringList &split_section) {
  if (split_section.count() != 0) {
    PlaceMembersIntoGroups(split_section);
    SortMembersInGroups();
    return AssembleSortedString();
  }
  return "";
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
    if (methods.count() != 0) {
      for (auto method : methods) {
        if (method.count("\n") > 0) {
          method.prepend("\n");
        }
        return_string += "\n" + method;
      }
      return_string.append("\n");
    }
  }
  return return_string.trimmed();
}

void MemberSorter::AddStringIntoListOfLists(int list_index,
                                            const QString &string) {
  member_groups_[list_index].push_back(string);
}

void MemberSorter::PlaceMembersIntoGroups(const QStringList &members) {
  for (auto member : members) {
    if (member.contains("*")) {
      if (member.contains("<")) {
        AddStringIntoListOfLists(kPointerContainers, member);
        continue;
      } else {
        AddStringIntoListOfLists(kPointers, member);
        continue;
      }
    }

    if (member.contains("&")) {
      if (member.contains("<")) {
        AddStringIntoListOfLists(kReferenceWrapperContainers, member);
        continue;
      } else {
        AddStringIntoListOfLists(kReferences, member);
        continue;
      }
    }

    if (member.contains("Pointer", Qt::CaseSensitive) ||
        member.contains("_ptr", Qt::CaseSensitive)) {
      if (member.contains("<")) {
        AddStringIntoListOfLists(kSmartPointersContainers, member);
        continue;
      } else {
        AddStringIntoListOfLists(kSmartPointers, member);
        continue;
      }
    }

    if (member.contains("QString", Qt::CaseSensitive) ||
        member.contains("QByteArray", Qt::CaseSensitive) ||
        member.contains("QStringList", Qt::CaseSensitive) ||
        member.contains("std::string", Qt::CaseSensitive)) {
      AddStringIntoListOfLists(kContainersUsedLikeValues, member);
      continue;
    }

    if (member.contains("<")) {
      AddStringIntoListOfLists(kValueContainers, member);
      continue;
    } else {
      AddStringIntoListOfLists(kValues, member);
      continue;
    }
  }
}

void MemberSorter::SortMembersInGroups() {
  for (int i = 0; i < kMemberGroupsAmount; ++i) {
    std::sort(member_groups_[i].begin(), member_groups_[i].end(),
              SortingForMembers);
  }
}
