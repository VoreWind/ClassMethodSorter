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
  QString relevant_code = header_code;
  QString unsortable_bottom = ExtractUnsortableBottomFromCode(relevant_code);
  QString extern_c_opener = ExtractExternCBlockFromCode(relevant_code);
  QString unsortable_top =
      ExtractUnsortableTopFromCode(relevant_code).join("\n");

  QVector<QStringList> groups;
  groups.resize(kBlocksAmount);

  ExtractStructuresFromCode(relevant_code, groups);
  QStringList methods = SplitCodeToMethods(relevant_code);
  PlaceMethodsIntoGroups(methods, groups);

  SortGroups(groups);

  return unsortable_top + "\n\n" + extern_c_opener + "\n" +
         AssembleHeaderBack(groups) + "\n\n" + unsortable_bottom;
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

bool PureCBreaker::IsBlockTypedef(const QString &block) {
  return !block.contains(" struct ") && !block.contains(" enum ") &&
         block.contains(" typedef ");
}

bool PureCBreaker::IsBlockOtherVariable(const QString &block) {
  return !block.contains("(") && !block.contains(" extern ") &&
         !block.contains(" struct ") && !block.contains(" enum ") &&
         !block.contains(" typedef ");
}

QString PureCBreaker::ExtractUnsortableBottomFromCode(QString &code) {
  QString extern_c_block_beginning = "#ifdef __cplusplus";
  int block_beginning_index = code.lastIndexOf("#endif");
  if (code.count(extern_c_block_beginning) == 2) {
    block_beginning_index = code.lastIndexOf(extern_c_block_beginning);
  } else {
    block_beginning_index = code.lastIndexOf("#endif");
  }
  int block_length = code.count() - block_beginning_index;
  QString unsortable_bottom = code.right(block_length);
  code.chop(block_length);
  return unsortable_bottom;
}

QString PureCBreaker::ExtractExternCBlockFromCode(QString &code) {
  QString extern_c_block_beginning =
      "#ifdef __cplusplus\nextern \"C\" {\n#endif";
  if (code.contains(extern_c_block_beginning)) {
    code.remove(extern_c_block_beginning);
    return extern_c_block_beginning;
  } else {
    return QString();
  }
}

QStringList PureCBreaker::ExtractUnsortableTopFromCode(QString &code) {
  QStringList code_lines = code.split("\n");
  QStringList unsortable_list;
  unsortable_list << code_lines.at(0) << code_lines.at(1);
  code_lines.pop_front();
  code_lines.pop_front();

  int if_counter = 0;
  bool insert_empty_lines = false;

  for (const auto &code_line : code_lines) {
    QString clean_line = code_line.trimmed();
    if (clean_line.isEmpty()) {
      if (insert_empty_lines) {
        unsortable_list << code_line;
        continue;
      }
    }

    if (clean_line.startsWith("#include")) {
      unsortable_list << code_line;
      continue;
    }

    if (clean_line.startsWith("#if")) {
      if_counter++;
      unsortable_list << code_line;
      continue;
    }

    if (clean_line.startsWith("#endif")) {
      if_counter--;
      unsortable_list << code_line;
      continue;
    }

    if (if_counter > 0) {
      unsortable_list << code_line;
      continue;
    }

    if (clean_line.startsWith("#define") || clean_line.endsWith("\\")) {
      unsortable_list << code_line;
      continue;
    }
  }

  for (const auto &unsortable_line : unsortable_list) {
    RemoveSingleLineFromCode(code, unsortable_line);
  }
  return unsortable_list;
}

void PureCBreaker::RemoveSingleLineFromCode(QString &code,
                                            const QString &line) {
  QString full_line = line + "\n";
  code.remove(code.indexOf(full_line), full_line.count());
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

QString PureCBreaker::AssembleHeaderBack(QVector<QStringList> groups) {
  QString parsed_code;
  QStringList parsed_groups;
  for (auto group : groups) {
    QString parsed_group = group.join(";\n");
    if (!parsed_group.isEmpty()) {
      parsed_groups << parsed_group;
    }
  }

  parsed_code = parsed_groups.join("\n\n");

  return parsed_code;
}

void PureCBreaker::AddStringIntoListOfLists(int list_index,
                                            const QString &string,
                                            QVector<QStringList> &groups) {
  groups[list_index].push_back(string.trimmed());
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
