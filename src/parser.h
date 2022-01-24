#pragma once
#include <QMap>
#include <QString>

#include "node.h"

class Parser {
 public:
  Parser(QString input);
  QVector<Node> parse_nodes();

 private:
  QString m_input;
  int m_pos;

  Node parse_node();
  Node parse_element();
  Node parse_textnode();
  QMap<QString, QString> parse_attributes();
  QPair<QString, QString> parse_attribute();
  QString parse_attribute_value();

  void consume_whitespace();
  QString consume_alphanumeric();
  QChar consume();
  QChar peek();
  bool eof();
};
