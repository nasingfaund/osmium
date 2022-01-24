#include "mainwindow.h"

#include <QDebug>
#include <QLabel>
#include <QVBoxLayout>

#include "node.h"
#include "parser.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
  QWidget* widget = new QWidget(this);
  QVBoxLayout* layout = new QVBoxLayout();

  QLabel* label = new QLabel("Hello Osmium!");
  layout->addWidget(label);

  widget->setLayout(layout);
  setGeometry(200, 100, 800, 600);
  setCentralWidget(widget);
}
