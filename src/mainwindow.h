#pragma once
#include <QAction>
#include <QDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QMainWindow>
#include <QTableWidget>
#include <QTextBrowser>
#include <QTreeWidgetItem>

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
  ~MainWindow();

 private:
  Ui::MainWindow* ui;

  const QVector<QString> kRenderBlacklist = {"style", "script", "head"};
  const QVector<QString> kNewLineBefore = {"h1", "h2", "h3",  "h4",
                                           "h5", "h6", "big", "p"};
  const QVector<QString> kNewLineAfter = {"h1",  "h2", "h3", "h4", "h5", "h6",
                                          "big", "ul", "li", "p",  "tr", "div"};

  QHBoxLayout* m_line = nullptr;

  QString m_current_url;
  Node m_current_root = Node();
  QVector<QString> m_history;
  QNetworkCookieJar* m_jar;

  void navigate(QString url);
  void handle_reply(QNetworkReply* reply);
  void render(Node n, Node parent);
  void append(QWidget* d);
  void new_line();
  void clear_page(QLayout* layout);

  void show_dom_inspector();
  void show_cookie_inspector();
  QTreeWidgetItem* render_dom_tree(Node n);
};

class ClickableLabel : public QLabel {
  Q_OBJECT
 public:
  explicit ClickableLabel(QWidget* parent = Q_NULLPTR) : QLabel(parent) {}
  QString href;

 signals:
  void clicked();

 protected:
  void mousePressEvent([[maybe_unused]] QMouseEvent* event) { emit clicked(); }
};
