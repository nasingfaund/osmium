#pragma once

#include <QMainWindow>
#include <QVBoxLayout>
#include <QtNetwork/QtNetwork>

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  MainWindow(QWidget* parent = nullptr);

 private:
  QNetworkReply* m_reply;
  QVBoxLayout* m_page_layout;

  void navigate(QString url);
  void clear_page();

 private slots:
  void handle_reply();
  void handle_reply_error(QNetworkReply::NetworkError error);
};
