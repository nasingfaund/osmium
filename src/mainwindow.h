#pragma once

#include <QMainWindow>
#include <QVBoxLayout>

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  MainWindow(QWidget* parent = nullptr);

 private:
  void navigate(QString url);
  void clear_page();
  QVBoxLayout* m_page_layout;
};
