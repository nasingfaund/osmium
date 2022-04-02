#pragma once
#include <QApplication>
#include <QDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QMenuBar>
#include <QNetworkCookie>
#include <QNetworkCookieJar>
#include <QNetworkReply>
#include <QPushButton>
#include <QScrollArea>
#include <QTableWidget>
#include <QTextBrowser>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QWidget>

#include "net.h"
#include "node.h"
#include "parsers/cssparser.h"
#include "parsers/htmlparser.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  MainWindow(char* argv[], QWidget* parent = nullptr);
  bool send_cookies() { return m_cookie_checkbox->isChecked(); }
  ~MainWindow();

 private:
  Ui::MainWindow* ui;

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
  QAction* m_cookie_checkbox;

  QString m_current_url;
  QString m_current_src = "";
  Node m_current_root = Node();
  QVector<QString> m_history;
  QNetworkCookieJar* m_jar;

  void setup_menubar();
  void show_dom_inspector();
  void show_cookie_inspector();
  void show_src_dialog();
  QTreeWidgetItem* render_dom_tree(Node n);

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
