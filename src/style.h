#pragma once
#include <QFont>
#include <QHBoxLayout>
#include <QMap>
#include <QPalette>
#include <QString>

namespace Style {
inline void apply_style(QMap<QString, QString> style, QPalette &palette,
                        QFont &font, QHBoxLayout *line) {
  if (style.count("color")) {
    palette.setColor(QPalette::WindowText, QColor(style["color"]));
  }

  if (style.count("background-color")) {
    palette.setColor(QPalette::Window, QColor(style["background-color"]));
  }

  if (style.count("font-family")) {
    font.setFamily(style["font-family"]);
  }

  if (style.count("font-size")) {
    QString size = style["font-size"];
    if (size.endsWith("px")) {
      size = size.left(size.length() - 2);
      font.setPointSizeF(size.toFloat());
    }
  }

  if (style.count("text-align")) {
    QString text_align = style["text-align"];
    if (text_align == "left")
      line->setAlignment(Qt::AlignLeft);
    else if (text_align == "center")
      line->setAlignment(Qt::AlignCenter);
    else if (text_align == "right")
      line->setAlignment(Qt::AlignRight);
  }
}
}  // namespace Style
