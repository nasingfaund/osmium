#include "dominspector.h"

#include <QDebug>

DOMInspector::DOMInspector(Node root, QWidget *parent) : QWidget(parent) {
  setGeometry(300, 100, 600, 600);
  QTreeWidget *tree = new QTreeWidget();
  tree->setHeaderHidden(true);
  tree->addTopLevelItem(render_tree(root));

  QVBoxLayout *layout = new QVBoxLayout();
  layout->addWidget(tree);
  setWindowTitle("DOM Inspector");
  setLayout(layout);
}

QTreeWidgetItem *DOMInspector::render_tree(Node n) {
  QTreeWidgetItem *item = new QTreeWidgetItem();

  if (n.type() == NodeType::Element) {
    item->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
    QString content = n.text();

    for (auto p : n.attrs().keys())
      content += " " + p + "=" + n.attrs()[p];

    item->setText(0, content);
  } else if (n.type() == NodeType::TextNode) {
    item->setIcon(0, style()->standardIcon(QStyle::SP_FileIcon));
    item->setText(0, n.text());
  } else {
    assert(false);
  }

  for (auto c : n.children())
    item->addChild(render_tree(c));

  return item;
}
