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

 private:
  enum Blocks {
    kUsingDirectives,
    kTypedefs,
    kEnums,
    kInlineStructs,
    kInlineClasses,
    kStaticMethods,
    kStaticFactoryMethods,
    kConstructors,
    kDestructors,
    kAssignmentOperators,
    kNonConstantOperators,
    kVirtualNonConstantMethods,
    kNonConstantMethods,
    kConstantOperators,
    kVirtualConstantMethods,
    kConstantMethods,

    kStaticNonConstantMembers,
    kStaticConstantMembers,
    kNonConstantMembers,
    kConstantMembers,

    kFriendClasses,
    kFriendOperators,
    kFriendMethods,
  };

  static int MethodParamsAmount(QString truncated_method);
  static int MethodStringAmount(QString truncated_method);
  static QString TruncateCommentsFromMethod(const QString &method);
  static QString CleanString(const QString &string);

  static bool SortingForMethods(const QString &left_method,
                                const QString &right_method);

  QString ReorganizeSection(const QString &code_section);
  QStringList SplitSectionIntoMethods(const QString &code_section);
  void PlaceMethodsIntoGroups(const QStringList &methods);
  void AddStringIntoListOfLists(int list_index, const QString &string);
  void SortMethodsInGroups();
  QString AssembleSortedString();

  QString FindClassBlocksInString(QString &block);

  static const int kMethodGroupsAmount = 23;
  Ui::MainWindow *ui;
  QString input_string_;
  QVector<QStringList> method_groups_;
};
#endif  // MAINWINDOW_H
