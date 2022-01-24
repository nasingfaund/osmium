#include "mainwindow.h"

#include <QDebug>
#include <QLabel>
#include <QLineEdit>

#include "node.h"
#include "parser.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
  QWidget* widget = new QWidget(this);
  QVBoxLayout* layout = new QVBoxLayout();

  QLineEdit* urlbar = new QLineEdit();
  layout->addWidget(urlbar);

  m_page_layout = new QVBoxLayout();
  m_page_layout->setAlignment(Qt::AlignTop);
  layout->addLayout(m_page_layout);

  widget->setLayout(layout);
  setGeometry(200, 100, 800, 600);
  setCentralWidget(widget);

  navigate("");
}

void MainWindow::navigate(QString url) {
  m_page_layout->addWidget(new QLabel("Hello Osmium! 1"));
  m_page_layout->addWidget(new QLabel("Hello Osmium! 2"));
}

void MainWindow::clear_page() {
  while (true) {
    QLayoutItem* item = m_page_layout->layout()->takeAt(0);
    if (item == NULL)
      break;
    delete item->widget();
    delete item;
  }
}
