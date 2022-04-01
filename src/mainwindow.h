#pragma once
#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>
#include <QWidget>

#include "dominspector.h"
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
};
