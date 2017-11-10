#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  enum ProgramMode { kSort, kFixHeaderGuards, kSortC };
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();
  QString ReorderTextFromString(const QString &text_section);

public slots:
  void ReorderAllTextInFolder();
  void ReorderText();
  void SelectDestinationFolder();
  void SelectSourceFolder();
  void SetMode(int);

private:
  static QString CleanString(const QString &string);

  Ui::MainWindow *ui;

  ProgramMode mode_ = kSort;
};
#endif // MAINWINDOW_H
