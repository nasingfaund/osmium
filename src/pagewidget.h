#pragma once
#include <QLabel>
#include <QLineEdit>
#include <QNetworkReply>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

#include "mainwindow.h"
#include "net.h"
#include "node.h"
#include "parsers/cssparser.h"
#include "parsers/htmlparser.h"

class MainWindow;

class PageWidget : public QWidget {
  Q_OBJECT
 public:
  explicit PageWidget(MainWindow* parent);
  void navigate(QString url);

  QString current_page_source() { return m_current_src; }
  QString url() { return m_current_url; }
  Node current_root() { return m_current_root; }
  QNetworkCookieJar* jar() { return m_jar; }

 private:
  const QVector<QString> kRenderBlacklist = {"style", "script", "head"};
  const QVector<QString> kNewLineBefore = {"h1", "h2", "h3",  "h4",
                                           "h5", "h6", "big", "p"};
  const QVector<QString> kNewLineAfter = {"h1",  "h2", "h3", "h4", "h5", "h6",
                                          "big", "ul", "li", "p",  "tr", "div"};

  MainWindow* m_parent;
  QLineEdit* m_urlbar;
  QLabel* m_statusbar;
  QVBoxLayout* m_page_layout;
  QHBoxLayout* m_line = nullptr;

  QString m_current_url;
  QString m_current_src = "";
  Node m_current_root = Node();
  QVector<QString> m_history;
  QNetworkCookieJar* m_jar;

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
