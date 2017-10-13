#include "classbreaker.h"

#include <QDebug>
#include <QStringList>

const QStringList ClassBreaker::section_names = {
    "public:",    "signals:",       "public slots:", "protected slots:",
    "protected:", "private slots:", "private:"};

QList<ParsedClass> ClassBreaker::FindClassBlocksInString(QString& block) {
  QList<ParsedClass> block_class_list;
  int token_position = block.indexOf("class ", Qt::CaseSensitive);

  while (token_position != -1) {
    QString class_block;
    ParsedClass parsed_class;

    parsed_class.class_name = FindClassName(block, token_position);
    qDebug() << parsed_class.class_name;

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
    parsed_class.inner_classes = FindClassBlocksInString(class_block);
    block_class_list.push_back(parsed_class);
    qDebug() << class_block;
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
  for (auto section : section_names) {
    regexp_string.append(section + "|");
  }
  regexp_string.chop(1);
  return QRegExp(regexp_string);
}
