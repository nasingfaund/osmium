#pragma once
#include <QApplication>
#include <QInputDialog>
#include <QNetworkCookie>
#include <QNetworkCookieJar>
#include <QString>
#include <QTableWidget>
#include <QTreeWidget>
#include <QVBoxLayout>

#include "node.h"

inline QTreeWidgetItem *render_dom_tree(Node n) {
  QTreeWidgetItem *item = new QTreeWidgetItem();
  QString content = n.text();

  switch (n.type()) {
    case NodeType::Element:
      item->setIcon(0, QApplication::style()->standardIcon(QStyle::SP_DirIcon));

      for (auto k : n.attrs().keys())
        content += QString(" %1=\"%2\"").arg(k).arg(n.attrs()[k]);
      item->setText(0, content);
      break;
    case NodeType::TextNode:
      item->setIcon(0,
                    QApplication::style()->standardIcon(QStyle::SP_FileIcon));
      item->setText(0, content.trimmed());
      break;
    case NodeType::Null:
      break;
  }

  for (auto c : n.children())
    item->addChild(render_dom_tree(c));

  return item;
}

inline void show_dom_inspector(Node root) {
  if (root.type() == NodeType::Null)
    return;

  QTreeWidget *tree = new QTreeWidget();
  tree->setHeaderHidden(true);
  tree->addTopLevelItem(render_dom_tree(root));

  QVBoxLayout *layout = new QVBoxLayout();
  layout->addWidget(tree);

  QDialog *dialog = new QDialog();
  dialog->setLayout(layout);
  dialog->setWindowTitle("DOM Inspector");
  dialog->setGeometry(300, 100, 600, 600);
  dialog->show();
}

inline void show_cookie_inspector(QNetworkCookieJar *jar, QString url) {
  QTableWidget *table = new QTableWidget();

  table->setColumnCount(4);
  table->setHorizontalHeaderLabels({"Name", "Value", "Path", "HTTP only?"});
  table->setEditTriggers(QAbstractItemView::NoEditTriggers);

  auto cookies = jar->cookiesForUrl(url);
  table->setRowCount(cookies.length());

  for (auto cookie : cookies) {
    int i = cookies.indexOf(cookie);
    table->setItem(i, 0, new QTableWidgetItem(QString(cookie.name())));
    table->setItem(i, 1, new QTableWidgetItem(QString(cookie.value())));
    table->setItem(i, 2, new QTableWidgetItem(cookie.path()));
    table->setItem(i, 3,
                   new QTableWidgetItem(cookie.isHttpOnly() ? "Yes" : "No"));
  }

  QDialog *dialog = new QDialog();
  QVBoxLayout *layout = new QVBoxLayout();
  layout->addWidget(table);
  dialog->setLayout(layout);
  dialog->setGeometry(350, 150, 800, 300);
  dialog->setWindowTitle("Cookie Inspector");
  dialog->show();
}

inline QString show_proxy_config(QString proxy) {
  bool ok;
  QString new_proxy = QInputDialog::getText(nullptr, "Change Proxy",
                                            "Enter new proxy (host:port)",
                                            QLineEdit::Normal, proxy, &ok);
  return ok ? new_proxy : proxy;
}
