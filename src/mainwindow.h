#pragma once

#include <QMainWindow>
#include <QVBoxLayout>
#include <QtNetwork/QtNetwork>

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  MainWindow(QWidget* parent = nullptr);

 private:
  QVBoxLayout* m_page_layout;

  void navigate(QString url);
  void handle_reply(QNetworkReply* reply);
  void clear_page();
};
