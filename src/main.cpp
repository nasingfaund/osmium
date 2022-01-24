#include <QApplication>
#include <QDebug>

#include "mainwindow.h"
#include "node.h"
#include "parser.h"

int main(int argc, char *argv[]) {
  Node root = parse(
      "<html><body a='b'><h1>Hello World!</h1><test>asd</test></body></html>");
  qDebug() << root.children()[0].attrs();
  qDebug() << root.children()[0].children()[1].children()[0].text();

  /*
  QApplication a(argc, argv);
  MainWindow w;
  w.show();
  return a.exec();
  */
}
