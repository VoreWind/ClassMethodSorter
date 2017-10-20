<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.0" language="ru_RU">
<context>
    <name>MainWindow</name>
    <message>
        <location filename="mainwindow.ui" line="14"/>
        <source>MainWindow</source>
        <translation>Сортировка кода</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="28"/>
        <location filename="mainwindow.cpp" line="58"/>
        <source>Source Directory</source>
        <translation>Папка с кодом</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="38"/>
        <location filename="mainwindow.ui" line="55"/>
        <source>Choose Folder</source>
        <translation>Выбор папки</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="45"/>
        <source>Output Directory</source>
        <translation>Папка назначения</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="62"/>
        <source>#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include &lt;QMainWindow&gt;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  explicit MainWindow(QWidget *parent = 0);
  QString ReorderTextFromString(const QString &amp;text_section);
  ~MainWindow();
 public slots:
  void ReorderText();
  void SelectSourceFolder();
  void SelectDestinationFolder();
  void ReorderAllTextInFolder();

 private:
  static QString CleanString(const QString &amp;string);

  Ui::MainWindow *ui;
};
#endif  // MAINWINDOW_H
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="97"/>
        <source>Sort</source>
        <translation>Сортировать</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="104"/>
        <source>Sort All</source>
        <translation>Сортировать все</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="111"/>
        <source>Recursive</source>
        <translation>Рекурсивно</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="67"/>
        <source>Destination Directory</source>
        <translation>Папка назанчения</translation>
    </message>
</context>
</TS>
