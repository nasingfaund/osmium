#pragma once
#include <QDebug>
#include <QString>

#define PARSER_ASSERT_MSG(x, msg)                                             \
  if (!static_cast<bool>(x)) {                                                \
    qCritical() << "ASSERTION FAILED in" << __FILE__ << "in line" << __LINE__ \
                << msg;                                                       \
    qCritical() << m_input.mid(m_pos - 5, 10);                                \
    exit(1);                                                                  \
  }
#define PARSER_ASSERT(x) PARSER_ASSERT_MSG(x, "")

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
  QString consume_until(QString s);
  QChar consume();
  QChar peek();
  bool is_alphanumeric(QChar c);
  bool eof();
};
