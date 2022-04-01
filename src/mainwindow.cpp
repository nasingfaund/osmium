#include "mainwindow.h"

MainWindow::MainWindow(char *argv[], QWidget *parent) : QMainWindow(parent) {
  setup_menubar();

  m_page_widget = new PageWidget(this);
  setCentralWidget(m_page_widget);

  setWindowTitle("Osmium");
  setGeometry(200, 100, 800, 600);
  m_page_widget->navigate(argv[1]);
}

void MainWindow::setup_menubar() {
  QMenu *osmium_menu = new QMenu("Osmium");
  QMenu *settings_menu = new QMenu("Settings");

  QAction *refresh_action = new QAction("Refresh");
  refresh_action->setShortcut(QKeySequence(Qt::Key_F5));
  connect(refresh_action, &QAction::triggered, this,
          [&]() { m_page_widget->navigate(m_page_widget->url()); });
  osmium_menu->addAction(refresh_action);

  QAction *dom_inspector_action = new QAction("DOM Inspector");
  dom_inspector_action->setShortcut(QKeySequence(Qt::Key_F12));
  connect(dom_inspector_action, &QAction::triggered, this, [&]() {
    if (m_page_widget->current_root().type() != NodeType::Null)
      show_dom_inspector();
  });
  osmium_menu->addAction(dom_inspector_action);

  QAction *cookie_inspector_action = new QAction("Cookie Inspector");
  connect(cookie_inspector_action, &QAction::triggered, this,
          [&]() { show_cookie_inspector(); });
  osmium_menu->addAction(cookie_inspector_action);

  osmium_menu->addSeparator();

  QAction *exit_action = new QAction("Exit");
  exit_action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q));
  connect(exit_action, &QAction::triggered, this, QApplication::quit);
  osmium_menu->addAction(exit_action);

  m_cookie_checkbox = new QAction("Send cookies");
  m_cookie_checkbox->setCheckable(true);
  m_cookie_checkbox->setChecked(true);
  settings_menu->addAction(m_cookie_checkbox);

  menuBar()->addMenu(osmium_menu);
  menuBar()->addMenu(settings_menu);
}

void MainWindow::show_dom_inspector() {
  QTreeWidget *tree = new QTreeWidget();
  tree->setHeaderHidden(true);
  tree->addTopLevelItem(render_dom_tree(m_page_widget->current_root()));

  QPushButton *button = new QPushButton("Raw");
  connect(button, &QPushButton::pressed, this, &MainWindow::show_src_dialog);

  QVBoxLayout *layout = new QVBoxLayout();
  layout->addWidget(button);
  layout->addWidget(tree);

  QDialog *dialog = new QDialog();
  dialog->setLayout(layout);
  dialog->setWindowTitle("DOM Inspector");
  dialog->setGeometry(300, 100, 600, 600);
  dialog->show();
}

QTreeWidgetItem *MainWindow::render_dom_tree(Node n) {
  QTreeWidgetItem *item = new QTreeWidgetItem();
  QString content = n.text();

  switch (n.type()) {
    case NodeType::Element:
      item->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));

      for (auto p : n.attrs().keys())
        content += " " + p + "=" + n.attrs()[p];
      item->setText(0, content);
      break;
    case NodeType::TextNode:
      item->setIcon(0, style()->standardIcon(QStyle::SP_FileIcon));
      item->setText(0, content);
      break;
    case NodeType::Null:
      break;
  }

  for (auto c : n.children())
    item->addChild(render_dom_tree(c));

  return item;
}

void MainWindow::show_src_dialog() {
  QDialog *dialog = new QDialog();
  QVBoxLayout *layout = new QVBoxLayout();
  QTextBrowser *browser = new QTextBrowser();
  browser->document()->setPlainText(m_page_widget->current_page_source());
  layout->addWidget(browser);
  dialog->setLayout(layout);
  dialog->setGeometry(350, 150, 800, 600);
  dialog->show();
}

void MainWindow::show_cookie_inspector() {
  QTableWidget *table = new QTableWidget();

  table->setColumnCount(4);
  table->setHorizontalHeaderLabels({"Name", "Value", "Path", "HTTP only?"});
  table->setEditTriggers(QAbstractItemView::NoEditTriggers);

  auto cookies = m_page_widget->jar()->cookiesForUrl(m_page_widget->url());
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
