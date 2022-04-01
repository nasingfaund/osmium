#include "mainwindow.h"

MainWindow::MainWindow(char* argv[], QWidget* parent) : QMainWindow(parent) {
  setup_menubar();

  m_page_widget = new PageWidget(this);
  setCentralWidget(m_page_widget);

  setWindowTitle("Osmium");
  setGeometry(200, 100, 800, 600);
  m_page_widget->navigate(argv[1]);
}

void MainWindow::setup_menubar() {
  QMenu* osmium_menu = new QMenu("Osmium");
  QMenu* settings_menu = new QMenu("Settings");

  QAction* refresh_action = new QAction("Refresh");
  refresh_action->setShortcut(QKeySequence(Qt::Key_F5));
  connect(refresh_action, &QAction::triggered, this,
          [&]() { m_page_widget->navigate(m_page_widget->current_url()); });
  osmium_menu->addAction(refresh_action);

  QAction* dom_inspector_action = new QAction("DOM Inspector");
  dom_inspector_action->setShortcut(QKeySequence(Qt::Key_F12));
  connect(dom_inspector_action, &QAction::triggered, this, [&]() {
    if (m_page_widget->current_root().type() == NodeType::Null)
      return;
    DOMInspector* inspector = new DOMInspector(
        m_page_widget->current_root(), m_page_widget->current_page_source());
    inspector->show();
  });
  osmium_menu->addAction(dom_inspector_action);

  osmium_menu->addSeparator();

  QAction* exit_action = new QAction("Exit");
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
