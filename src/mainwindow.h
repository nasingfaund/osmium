#pragma once
#include <QApplication>
#include <QDialog>
#include <QMainWindow>
#include <QMenuBar>
#include <QTableWidget>
#include <QTextBrowser>
#include <QTreeWidget>
#include <QWidget>

#include "node.h"
#include "pagewidget.h"

class PageWidget;

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  MainWindow(char* argv[], QWidget* parent = nullptr);
  bool send_cookies() { return m_cookie_checkbox->isChecked(); }

 private:
  PageWidget* m_page_widget;
  QAction* m_cookie_checkbox;

  void setup_menubar();
  void show_dom_inspector();
  void show_cookie_inspector();
  void show_src_dialog();
  QTreeWidgetItem* render_dom_tree(Node n);
};
