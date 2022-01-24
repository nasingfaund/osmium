#pragma once
#include <QLineEdit>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QtNetwork/QtNetwork>

#include "node.h"

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  MainWindow(char* argv[], QWidget* parent = nullptr);

 private:
  const QVector<QString> kRenderBlacklist = {
      "style",
      "script",
      "head",
  };
  QVBoxLayout* m_page_layout;
  QLineEdit* m_urlbar;

  void navigate(QString url);
  void handle_reply(QNetworkReply* reply);
  void render(Node n, Node parent);
  void clear_page();
};
