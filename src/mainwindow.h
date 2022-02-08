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
  const QVector<QString> kNewLineBefore = {"h1", "h2", "h3",  "h4",
                                           "h5", "h6", "big", "p"};
  const QVector<QString> kNewLineAfter = {"h1",  "h2", "h3", "h4", "h5", "h6",
                                          "big", "ul", "li", "p",  "tr", "div"};

  QLineEdit* m_urlbar;
  QLabel* m_statusbar;
  QVBoxLayout* m_page_layout;
  QHBoxLayout* m_line;

  QString m_current_url;
  Node m_current_root;
  QVector<QString> m_history;

  void navigate(QString url);
  void handle_reply(QNetworkReply* reply);
  void render(Node n, Node parent);
  void append(QWidget* d);
  void new_line();
  void clear_page(QLayout* layout);
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
