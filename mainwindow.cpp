#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <algorithm>
#include <functional>

#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFileDialog>
#include <QProcess>
#include <QString>

#include <classbreaker.h>
#include <headerguardfixer.h>
#include <purecbreaker.h>
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
  connect(ui->mode_combo, SIGNAL(currentIndexChanged(int)), SLOT(SetMode(int)));

  SetMode(ui->mode_combo->currentIndex());
}

QString MainWindow::ReorderTextFromString(const QString &text_section) {
  QString non_const_section = text_section;
  QList<ParsedClass> broken_classes =
      ClassBreaker::FindClassBlocksInString(non_const_section);

  if (broken_classes.count() == 0) {
    return QString();
  }

  for (auto broken_class : broken_classes) {
    ClassBreaker::SortClassSections(broken_class);
    ClassBreaker::AssembleClassBack(broken_class, non_const_section);
  }

  return non_const_section;
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::ReorderText() {
  QString text_section = ui->segment_text_edit->toPlainText();
  if (mode_ == kSort) {
    QList<ParsedClass> broken_classes =
        ClassBreaker::FindClassBlocksInString(text_section);

    if (broken_classes.count() == 0) {
      ui->segment_text_edit->setPlainText("");
      return;
    }

    for (auto broken_class : broken_classes) {
      ClassBreaker::SortClassSections(broken_class);
      ClassBreaker::AssembleClassBack(broken_class, text_section);
    }
    ui->segment_text_edit->setPlainText(text_section);
  } else if (mode_ == kFixHeaderGuards) {
    text_section = HeaderGuardFixer::FixHeaderGuardsInText(
        text_section, ui->name_line_edit->text());
  } else if (mode_ == kSortC) {
    text_section = PureCBreaker::SortHeader(text_section);
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
                  QDir::Files, flag);

  while (it.hasNext()) {
    QFile source_file(it.next());
    source_file.open(QIODevice::ReadOnly);
    QString file_contents = source_file.readAll();
    source_file.close();

    QFileInfo source_file_info(source_file.fileName());

    QString subdirectory_path = source_file_info.dir().path().mid(
        ui->source_directory_line->text().count());

    QString parsed_file;
    if (mode_ == kSort) {
      parsed_file = ReorderTextFromString(file_contents);
    } else if (mode_ == kFixHeaderGuards) {
      parsed_file = HeaderGuardFixer::FixHeaderGuardsInText(
          file_contents, source_file_info.fileName());
    } else if (mode_ == kSortC) {
      parsed_file = PureCBreaker::SortHeader(file_contents);
    }

    if (parsed_file.isEmpty()) {
      qDebug() << source_file.fileName();
      continue;
    }

    QString destination_file_name = ui->destination_directory_line->text() +
                                    subdirectory_path + "/" +
                                    source_file_info.fileName();
    QFile destination_file(destination_file_name);

    destination_file.open(QIODevice::WriteOnly);

    QTextStream out(&destination_file);
    out << parsed_file;
    destination_file.close();

    QProcess::startDetached("clang-format-3.8", {"-i", destination_file_name});
  }
}

void MainWindow::SetMode(int mode) {
  if (mode == 0) {
    mode_ = kSort;
    ui->sort_all_button->setText("Sort All");
    ui->sort_button->setText("Sort");
    ui->name_line_edit->setVisible(false);
    ui->name_label->setVisible(false);
  } else if (mode == 1) {
    mode_ = kFixHeaderGuards;
    ui->sort_all_button->setText("Fix All Guards");
    ui->sort_button->setText("Fix Guards");
    ui->name_line_edit->setVisible(true);
    ui->name_label->setVisible(true);
  } else if (mode == 2) {
    mode_ = kSortC;
    ui->sort_all_button->setText("Sort All");
    ui->sort_button->setText("Sort");
    ui->name_line_edit->setVisible(false);
    ui->name_label->setVisible(false);
  }
}
