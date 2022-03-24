#include "dominspector.h"

#include <QPushButton>
#include <QTextBrowser>

DOMInspector::DOMInspector(Node root, QString src, QWidget *parent)
    : QWidget(parent) {
  m_src = src;

  QTreeWidget *tree = new QTreeWidget();
  tree->setHeaderHidden(true);
  tree->addTopLevelItem(render_tree(root));

  QPushButton *button = new QPushButton("Raw");
  connect(button, &QPushButton::pressed, this, &DOMInspector::show_dialog);

  QVBoxLayout *layout = new QVBoxLayout();
  layout->addWidget(button);
  layout->addWidget(tree);
  setWindowTitle("DOM Inspector");
  setGeometry(300, 100, 600, 600);
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

void DOMInspector::show_dialog() {
  QDialog *dialog = new QDialog();
  QVBoxLayout *layout = new QVBoxLayout();
  QTextBrowser *browser = new QTextBrowser();
  browser->document()->setPlainText(m_src);
  layout->addWidget(browser);
  dialog->setLayout(layout);
  dialog->setGeometry(350, 150, 800, 600);
  dialog->show();
}
