#pragma once

#include <QTreeWidget>
#include <QVBoxLayout>

#include "node.h"

class DOMInspector : public QWidget {
  Q_OBJECT
 public:
  DOMInspector(Node root, QWidget* parent = nullptr);

 private:
  Node m_root;
  QTreeWidgetItem* render_tree(Node n);
  void show_dialog();
};
