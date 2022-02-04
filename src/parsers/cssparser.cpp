#include "cssparser.h"

QMap<QString, QString> CSSParser::parse_definitions() {
  QMap<QString, QString> defs;
  while (true) {
    consume_whitespace();
    if (eof())
      break;
    QPair<QString, QString> def = parse_definition();
    defs[def.first] = def.second;
  }

  return defs;
}

QPair<QString, QString> CSSParser::parse_definition() {
  QString name = consume_alphanumeric();
  assert(consume() == ':');
  consume_whitespace();

  QString value = consume_until(";");
  if (peek() == ';')
    consume();

  return {name, value};
}
