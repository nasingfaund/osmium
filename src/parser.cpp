#include "parser.h"

Node parse(QString input) {
  QVector<Node> nodes = Parser(input).parse_nodes();

  if (nodes.size() == 1)
    return nodes[0];
  else
    return Node("html", {}, nodes);
}

Parser::Parser(QString input) {
  m_input = input;
  m_pos = 0;
}

QVector<Node> Parser::parse_nodes() {
  QVector<Node> nodes;

  while (true) {
    consume_whitespace();
    if (eof() || rest().startsWith("</"))
      break;
    Node node = parse_node();
    nodes.push_back(node);
  }
  return nodes;
}

Node Parser::parse_node() {
  switch (peek().toLatin1()) {
    case '<':
      return parse_element();
    default:
      return parse_textnode();
  }
}

Node Parser::parse_textnode() {
  QString content;
  while (peek() != '<')
    content.push_back(consume());
  return Node(content);
}

Node Parser::parse_element() {
  assert(consume() == '<');
  // TODO: skip comments
  QString tag_name = consume_alphanumeric();
  QMap<QString, QString> attrs = parse_attributes();
  assert(consume() == '>');

  if (kVoidElements.contains(tag_name))
    return Node(tag_name, attrs, {});

  QVector<Node> children = parse_nodes();

  assert(consume() == '<');
  assert(consume() == '/');
  consume_alphanumeric();
  assert(consume() == '>');

  return Node(tag_name, attrs, children);
}

QPair<QString, QString> Parser::parse_attribute() {
  QString name = consume_alphanumeric();
  consume_whitespace();
  // TODO: allow attributes without values
  assert(consume() == '=');
  consume_whitespace();
  QString value = parse_attribute_value();
  return {name, value};
}

QString Parser::parse_attribute_value() {
  QChar quote = consume();
  // TODO: allow attributes without quotes
  assert(quote == '"' || quote == '\'');

  QString value;
  while (peek() != quote)
    value += consume();
  assert(consume() == quote);
  return value;
}

QMap<QString, QString> Parser::parse_attributes() {
  QMap<QString, QString> attributes;
  while (true) {
    consume_whitespace();
    if (peek() == '>')
      break;
    QPair<QString, QString> pair = parse_attribute();
    attributes[pair.first] = pair.second;
  }
  return attributes;
}

QString Parser::rest() { return m_input.mid(m_pos); }

QChar Parser::consume() {
  QChar c = peek();
  m_pos++;
  return c;
}

void Parser::consume_whitespace() {
  while (peek().isSpace())
    consume();
}

QString Parser::consume_alphanumeric() {
  QString out;
  while (peek().isDigit() || peek().isLetter())
    out.push_back(consume());
  return out;
}

QChar Parser::peek() {
  if (eof())
    return 0;
  return m_input[m_pos];
}

bool Parser::eof() { return m_pos >= m_input.length(); }
