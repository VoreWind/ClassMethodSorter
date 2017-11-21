#include "purecbreaker.h"

#include <QDebug>

#include <membersorter.h>
#include <sectionsorter.h>
#include <sorter.h>

const QMap<PureCBreaker::Blocks, bool (*)(const QString &)>
    PureCBreaker::kSortingAssistant = PopulateAssistant();

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
  QStringList includes_list = ExtractIncludesFromRelevantCode(relevant_code);
  int include_insertion_position =
      FindLowestIncludeInIrrelevantCode(non_const_header_code);

  non_const_header_code.insert(include_insertion_position,
                               includes_list.join("\n"));

  QVector<QStringList> groups;
  groups.resize(kBlocksAmount);

  ExtractIfdefMacrosFromCode(relevant_code, groups);
  ExtractMacrosFromCode(relevant_code, groups);
  ExtractStructuresFromCode(relevant_code, groups);

  QStringList methods = SplitCodeToMethods(relevant_code);
  PlaceMethodsIntoGroups(methods, groups);

  SortGroups(groups);
  AssembleHeaderBack(non_const_header_code, groups);
  return non_const_header_code;
}

bool PureCBreaker::IsBlockTypedefEnum(const QString &block) {
  return block.contains(" enum ") && block.contains(" typedef ");
}

bool PureCBreaker::IsBlockEnum(const QString &block) {
  return block.contains(" enum ") && !block.contains(" typedef ");
}

bool PureCBreaker::IsBlockFunction(const QString &block) {
  return block.contains("(");
}

bool PureCBreaker::IsBlockExternVariable(const QString &block) {
  return !block.contains("(") && block.contains(" extern ");
}

bool PureCBreaker::IsBlockOtherVariable(const QString &block) {
  return !block.contains("(") && !block.contains(" extern ") &&
         !block.contains(" struct ") && !block.contains(" enum ") &&
         !block.contains(" typedef ");
}

bool PureCBreaker::IsBlockTypedef(const QString &block) {
  return !block.contains(" struct ") && !block.contains(" enum ") &&
         block.contains(" typedef ");
}

QStringList PureCBreaker::ExtractIncludesFromRelevantCode(
    QString &relevant_code) {
  QRegExp include_regexp("#include [<\"].*[>\"]");
  include_regexp.setMinimal(true);
  QStringList include_list;

  while ((include_regexp.indexIn(relevant_code, 0)) != -1) {
    include_list << include_regexp.cap(0);
    relevant_code.remove(include_regexp.cap(0));
  }

  return include_list;
}

int PureCBreaker::FindLowestIncludeInIrrelevantCode(
    const QString &irrelevant_code) {
  QRegExp include_insertion_regexp("#include [<\"].*[>\"]");
  include_insertion_regexp.setMinimal(true);
  int last_position = include_insertion_regexp.lastIndexIn(irrelevant_code);
  if (last_position == -1) {
    include_insertion_regexp.setPattern("#define [A-Z_]+\n");
    last_position = include_insertion_regexp.indexIn(irrelevant_code);
  }
  last_position += include_insertion_regexp.cap(0).count();
  return last_position;
}

QMap<PureCBreaker::Blocks, bool (*)(const QString &)>
PureCBreaker::PopulateAssistant() {
  QMap<Blocks, bool (*)(const QString &)> assistant;
  assistant.insert(kTypedefEnums, *IsBlockTypedefEnum);
  assistant.insert(kEnums, *IsBlockEnum);
  assistant.insert(kFunctions, *IsBlockFunction);
  assistant.insert(kExternVariables, *IsBlockExternVariable);
  assistant.insert(kOtherVariables, *IsBlockOtherVariable);
  assistant.insert(kTypedefs, *IsBlockTypedef);
  return assistant;
}

void PureCBreaker::ExtractIfdefMacrosFromCode(QString &relevant_code,
                                              QVector<QStringList> &groups) {
  QRegExp ifdef_regexp("#ifn?def[\\w\\s]+\n#[define|undef][\\w\\s]+\n#endif");
  ifdef_regexp.setMinimal(true);

  int ifdef_index = ifdef_regexp.indexIn(relevant_code);
  while (ifdef_index != -1) {
    QString macro = ifdef_regexp.cap(0);
    AddStringIntoListOfLists(kDefineCostants, macro, groups);
    relevant_code.remove(macro);
    ifdef_index = ifdef_regexp.indexIn(relevant_code);
  }
}

void PureCBreaker::ExtractMacrosFromCode(QString &relevant_code,
                                         QVector<QStringList> &groups) {
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

  for (auto macro : macros) {
    if (macro.contains("(")) {
      AddStringIntoListOfLists(kMacros, macro, groups);
      continue;
    } else {
      AddStringIntoListOfLists(kDefineCostants, macro, groups);
      continue;
    }
  }
}

void PureCBreaker::ExtractStructuresFromCode(QString &relevant_code,
                                             QVector<QStringList> &groups) {
  QRegExp typedef_struct_starter = QRegExp("(//[^\n]*\n)*typedef struct");
  typedef_struct_starter.setMinimal(true);
  int starter_index = relevant_code.indexOf(typedef_struct_starter);

  while (starter_index != -1) {
    int close_brace_position =
        FindCloseCurvyBracePositions(starter_index, relevant_code);
    QString struct_block;
    int semicolon_position = relevant_code.indexOf(";", close_brace_position);
    struct_block = relevant_code.mid(starter_index,
                                     semicolon_position - starter_index + 1);
    relevant_code.remove(struct_block);
    struct_block.chop(1);
    starter_index = relevant_code.indexOf(typedef_struct_starter);
    AddStringIntoListOfLists(kTypedefStructs, struct_block, groups);
  }
}

QStringList PureCBreaker::SplitCodeToMethods(QString &relevant_code) {
  return relevant_code.split(";", QString::SkipEmptyParts);
}

void PureCBreaker::PlaceMethodsIntoGroups(const QStringList &methods,
                                          QVector<QStringList> &groups) {
  groups.resize(kBlocksAmount);

  for (const auto &method : methods) {
    QMapIterator<Blocks, bool (*)(const QString &)> i(kSortingAssistant);
    while (i.hasNext()) {
      i.next();
      if (i.value()(method)) {
        AddStringIntoListOfLists(i.key(), method, groups);
        continue;
      }
    }
  }
}

void PureCBreaker::SortGroups(QVector<QStringList> &groups) {
  for (int i = 0; i < kBlocksAmount; ++i) {
    if (i == kExternVariables) {
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
  QStringList parsed_groups;
  for (auto group : groups) {
    QString parsed_group = group.join(";");
    if (!parsed_group.isEmpty()) {
      parsed_groups << parsed_group;
    }
  }

  parsed_code = parsed_groups.join("\n");

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

int PureCBreaker::FindCloseCurvyBracePositions(int token_position,
                                               QString &block) {
  int open_curvy_brace_position = block.indexOf("{", token_position);
  int next_open_curvy_brace_position = open_curvy_brace_position;
  int close_curvy_brace_position = open_curvy_brace_position;

  while (true) {
    if (block.indexOf("}", close_curvy_brace_position + 1) != -1) {
      close_curvy_brace_position =
          block.indexOf("}", close_curvy_brace_position + 1);
    }
    next_open_curvy_brace_position =
        block.indexOf("{", next_open_curvy_brace_position + 1);

    if (next_open_curvy_brace_position > close_curvy_brace_position ||
        next_open_curvy_brace_position == -1) {
      break;
    }
  };

  return close_curvy_brace_position;
}
