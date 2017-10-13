#include "classbreaker.h"

#include <QDebug>
#include <QStringList>

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
    parsed_class.class_body = class_block;
    SplitClassBlockToSections(class_block);
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
