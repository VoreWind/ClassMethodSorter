#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <algorithm>
#include <functional>

#include <QDebug>
#include <QFileDialog>
#include <QString>

#include <classbreaker.h>
#include <sectionsorter.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  connect(ui->sort_button, SIGNAL(pressed()), SLOT(ReorderText()));
  connect(ui->source_directory_button, SIGNAL(pressed()),
          SLOT(SelectSourceFolder()));
  connect(ui->destination_directory_button, SIGNAL(pressed()),
          SLOT(SelectDestinationFolder()));
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

void MainWindow::SelectSourceFolder() {
  QString dir = QFileDialog::getExistingDirectory(
      this, tr("Source Directory"), "/home/",
      QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

  ui->source_directory_line->setText(dir);
  ui->destination_directory_line->setText(dir);
}

void MainWindow::SelectDestinationFolder() {
  QString dir = QFileDialog::getExistingDirectory(
      this, tr("Destination Directory"), "/home/",
      QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

  ui->destination_directory_line->setText(dir);
}
