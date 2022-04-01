#pragma once
#include <QDebug>
#include <QMap>
#include <QString>

#include "../node.h"
#include "parser.h"

class HTMLParser : public Parser {
  using Parser::Parser;

 public:
  QVector<Node> parse_nodes();

 private:
  const QVector<QString> kVoidElements = {
      "area",  "base", "br",   "col",   "embed",  "hr",    "img",
      "input", "link", "meta", "param", "source", "track", "wbr"};

  Node parse_node();
  Node parse_element();
  Node parse_textnode();
  QMap<QString, QString> parse_attributes();
  QPair<QString, QString> parse_attribute();
  QString parse_attribute_value();
};

Node parse_html(QString input);
