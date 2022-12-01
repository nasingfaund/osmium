#include "htmlparser.h"

Node parse_html(QString input) {
  QVector<Node> nodes = HTMLParser(input).parse_nodes();

  if (nodes.size() == 1)
    return nodes[0];
  else
    return Node("html", {}, nodes);
}

QVector<Node> HTMLParser::parse_nodes() {
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

Node HTMLParser::parse_node() {
  switch (peek().toLatin1()) {
    case '<':
      return parse_element();
    default:
      return parse_textnode();
  }
}

Node HTMLParser::parse_textnode() {
  QString content;

  while (!eof() && peek() != '<')
    content.push_back(consume());

  for (auto entity : kHtmlEntities.keys())
    content = content.replace("&" + entity + ";", kHtmlEntities[entity]);

  return Node(content);
}

Node HTMLParser::parse_element() {
  PARSER_ASSERT(consume() == '<');
  QString tag_name = consume_alphanumeric("!-:");

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
  PARSER_ASSERT(consume() == '>');

  if (kVoidElements.contains(tag_name))
    return Node(tag_name, attrs, {});

  QVector<Node> children = parse_nodes();

  if (!eof()) {
    PARSER_ASSERT(consume() == '<');
    PARSER_ASSERT(consume() == '/');
    QString ending_tag_name = consume_alphanumeric("!-:");
    if (ending_tag_name != tag_name)
      qWarning() << "ending tag name is" << ending_tag_name << "should be"
                 << tag_name;
    PARSER_ASSERT(consume() == '>');
  }

  return Node(tag_name, attrs, children);
}

QMap<QString, QString> HTMLParser::parse_attributes() {
  QMap<QString, QString> attributes;

  while (true) {
    consume_whitespace();

    if (rest().startsWith("/>"))
      consume();

    if (peek() == '>')
      break;

    int start_pos = m_pos;
    QPair<QString, QString> pair = parse_attribute();
    PARSER_ASSERT_MSG(m_pos > start_pos, "Invalid attribute name");

    attributes[pair.first] = pair.second;
  }

  return attributes;
}

QPair<QString, QString> HTMLParser::parse_attribute() {
  QString name = consume_alphanumeric("-:_");
  consume_whitespace();

  // check if attribute has a value
  if (peek() == '=') {
    PARSER_ASSERT(consume() == '=');
    consume_whitespace();
    QString value = parse_attribute_value();
    return {name, value};
  } else {
    return {name, ""};
  }
}

QString HTMLParser::parse_attribute_value() {
  QChar quote = peek();
  QString value;

  if (quote == '"' || quote == '\'') {
    PARSER_ASSERT(consume() == quote);
    while (peek() != quote)
      value.push_back(consume());
    PARSER_ASSERT(consume() == quote);
  } else {
    while (peek() != '>' && !peek().isSpace())
      value.push_back(consume());
  }

  return value;
}
