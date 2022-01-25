#pragma once
#include <QLabel>
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
  const QVector<QString> kRenderBlacklist = {"style", "script", "head"};

  QVBoxLayout* m_page_layout;
  QLineEdit* m_urlbar;
  QString m_current_url;
  Node m_current_root;
  QVector<QString> m_history;

  void navigate(QString url);
  void handle_reply(QNetworkReply* reply);
  void render(Node n, Node parent);
  QString make_absolute(QString current_url, QString url);
  void clear_page();
};

class ClickableLabel : public QLabel {
  Q_OBJECT
 public:
  explicit ClickableLabel(QWidget* parent = Q_NULLPTR) : QLabel(parent) {}
  QString href() { return m_href; }
  void setHref(QString url) { m_href = url; }

 signals:
  void clicked();

 protected:
  void mousePressEvent([[maybe_unused]] QMouseEvent* event) { emit clicked(); }

 private:
  QString m_href;
};
