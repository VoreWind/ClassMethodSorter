#include "classbreaker.h"

#include <QDebug>
#include <QStringList>

#include <sectionsorter.h>

const QStringList ClassBreaker::kSectionNames = {
    "public:",    "signals:",       "public slots:", "protected slots:",
    "protected:", "private slots:", "private:"};

QList<ParsedClass> ClassBreaker::FindClassBlocksInString(QString& block) {
  QList<ParsedClass> block_class_list;
  QRegExp class_token_regexp("(class|struct)[^;]*\\{\n", Qt::CaseSensitive);
  class_token_regexp.setMinimal(true);

  if (DoesBlockContainUnparseableCode(block)) {
    return {};
  }

  int token_position = block.indexOf(class_token_regexp);

  while (token_position != -1) {
    QString class_block;
    ParsedClass parsed_class;

    parsed_class.class_name = FindClassName(block, token_position);
    parsed_class.class_header = FindHeader(block, token_position);

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
        class_block = block.mid(
            open_curvy_brace_position + 2,
            close_curvy_brace_position - open_curvy_brace_position - 1);
        break;
      }
    };

    class_block = InsertSectionToClass(class_block);

    QString first_header_word = parsed_class.class_header.section(" ", 0, 0);
    block.replace(token_position,
                  close_curvy_brace_position - token_position + 1,
                  first_header_word + " ##" + parsed_class.class_name + "##");

    parsed_class.inner_classes = FindClassBlocksInString(class_block);

    QString trimmed_class_block = class_block.trimmed();
    if (IsClassBlockStatringWithSectionToken(trimmed_class_block) &&
        parsed_class.class_header.startsWith("struct")) {
      parsed_class.is_public_section_shown = false;
      trimmed_class_block.prepend("public:\n");
    }

    parsed_class.split_class_body =
        SplitClassBlockToSections(trimmed_class_block);
    block_class_list.push_back(parsed_class);

    token_position = block.indexOf(class_token_regexp);
  };
  return block_class_list;
}

QString ClassBreaker::FindClassName(const QString& block, int token_position) {
  int first_space_position = block.indexOf(" ", token_position);
  first_space_position++;
  int second_space_position = block.indexOf(" ", first_space_position + 1);
  return block.mid(first_space_position,
                   second_space_position - first_space_position);
}

QString ClassBreaker::FindClassHeader(const QString& block,
                                      int token_position) {
  QString edge_token = "{\n";
  int section_position = block.indexOf(edge_token);
  return block.mid(token_position,
                   section_position + edge_token.count() - token_position);
}

QString ClassBreaker::FindStructHeader(const QString& block,
                                       int token_position) {
  QString edge_token = "{\n";
  int section_position = block.indexOf(edge_token);
  return block.mid(token_position,
                   section_position + edge_token.count() - token_position);
}

QString ClassBreaker::FindHeader(const QString& block, int token_position) {
  if (block.mid(token_position).startsWith("struct")) {
    return FindStructHeader(block, token_position);
  } else if (block.mid(token_position).startsWith("class")) {
    return FindClassHeader(block, token_position);
  }
  return QString();
}

bool ClassBreaker::IsClassBlockStatringWithSectionToken(const QString& block) {
  QRegExp section_regexp = SectionFinderRegExp();
  return block.indexOf(section_regexp) != 0;
}

bool ClassBreaker::DoesBlockContainUnparseableCode(QString& block) {
  return block.contains(QRegExp("^.*\\(.*\\).*\\{.*;.*\\}[^;]"));
}

QRegExp ClassBreaker::SectionFinderRegExp() {
  QString regexp_string;
  for (auto section : kSectionNames) {
    regexp_string.append(section + "|");
  }
  regexp_string.chop(1);
  return QRegExp(regexp_string);
}

QList<int> ClassBreaker::BuildSectionPositionList(const QString& class_block) {
  QList<int> section_positions;
  QRegExp section_regexp = SectionFinderRegExp();
  int section_position = class_block.indexOf(section_regexp);
  while (section_position != -1) {
    section_positions.push_back(section_position);
    section_position =
        class_block.indexOf(section_regexp, section_position + 1);
  }

  return section_positions;
}

QVector<QString> ClassBreaker::SplitClassBlockToSections(
    const QString& class_block) {
  QList<int> section_positions = BuildSectionPositionList(class_block);

  QVector<QString> sections;
  sections.resize(kSectionsAmount);

  for (int i = 0; i < section_positions.count(); ++i) {
    int section_position = section_positions.at(i);
    int colon_position = class_block.indexOf(":", section_position);
    colon_position++;
    QString section_name =
        class_block.mid(section_position, colon_position - section_position);

    int next_section_position = i + 1 < section_positions.count()
                                    ? section_positions.at(i + 1) - 1
                                    : class_block.count();

    QString section = class_block.mid(colon_position + 1,
                                      next_section_position - colon_position);

    int section_index = kSectionNames.indexOf(section_name);
    sections[section_index].append(section);
  }

  return sections;
}

QString ClassBreaker::AssembleClassBack(ParsedClass parsed_class,
                                        QString& initial_string) {
  QString assembled_class;
  assembled_class.append(parsed_class.class_header.trimmed());
  assembled_class.append(AssembleBlockFromSections(
      parsed_class.split_class_body, parsed_class.is_public_section_shown));

  for (auto inner_class : parsed_class.inner_classes) {
    AssembleClassBack(inner_class, assembled_class);
  }

  assembled_class.append("\n}");

  QString first_header_word = parsed_class.class_header.section(" ", 0, 0);
  initial_string.replace(
      first_header_word + " ##" + parsed_class.class_name + "##",
      assembled_class);

  return assembled_class;
}

void ClassBreaker::SortClassSections(ParsedClass& parsed_class) {
  for (int i = 0; i < kSectionsAmount; ++i) {
    auto section = parsed_class.split_class_body.at(i);
    if (!section.isEmpty()) {
      SectionSorter sorter(parsed_class.class_name);
      section = sorter.SortSection(section);
      parsed_class.split_class_body.replace(i, section);
    }
  }

  for (int i = 0; i < parsed_class.inner_classes.count(); ++i) {
    auto subclass = parsed_class.inner_classes.at(i);
    SortClassSections(subclass);
    parsed_class.inner_classes.replace(i, subclass);
  }
}

QString ClassBreaker::AssembleBlockFromSections(QVector<QString> sections,
                                                bool is_public_section_shown) {
  QString assembled_block;
  for (int i = 0; i < sections.count(); ++i) {
    bool do_we_show_section_name = is_public_section_shown || i != 0;
    QString section = sections.at(i);
    if (!section.isEmpty()) {
      if (do_we_show_section_name) {
        section.prepend("\n\n " + kSectionNames.at(i));
      }
      assembled_block.append(section);
    }
  }
  return assembled_block;
}

QString ClassBreaker::CleanClassFromMacros(const QString& class_string) {
  QRegExp macro_braced_regexp(" +[A-Z_0-9]+\\(.+\n*.+\\)");
  QRegExp macro_unbraced_regexp("^ +[A-Z_0-9]+\n");
  macro_braced_regexp.setMinimal(true);
  macro_unbraced_regexp.setMinimal(true);

  qDebug() << class_string.contains(macro_unbraced_regexp);

  QString macrossless_string = class_string;
  macrossless_string.remove(macro_braced_regexp);
  macrossless_string.remove(macro_unbraced_regexp);

  return macrossless_string;
}

QString ClassBreaker::InsertSectionToClass(const QString& class_string) {
  QString return_string = class_string;
  QString clean_string = CleanClassFromMacros(class_string).trimmed();
  QString first_line_after_header =
      clean_string.left(clean_string.indexOf("\n"));
  qDebug() << first_line_after_header;
  if (!first_line_after_header.contains(SectionFinderRegExp())) {
    return_string.insert(return_string.indexOf(first_line_after_header),
                         " private:\n");
  }
  return return_string;
}
