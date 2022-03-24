#pragma once

#include <QTreeWidget>
#include <QVBoxLayout>

#include "node.h"

class DOMInspector : public QWidget {
  Q_OBJECT
 public:
  DOMInspector(Node root, QString src, QWidget* parent = nullptr);

 private:
  QString m_src;

  QTreeWidgetItem* render_tree(Node n);
  void show_dialog();
};
