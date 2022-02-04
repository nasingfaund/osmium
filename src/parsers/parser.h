#pragma once
#include <QString>

class Parser {
 public:
  Parser(QString input);

 protected:
  QString m_input;
  int m_pos;

  void consume_whitespace();
  QString consume_alphanumeric();
  QString rest();
  void skip_until(QString s);
  QChar consume();
  QChar peek();
  bool is_alphanumeric(QChar c);
  bool eof();
};
