#include "purecbreaker.h"

#include <QDebug>

#include <membersorter.h>
#include <sectionsorter.h>
#include <sorter.h>

QString PureCBreaker::FindRelevantCode(QString &header_code) {
  const QString opening_tag = "#ifdef __cplusplus\nextern \"C\" {\n#endif\n";
  const QString closing_tag = "\n#ifdef __cplusplus\n}";

  int opening_tag_position =
      header_code.indexOf(opening_tag) + opening_tag.count();
  int closing_tag_position = header_code.indexOf(closing_tag);

  if (opening_tag_position == -1 || closing_tag_position == -1) {
    return "";
  } else {
    QString relevant_code = header_code.mid(
        opening_tag_position, closing_tag_position - opening_tag_position);
    header_code.replace(relevant_code, "##relevant_code##");
    return relevant_code;
  }
}

QString PureCBreaker::SortHeader(const QString &header_code) {
  QString non_const_header_code = header_code;
  QString relevant_code = FindRelevantCode(non_const_header_code).trimmed();
  QStringList macros = RemoveMacrosFromCode(relevant_code);
  QStringList methods = SplitCodeToMethods(relevant_code);

  QVector<QStringList> groups = PlaceMethodsIntoGroups(macros, methods);
  SortGroups(groups);
  AssembleHeaderBack(non_const_header_code, groups);
  return non_const_header_code;
}

QStringList PureCBreaker::RemoveMacrosFromCode(QString &relevant_code) {
  const QString define_starter = "#define ";
  QStringList macros;
  int macro_index = relevant_code.indexOf(define_starter);
  while (macro_index != -1) {
    int line_end_position = relevant_code.indexOf("\n", macro_index);
    QString macro =
        relevant_code.mid(macro_index, line_end_position - macro_index);

    while (macro.endsWith("\\")) {
      line_end_position = relevant_code.indexOf("\n", line_end_position + 1);
      macro = relevant_code.mid(macro_index, line_end_position - macro_index);
    }
    macros.push_back(macro);

    relevant_code.remove(macro);
    macro_index = relevant_code.indexOf(define_starter);
  }
  return macros;
}

QStringList PureCBreaker::RemoveStructuresFromCode(QString &relevant_code) {
  return {};
}

QStringList PureCBreaker::SplitCodeToMethods(QString &relevant_code) {
  return relevant_code.split(";", QString::SkipEmptyParts);
}

QVector<QStringList> PureCBreaker::PlaceMethodsIntoGroups(
    const QStringList &macros, const QStringList &methods) {
  QVector<QStringList> groups;
  groups.resize(kBlocksAmount);
  for (auto macro : macros) {
    if (macro.contains("(")) {
      AddStringIntoListOfLists(kMacros, macro, groups);
      continue;
    } else {
      AddStringIntoListOfLists(kDefineCostants, macro, groups);
      continue;
    }
  }

  for (auto method : methods) {
    if (method.contains(" enum")) {
      AddStringIntoListOfLists(kEnums, method, groups);
      continue;
    } else if (method.contains("typedef ")) {
      AddStringIntoListOfLists(kTypedefs, method, groups);
      continue;
    }

    if (method.contains("(")) {
      if (method.contains("static ")) {
        AddStringIntoListOfLists(kStaticMethods, method, groups);
        continue;
      }
      if (method.contains("const ")) {
        AddStringIntoListOfLists(kConstantMethods, method, groups);
        continue;
      }

      AddStringIntoListOfLists(kNonConstantMethods, method, groups);
      continue;
    }

    if (method.contains("static const ")) {
      AddStringIntoListOfLists(kStaticConstantMembers, method, groups);
      continue;
    }
    if (method.contains("static ")) {
      AddStringIntoListOfLists(kStaticNonConstantMembers, method, groups);
      continue;
    }
    if (method.contains("const ")) {
      AddStringIntoListOfLists(kConstantMembers, method, groups);
      continue;
    }

    AddStringIntoListOfLists(kNonConstantMembers, method, groups);
    continue;
  }

  return groups;
}

void PureCBreaker::SortGroups(QVector<QStringList> &groups) {
  for (int i = 0; i < kBlocksAmount; ++i) {
    if (i >= kStaticNonConstantMembers && i < kConstantMembers) {
      MemberSorter sorter;
      QString sorted_string = sorter.SortMembers(groups[i]);
      if (!sorted_string.isEmpty()) {
        groups[i] = QStringList(sorted_string);
      }
    } else {
      std::sort(groups[i].begin(), groups[i].end(), SortingForPureC);
    }
  }
}

void PureCBreaker::AssembleHeaderBack(QString &header_code,
                                      QVector<QStringList> groups) {
  QString parsed_code;
  for (auto group : groups) {
    for (auto element : group) {
      parsed_code.append(element + ";\n");
    }
    parsed_code.append("\n");
  }
  header_code.replace("##relevant_code##", parsed_code);
}

void PureCBreaker::AddStringIntoListOfLists(int list_index,
                                            const QString &string,
                                            QVector<QStringList> &groups) {
  groups[list_index].push_back(string);
}

bool PureCBreaker::SortingForPureC(const QString &left_method,
                                   const QString &right_method) {
  QString left_truncated_method =
      Sorter::TruncateCommentsFromElement(left_method);
  QString right_truncated_method =
      Sorter::TruncateCommentsFromElement(right_method);

  int right_method_string_count = Sorter::ElementStringAmount(right_method);
  int left_method_string_count = Sorter::ElementStringAmount(left_method);

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
      Sorter::ElementStringAmount(right_truncated_method);
  int left_truncated_method_string_count =
      Sorter::ElementStringAmount(left_truncated_method);

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

int PureCBreaker::MethodParamsAmount(const QString &method) {
  return method.count(",");
}

QStringList PureCBreaker::ExtractStructsFromCode(QString &code_block) {
  int struct_token_position = code_block.indexOf("struct");
  while (struct_token_position != -1) {
  }
}
