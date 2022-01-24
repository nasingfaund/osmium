#include <QApplication>
#include <QDebug>

#include "mainwindow.h"
#include "node.h"

int main(int argc, char *argv[]) {
  Node text1 = Node("Hello World!");
  Node elem1 = Node("h1", {}, {text1});

  /*
  QApplication a(argc, argv);
  MainWindow w;
  w.show();
  return a.exec();
  */
}
