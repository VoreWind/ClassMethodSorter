#include "classbreaker.h"

#include <QDebug>
#include <QStringList>

#include <sectionsorter.h>

const QStringList ClassBreaker::kSectionNames = {
    "public:",    "signals:",       "public slots:", "protected slots:",
    "protected:", "private slots:", "private:"};

QList<ParsedClass> ClassBreaker::FindClassBlocksInString(QString& block) {
  QList<ParsedClass> block_class_list;
  int token_position = block.indexOf("class ", Qt::CaseSensitive);

  while (token_position != -1) {
    QString class_block;
    ParsedClass parsed_class;

    parsed_class.class_name = FindClassName(block, token_position);
    parsed_class.class_header = FindClassHeader(block, token_position);

    int open_curvy_brace_position = block.indexOf("{", token_position);
    int next_open_curvy_brace_position = open_curvy_brace_position;
    int close_curvy_brace_position = open_curvy_brace_position;

    while (true) {
      close_curvy_brace_position =
          block.indexOf("};", close_curvy_brace_position + 1);
      next_open_curvy_brace_position =
          block.indexOf("{", next_open_curvy_brace_position + 1);

      if (next_open_curvy_brace_position > close_curvy_brace_position ||
          next_open_curvy_brace_position == -1) {
        class_block = block.mid(token_position,
                                close_curvy_brace_position - token_position);
        break;
      }
    };
    block.replace(token_position,
                  close_curvy_brace_position - token_position + 1,
                  "class ##" + parsed_class.class_name + "##");
    parsed_class.split_class_body = SplitClassBlockToSections(class_block);
    parsed_class.inner_classes = FindClassBlocksInString(class_block);
    block_class_list.push_back(parsed_class);
    //    qDebug() << class_block;
    token_position =
        block.indexOf("class ", close_curvy_brace_position, Qt::CaseSensitive);
  };
  return block_class_list;
}

QString ClassBreaker::FindClassName(const QString& block, int token_position) {
  int first_space_position = block.indexOf(" ", token_position);
  // Придумать, как это покрасивее описать.
  first_space_position++;
  int second_space_position = block.indexOf(" ", first_space_position + 1);
  return block.mid(first_space_position,
                   second_space_position - first_space_position);
}

QString ClassBreaker::FindClassHeader(const QString& block,
                                      int token_position) {
  int brace_position = block.indexOf("{", token_position);
  // Придумать, как это покрасивее описать.
  brace_position++;
  return block.mid(token_position, brace_position - token_position);
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
    int space_position =
        class_block.indexOf(QRegExp("[ \n]"), section_position);
    QString section_name =
        class_block.mid(section_position, space_position - section_position);

    int next_section_position = i + 1 < section_positions.count()
                                    ? section_positions.at(i + 1) - 1
                                    : class_block.count();

    QString section = class_block.mid(space_position + 1,
                                      next_section_position - space_position);

    int section_index = kSectionNames.indexOf(section_name);
    sections[section_index].append(section);
    qDebug() << section;
  }

  return sections;
}

QString ClassBreaker::AssembleBlockBack(ParsedClass parsed_class,
                                        QString& initial_string) {
  QString assembled_class;
  assembled_class.append(parsed_class.class_header);
  assembled_class.append("\n");
  assembled_class.append(
      AssembleBlockFromSections(parsed_class.split_class_body));

  for (auto inner_class : parsed_class.inner_classes) {
    AssembleBlockBack(inner_class, assembled_class);
  }
  assembled_class.append("}");
  initial_string.replace("class ##" + parsed_class.class_name + "##",
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
}

QString ClassBreaker::AssembleBlockFromSections(QVector<QString> sections) {
  QString assembled_block;
  for (int i = 0; i < sections.count(); ++i) {
    QString section = sections.at(i);
    if (!section.isEmpty()) {
      section.prepend(" " + kSectionNames.at(i));
      assembled_block.append(section);
    }
  }
  return assembled_block;
}
