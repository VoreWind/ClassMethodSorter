#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <algorithm>
#include <functional>

#include <QDebug>
#include <QString>

#include <classbreaker.h>
#include <sectionsorter.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  connect(ui->sort_button, SIGNAL(pressed()), SLOT(ReorderText()));
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::ReorderText() {
  QString text_section = ui->segment_text_edit->toPlainText();
  QList<ParsedClass> broken_classes =
      ClassBreaker::FindClassBlocksInString(text_section);

  for (auto broken_class : broken_classes) {
    ClassBreaker::SortClassSections(broken_class);
    ClassBreaker::AssembleBlockBack(broken_class, text_section);
  }
  ui->segment_text_edit->setPlainText(text_section);
}
