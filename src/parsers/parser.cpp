#include "parser.h"

Parser::Parser(QString input) {
  m_input = input;
  m_pos = 0;
}

void Parser::skip_until(QString s) {
  while (!rest().startsWith(s))
    consume();
  for (int i = 0; i < s.length(); i++)
    consume();
}

QString Parser::consume_until(QString s) {
  QString o;
  while (!eof() && !rest().startsWith(s))
    o += consume();
  return o;
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

QString Parser::consume_alphanumeric(QString extra_chars) {
  QString out;
  while (is_alphanumeric(peek()) || extra_chars.contains(peek()))
    out.push_back(consume());
  return out;
}

QChar Parser::peek() {
  if (eof())
    return 0;
  return m_input[m_pos];
}

bool Parser::eof() { return m_pos >= m_input.length(); }

bool Parser::is_alphanumeric(QChar c) { return c.isDigit() || c.isLetter(); }
