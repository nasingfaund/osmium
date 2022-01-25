#pragma once

#include <QTreeWidget>
#include <QVBoxLayout>

#include "node.h"

class DOMInspector : public QWidget {
  Q_OBJECT
 public:
  DOMInspector(Node root, QWidget* parent = nullptr);

 private:
  QTreeWidgetItem* render_tree(Node n);
};
