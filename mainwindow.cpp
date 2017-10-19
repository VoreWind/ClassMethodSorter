#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <algorithm>
#include <functional>

#include <QDebug>
#include <QDir>
#include <QDirIterator>
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
  connect(ui->sort_all_button, SIGNAL(pressed()),
          SLOT(ReorderAllTextInFolder()));
}

QString MainWindow::ReorderTextFromString(const QString &text_section) {
  QString non_const_section = text_section;
  QList<ParsedClass> broken_classes =
      ClassBreaker::FindClassBlocksInString(non_const_section);

  for (auto broken_class : broken_classes) {
    ClassBreaker::SortClassSections(broken_class);
    ClassBreaker::AssembleBlockBack(broken_class, non_const_section);
  }

  return text_section;
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
      this, tr("Source Directory"), "/home",
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

void MainWindow::ReorderAllTextInFolder() {
  QDirIterator::IteratorFlag flag = QDirIterator::NoIteratorFlags;
  if (ui->is_recursive_check->isChecked()) {
    flag = QDirIterator::Subdirectories;
  }

  QDirIterator it(ui->source_directory_line->text(), QStringList() << "*.h",
                  QDir::Files, QDirIterator::Subdirectories);

  while (it.hasNext()) {
    QFile source_file(it.next());
    source_file.open(QIODevice::ReadOnly);

    QString parsed_file = ReorderTextFromString(source_file.readAll());
    source_file.close();

    QFile destination_file(ui->destination_directory_line->text() + "/" +
                           source_file.fileName());
    destination_file.open(QIODevice::WriteOnly);

    QTextStream out(&destination_file);
    out << parsed_file;
  }
}
