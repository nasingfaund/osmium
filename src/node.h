#pragma once
#include <QJsonArray>
#include <QJsonObject>
#include <QMap>
#include <QVector>

enum NodeType {
  Null,
  Element,
  TextNode,
};

class Node {
 public:
  Node() { m_type = NodeType::Null; }
  Node(QString data) {
    m_type = NodeType::TextNode;
    m_text = data;
  };
  Node(QString tag_name, QMap<QString, QString> attrs, QVector<Node> children) {
    m_type = NodeType::Element;
    m_text = tag_name;
    m_attrs = attrs;
    m_children = children;
  }

  NodeType type() { return m_type; }
  QMap<QString, QString> attrs() { return m_attrs; }
  QVector<Node> children() { return m_children; }
  QString text() { return m_text; }
  QMap<QString, QString> style() { return m_style; }
  void set_style(QMap<QString, QString> s) { m_style = s; }

  size_t count() {
    int c = 1;
    for (auto e : m_children)
      c += e.count();
    return c;
  }
  QJsonObject to_json() {
    auto out = QJsonObject({{"type", m_type}, {"text", m_text}});

    if (m_type == NodeType::Element) {
      QVariantMap attrs;
      QMapIterator<QString, QString> i(m_attrs);
      while (i.hasNext()) {
        i.next();
        attrs.insert(i.key(), i.value());
      }
      out["attrs"] = QJsonObject::fromVariantMap(attrs);

      QJsonArray children;
      for (auto c : m_children)
        children.push_back(c.to_json());
      out["children"] = children;
    }
    return out;
  }

 private:
  NodeType m_type;
  QMap<QString, QString> m_attrs;
  QVector<Node> m_children;
  QString m_text;
  QMap<QString, QString> m_style;
};
