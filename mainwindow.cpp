#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <algorithm>
#include <functional>

#include <QDebug>
#include <QString>

#include <classbreaker.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  connect(ui->sort_button, SIGNAL(pressed()), SLOT(ReorderText()));
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::ReorderText() {
  QString text_section = ui->segment_text_edit->toPlainText();
  text_section = CleanString(text_section);
  QList<ParsedClass> broken_class =
      ClassBreaker::FindClassBlocksInString(text_section);
  ui->segment_text_edit->setPlainText(ReorganizeSection(text_section));
}

QString MainWindow::ReorganizeSection(const QString &code_section) {
  //  QStringList methods = SplitSectionIntoMethods(code_section);
  //  PlaceMethodsIntoGroups(methods);
  //  //  qDebug() << method_groups_;

  //  SortMethodsInGroups();
  //  return AssembleSortedString();
}

QString MainWindow::CleanString(const QString &string) {
  QString clean_string = string;
  while (clean_string.contains("\n\n")) {
    clean_string.replace("\n\n", "\n");
  }
  return clean_string;
}
