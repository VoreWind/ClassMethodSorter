#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  enum ProgramMode { kSort, kFixHeaderGuards };

  explicit MainWindow(QWidget *parent = 0);
  QString ReorderTextFromString(const QString &text_section);
  ~MainWindow();
 public slots:
  void ReorderText();
  void SelectSourceFolder();
  void SelectDestinationFolder();
  void ReorderAllTextInFolder();
  void SetMode(int);

 private:
  static QString CleanString(const QString &string);

  ProgramMode mode_ = kSort;
  Ui::MainWindow *ui;
};
#endif  // MAINWINDOW_H
