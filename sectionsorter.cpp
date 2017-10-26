#include "sectionsorter.h"

#include <QDebug>
#include <QString>
#include <QStringList>

#include <membersorter.h>

SectionSorter::SectionSorter(QString class_name) : class_name_(class_name) {
  method_groups_.resize(kMethodGroupsAmount);
}

QString SectionSorter::SortSection(QString &section) {
  auto methods = SplitSectionIntoElements(section);
  PlaceMethodsIntoGroups(methods);
  SortMethodsInGroups();
  return AssembleSortedString();
}

bool SectionSorter::SortingForMethods(const QString &left_method,
                                      const QString &right_method) {
  QString left_truncated_method = TruncateCommentsFromElement(left_method);
  QString right_truncated_method = TruncateCommentsFromElement(right_method);

  int right_method_string_count = ElementStringAmount(right_method);
  int left_method_string_count = ElementStringAmount(left_method);

  const int one_line_method_string_count = 0;
  if (right_method_string_count == one_line_method_string_count &&
      left_method_string_count == one_line_method_string_count) {
    return left_method < right_method;
  }

  if (right_method_string_count == one_line_method_string_count ||
      left_method_string_count == one_line_method_string_count) {
    return left_method_string_count < right_method_string_count;
  }

  int right_method_params_count = MethodParamsAmount(right_truncated_method);
  int left_method_params_count = MethodParamsAmount(left_truncated_method);

  if (right_method_params_count != left_method_params_count) {
    return left_method_params_count < right_method_params_count;
  }

  if (right_method_string_count != left_method_string_count) {
    return left_method_string_count < right_method_string_count;
  }

  int right_truncated_method_string_count =
      ElementStringAmount(right_truncated_method);
  int left_truncated_method_string_count =
      ElementStringAmount(left_truncated_method);

  if (right_truncated_method_string_count !=
      left_truncated_method_string_count) {
    return left_truncated_method_string_count <
           right_truncated_method_string_count;
  }

  int left_method_brace_position = left_truncated_method.indexOf("(");
  int right_method_brace_position = right_truncated_method.indexOf("(");

  if (right_method_brace_position != left_method_brace_position) {
    return left_method_brace_position < right_method_brace_position;
  }

  return left_truncated_method < right_truncated_method;
}

int SectionSorter::MethodParamsAmount(const QString &method) {
  return method.count(",");
}

void SectionSorter::SortMethodsInGroups() {
  for (int i = 0; i < kMethodGroupsAmount; ++i) {
    if (i >= kStaticNonConstantMembers && i < kConstantMembers) {
      MemberSorter sorter;
      QString sorted_string = sorter.SortMembers(method_groups_[i]);
      if (!sorted_string.isEmpty()) {
        method_groups_[i] = QStringList(sorted_string);
      }
    } else {
      std::sort(method_groups_[i].begin(), method_groups_[i].end(),
                SortingForMethods);
    }
  }
}

QString SectionSorter::AssembleSortedString() {
  QString return_string;
  for (int i = 0; i < kMethodGroupsAmount; ++i) {
    QStringList methods = method_groups_.at(i);
    if (methods.count() != 0) {
      for (auto method : methods) {
        if (method.count("\n") > 0) {
          method.prepend("\n");
        }
        return_string += "\n" + method;
      }
    }
  }
  return return_string;
}

void SectionSorter::PlaceMethodsIntoGroups(const QStringList &methods) {
  for (auto method : methods) {
    auto clean_method = TruncateCommentsFromElement(method);
    if (clean_method.contains("friend")) {
      if (clean_method.contains("operator ")) {
        AddStringIntoListOfLists(kFriendOperators, method);
        continue;
      }
      if (clean_method.contains("class ")) {
        AddStringIntoListOfLists(kFriendClasses, method);
        continue;
      }
      if (clean_method.contains("(") && method.contains(")")) {
        AddStringIntoListOfLists(kFriendMethods, method);
        continue;
      }
    }

    // Methods
    if (clean_method.contains("(") && method.contains(")")) {
      // Destructors.
      if (clean_method.contains("~", Qt::CaseSensitive)) {
        AddStringIntoListOfLists(kDestructors, method);
        continue;
      }
      if (clean_method.contains("static ", Qt::CaseSensitive)) {
        AddStringIntoListOfLists(kStaticMethods, method);
        continue;
      }

      if (clean_method.contains(class_name_ + "(", Qt::CaseSensitive)) {
        AddStringIntoListOfLists(kConstructors, method);
        continue;
      }

      if (clean_method.contains("virtual ") || method.contains(" override")) {
        if (clean_method.contains(") const", Qt::CaseSensitive)) {
          AddStringIntoListOfLists(kVirtualConstantMethods, method);
          continue;
        } else {
          AddStringIntoListOfLists(kVirtualNonConstantMethods, method);
          continue;
        }
      }

      if (clean_method.contains("operator= ", Qt::CaseSensitive)) {
        AddStringIntoListOfLists(kAssignmentOperators, method);
        continue;
      } else if (clean_method.contains("const", Qt::CaseSensitive) &&
                 clean_method.contains("operator ", Qt::CaseSensitive)) {
        AddStringIntoListOfLists(kConstantOperators, method);
        continue;
      } else if (clean_method.contains(") const", Qt::CaseSensitive)) {
        AddStringIntoListOfLists(kConstantMethods, method);
        continue;
      } else {
        AddStringIntoListOfLists(kNonConstantMethods, method);
        continue;
      }
    } else  // Non-methods
        if (clean_method.contains("using ", Qt::CaseSensitive)) {
      AddStringIntoListOfLists(kUsingDirectives, method);
      continue;
    }
    if (clean_method.contains("typedef ", Qt::CaseSensitive)) {
      AddStringIntoListOfLists(kTypedefs, method);
      continue;
    }
    if (clean_method.contains("enum ", Qt::CaseSensitive)) {
      AddStringIntoListOfLists(kEnums, method);
      continue;
    }
    if (clean_method.contains("struct ##", Qt::CaseSensitive)) {
      AddStringIntoListOfLists(kInlineStructs, method);
      continue;
    }
    if (clean_method.contains("class ##", Qt::CaseSensitive)) {
      AddStringIntoListOfLists(kInlineClasses, method);
      continue;
    }

    // Members
    if (clean_method.contains("static ")) {
      if (clean_method.contains("const ", Qt::CaseSensitive)) {
        AddStringIntoListOfLists(kStaticConstantMembers, method);
        continue;
      } else {
        AddStringIntoListOfLists(kStaticNonConstantMembers, method);
        continue;
      }
    } else {
      if (clean_method.contains("const ", Qt::CaseSensitive)) {
        AddStringIntoListOfLists(kConstantMembers, method);
        continue;
      } else {
        AddStringIntoListOfLists(kNonConstantMembers, method);
        continue;
      }
    }
  }
}

void SectionSorter::AddStringIntoListOfLists(int list_index,
                                             const QString &string) {
  method_groups_[list_index].push_back(string);
}
