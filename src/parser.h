#pragma once
#include <QDebug>
#include <QMap>
#include <QString>

#include "node.h"

class Parser {
 public:
  Parser(QString input);
  QVector<Node> parse_nodes();

 private:
  const QVector<QString> kVoidElements = {
      "area",  "base", "br",   "col",   "embed",  "hr",    "img",
      "input", "link", "meta", "param", "source", "track", "wbr"};

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
  QString rest();
  void skip_until(QString s);
  QChar consume();
  QChar peek();
  bool is_alphanumeric(QChar c);
  bool eof();
};

Node parse(QString input);
