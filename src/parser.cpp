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
    if (node.type() != NodeType::Null)
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
  QString tag_name = consume_alphanumeric();

  if (tag_name == "!--") {
    skip_until("-->");
    return Node();
  }

  if (tag_name.startsWith("!")) {
    skip_until(">");
    return Node();
  }

  if (tag_name == "script") {
    skip_until("</script>");
    return Node();
  }

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

  // check if attribute has a value
  if (peek() == '=') {
    assert(consume() == '=');
    consume_whitespace();
    QString value = parse_attribute_value();
    return {name, value};
  } else {
    return {name, ""};
  }
}

QString Parser::parse_attribute_value() {
  QChar quote = peek();
  QString value;

  if (quote == '"' || quote == '\'') {
    assert(consume() == quote);
    while (peek() != quote)
      value.push_back(consume());
    assert(consume() == quote);
  } else {
    while (peek() != '>' && !peek().isSpace())
      value.push_back(consume());
  }

  return value;
}

QMap<QString, QString> Parser::parse_attributes() {
  QMap<QString, QString> attributes;
  while (true) {
    consume_whitespace();
    if (rest().startsWith("/>"))
      consume();
    if (peek() == '>')
      break;
    QPair<QString, QString> pair = parse_attribute();
    attributes[pair.first] = pair.second;
  }
  return attributes;
}

void Parser::skip_until(QString s) {
  while (!rest().startsWith(s))
    consume();
  for (int i = 0; i < s.length(); i++)
    consume();
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
  while (is_alphanumeric(peek()))
    out.push_back(consume());
  return out;
}

QChar Parser::peek() {
  if (eof())
    return 0;
  return m_input[m_pos];
}

bool Parser::eof() { return m_pos >= m_input.length(); }

bool Parser::is_alphanumeric(QChar c) {
  return c.isDigit() || c.isLetter() || c == '!' || c == ':' || c == '-' ||
         c == '_';
}
