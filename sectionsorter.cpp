#include "sectionsorter.h"

#include <QString>
#include <QStringList>

SectionSorter::SectionSorter(QString class_name) : class_name_(class_name) {
  method_groups_.resize(kMethodGroupsAmount);
}

QString SectionSorter::SortSection(QString &section) {
  auto methods = SplitSectionIntoMethods(section);
  PlaceMethodsIntoGroups(methods);
  SortMethodsInGroups();
  return AssembleSortedString();
}

bool SectionSorter::SortingForMethods(const QString &left_method,
                                      const QString &right_method) {
  QString left_truncated_method = TruncateCommentsFromMethod(left_method);
  QString right_truncated_method = TruncateCommentsFromMethod(right_method);

  int right_method_string_count = MethodStringAmount(right_method);
  int left_method_string_count = MethodStringAmount(left_method);

  if (right_method_string_count == 1 && left_method_string_count == 1) {
    return left_method < right_method;
  }

  if (right_method_string_count == 1 || left_method_string_count == 1) {
    return left_method_string_count < right_method_string_count;
  }

  int right_method_params_count = MethodParamsAmount(right_truncated_method);
  int left_method_params_count = MethodParamsAmount(left_truncated_method);

  if (right_method_params_count != left_method_params_count) {
    return left_method_params_count < right_method_params_count;
  }

  int right_truncated_method_string_count =
      MethodStringAmount(right_truncated_method);
  int left_truncated_method_string_count =
      MethodStringAmount(left_truncated_method);

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

QString SectionSorter::TruncateCommentsFromMethod(const QString &method) {
  QStringList split_method = method.split("\n");
  QString truncated_method;
  for (auto method_line : split_method) {
    if (!method_line.contains(QRegExp("^ *\/\/"))) {
      truncated_method += method_line + "\n";
    }
  }
  return truncated_method;
}

int SectionSorter::MethodParamsAmount(QString truncated_method) {
  return truncated_method.count(",");
}

int SectionSorter::MethodStringAmount(QString truncated_method) {
  return truncated_method.count("\n");
}

void SectionSorter::SortMethodsInGroups() {
  for (int i = 0; i < kMethodGroupsAmount; ++i) {
    std::sort(method_groups_[i].begin(), method_groups_[i].end(),
              this->SortingForMethods);
  }
}

QString SectionSorter::AssembleSortedString() {
  QString return_string;
  for (int i = 0; i < kMethodGroupsAmount; ++i) {
    QStringList methods = method_groups_.at(i);
    for (auto method : methods) {
      return_string += method + ";\n\n";
    }
  }
  return return_string;
}

void SectionSorter::PlaceMethodsIntoGroups(const QStringList &methods) {
  for (auto method : methods) {
    if (method.contains("friend")) {
      if (method.contains("operator ")) {
        AddStringIntoListOfLists(kFriendOperators, method);
        continue;
      }
      if (method.contains("class ")) {
        AddStringIntoListOfLists(kFriendClasses, method);
        continue;
      }
      if (method.contains("(") && method.contains(")")) {
        AddStringIntoListOfLists(kFriendMethods, method);
        continue;
      }
    }

    // Methods
    if (method.contains("(") && method.contains(")")) {
      // Destructors.
      if (method.contains("~", Qt::CaseSensitive)) {
        AddStringIntoListOfLists(kDestructors, method);
        continue;
      }
      if (method.contains("static ", Qt::CaseSensitive)) {
        AddStringIntoListOfLists(kStaticMethods, method);
        continue;
      }

      if (method.contains(class_name_ + "(", Qt::CaseSensitive)) {
        AddStringIntoListOfLists(kConstructors, method);
        continue;
      }

      if (method.contains("virtual ") || method.contains(" override")) {
        if (method.contains(") const ", Qt::CaseSensitive)) {
          AddStringIntoListOfLists(kVirtualConstantMethods, method);
          continue;
        } else {
          AddStringIntoListOfLists(kVirtualNonConstantMethods, method);
          continue;
        }
      }

      if (method.contains("operator= ", Qt::CaseSensitive)) {
        AddStringIntoListOfLists(kAssignmentOperators, method);
        continue;
      } else if (method.contains("const operator ", Qt::CaseSensitive)) {
        AddStringIntoListOfLists(kConstantOperators, method);
        continue;
      } else if (method.contains(") const", Qt::CaseSensitive)) {
        AddStringIntoListOfLists(kConstantMethods, method);
        continue;
      } else {
        AddStringIntoListOfLists(kNonConstantMethods, method);
        continue;
      }
    } else  // Non-methods
        if (method.contains("using ", Qt::CaseSensitive)) {
      AddStringIntoListOfLists(kUsingDirectives, method);
      continue;
    }
    if (method.contains("typedef ", Qt::CaseSensitive)) {
      AddStringIntoListOfLists(kTypedefs, method);
      continue;
    }
    if (method.contains("enum ", Qt::CaseSensitive)) {
      AddStringIntoListOfLists(kEnums, method);
      continue;
    }
    if (method.contains("struct ##", Qt::CaseSensitive)) {
      AddStringIntoListOfLists(kInlineStructs, method);
      continue;
    }
    if (method.contains("class ##", Qt::CaseSensitive)) {
      AddStringIntoListOfLists(kInlineClasses, method);
      continue;
    }

    // Members
    if (method.contains("static ")) {
      if (method.contains("const ", Qt::CaseSensitive)) {
        AddStringIntoListOfLists(kStaticConstantMembers, method);
        continue;
      } else {
        AddStringIntoListOfLists(kStaticNonConstantMembers, method);
        continue;
      }
    } else {
      if (method.contains("const ", Qt::CaseSensitive)) {
        AddStringIntoListOfLists(kConstantMembers, method);
        continue;
      } else {
        AddStringIntoListOfLists(kNonConstantMembers, method);
        continue;
      }
    }
  }
}

QStringList SectionSorter::SplitSectionIntoMethods(
    const QString &code_section) {
  return code_section.split(QRegExp("[;\.]\n"));  // NOLINT
}

void SectionSorter::AddStringIntoListOfLists(int list_index,
                                             const QString &string) {
  method_groups_[list_index].push_back(string);
}
