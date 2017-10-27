#include "classbreaker.h"

#include <QDebug>
#include <QStringList>

#include <sectionsorter.h>

const QStringList ClassBreaker::kSectionNames = {
    "public:",    "signals:",       "public slots:", "protected slots:",
    "protected:", "private slots:", "private:"};

QList<ParsedClass> ClassBreaker::FindClassBlocksInString(QString& block) {
  if (DoesBlockContainUnparseableCode(block)) {
    return {};
  }

  QRegExp class_token_regexp("\n\\s*(class|struct)[^;]*\\{\n",
                             Qt::CaseSensitive);
  class_token_regexp.setMinimal(true);
  int token_position = block.indexOf(class_token_regexp) + 1;

  QList<ParsedClass> block_class_list;
  while (token_position != -1) {
    QString class_block;
    ParsedClass parsed_class;

    int open_curvy_brace_position = block.indexOf("{", token_position);

    if (open_curvy_brace_position == -1) {
      return {};
    }

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

    QString header = FindHeader(block, token_position).trimmed();
    QString first_header_word = header.section(" ", 0, 0);
    bool is_class = first_header_word == "class";

    QString new_class_block = InsertSectionToClass(class_block, is_class);
    if (class_block != new_class_block) {
      block.replace(class_block, new_class_block);

      close_curvy_brace_position +=
          new_class_block.count() - class_block.count();

      class_block = new_class_block;

      parsed_class.is_public_section_shown = is_class;
    }

    parsed_class.class_name = FindClassName(block, token_position);
    parsed_class.class_header = FindHeader(block, token_position);
    block.replace(token_position,
                  close_curvy_brace_position - token_position + 1,
                  first_header_word + " ##" + parsed_class.class_name + "##");

    qDebug().noquote() << block;
    parsed_class.inner_classes = FindClassBlocksInString(class_block);

    QString trimmed_class_block = class_block.trimmed();

    parsed_class.split_class_body =
        SplitClassBlockToSections(trimmed_class_block);

    block_class_list.push_back(parsed_class);

    token_position = block.indexOf(class_token_regexp);
  };
  return block_class_list;
}

QString ClassBreaker::FindClassName(const QString& block, int token_position) {
  QString right_block = block.right(block.count() - token_position).trimmed();
  int section = 1;
  QString class_name;
  do {
    class_name = right_block.section(" ", section, section);
    ++section;
  } while (class_name.toUpper() == class_name);
  return class_name;
}

QString ClassBreaker::FindClassHeader(const QString& block,
                                      int token_position) {
  QRegExp section_regexp = SectionFinderRegExp();
  int section_position = block.indexOf(section_regexp);
  return block.mid(token_position, section_position - token_position);
}

QString ClassBreaker::FindStructHeader(const QString& block,
                                       int token_position) {
  QString edge_token = "{\n";
  int section_position = block.indexOf(edge_token);
  return block.mid(token_position,
                   section_position + edge_token.count() - token_position);
}

QString ClassBreaker::FindHeader(const QString& block, int token_position) {
  QString right_block = block.right(block.count() - token_position).trimmed();

  if (right_block.trimmed().startsWith("struct")) {
    return FindStructHeader(block, token_position).trimmed();
  } else if (right_block.startsWith("class")) {
    return FindClassHeader(block, token_position).trimmed();
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

  QString macrossless_string = class_string;
  macrossless_string.remove(macro_braced_regexp);
  macrossless_string.remove(macro_unbraced_regexp);

  return macrossless_string;
}

QString ClassBreaker::InsertSectionToClass(const QString& class_string,
                                           bool is_class) {
  QString return_string = class_string;
  QString clean_string = CleanClassFromMacros(class_string).trimmed();
  QString first_line_after_header =
      clean_string.left(clean_string.indexOf("\n"));
  if (!first_line_after_header.contains(SectionFinderRegExp())) {
    QString token_string = is_class ? "private" : "public";
    return_string.insert(return_string.indexOf(first_line_after_header),
                         token_string + ":\n");
  }
  return return_string;
}
