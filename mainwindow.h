#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  explicit MainWindow(QWidget *parent = 0);

  ~MainWindow();
 public slots:
  void ReorderText();
  void SelectSourceFolder();
  void SelectDestinationFolder();

 private:
  static QString CleanString(const QString &string);

  Ui::MainWindow *ui;
};
#endif  // MAINWINDOW_H
