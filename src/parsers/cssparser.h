#pragma once
#include <QMap>
#include <QString>

#include "parser.h"

class CSSParser : public Parser {
  using Parser::Parser;

 public:
  QMap<QString, QString> parse_definitions();
};
