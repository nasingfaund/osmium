#include "mainwindow.h"

#include <QDebug>
#include <QFont>
#include <QLabel>

#include "parser.h"

MainWindow::MainWindow(char* argv[], QWidget* parent) : QMainWindow(parent) {
  QWidget* widget = new QWidget(this);
  QVBoxLayout* layout = new QVBoxLayout();

  m_urlbar = new QLineEdit();
  connect(m_urlbar, &QLineEdit::returnPressed, this,
          [=]() { navigate(m_urlbar->text()); });
  layout->addWidget(m_urlbar);

  // TODO: add back button

  m_page_layout = new QVBoxLayout();
  m_page_layout->setAlignment(Qt::AlignTop);
  layout->addLayout(m_page_layout);

  widget->setLayout(layout);
  setGeometry(200, 100, 800, 600);
  setCentralWidget(widget);

  navigate(argv[1]);
}

void MainWindow::navigate(QString url) {
  if (!url.contains("://"))
    url = "http://" + url;
  QNetworkAccessManager* manager = new QNetworkAccessManager();

  QNetworkRequest request;
  request.setUrl(url);

  manager->get(request);
  connect(manager, &QNetworkAccessManager::finished, this,
          &MainWindow::handle_reply);
}

void MainWindow::handle_reply(QNetworkReply* reply) {
  if (reply->error()) {
    // TODO: display in status bar
    qWarning() << "Error:" << reply->errorString();
    return;
  }

  // TODO: follow redirects

  QString body = QString(reply->readAll());
  Node root = parse(body);

  setWindowTitle(reply->url().toString() + " - Osmium");
  m_urlbar->setText(reply->url().toString());
  clear_page();
  render(root, Node());
}

void MainWindow::render(Node n, Node parent) {
  if (n.type() == NodeType::Element) {
    if (n.text() == "br")
      m_page_layout->addWidget(new QLabel());
    // TODO: render img tag

    for (auto c : n.children())
      render(c, n);
  } else if (n.type() == NodeType::TextNode) {
    if (kRenderBlacklist.contains(parent.text()))
      return;

    QLabel* label = new QLabel();
    label->setTextFormat(Qt::PlainText);
    label->setText(n.text().replace("\n", ""));

    QFont font = label->font();
    font.setPointSize(16);
    font.setFamily("Fira Sans");

    if (parent.text() == "h1") {
      font.setWeight(QFont::Bold);
      font.setPointSize(32);
    } else if (parent.text() == "h2") {
      font.setWeight(QFont::Bold);
      font.setPointSize(23);
    } else if (parent.text() == "h3") {
      font.setWeight(QFont::Bold);
      font.setPointSizeF(18.72);
    } else if (parent.text() == "h4" || parent.text() == "b") {
      font.setWeight(QFont::Bold);
    } else if (parent.text() == "i") {
      font.setItalic(true);
    } else if (parent.text() == "u") {
      font.setUnderline(true);
    } else if (parent.text() == "s") {
      font.setStrikeOut(true);
    } else if (parent.text() == "title") {
      setWindowTitle(n.text() + " - Osmium");
    }
    // TODO: render a tag

    label->setFont(font);
    m_page_layout->addWidget(label);
  } else {
    assert(false);
  }
}

void MainWindow::clear_page() {
  while (true) {
    QLayoutItem* item = m_page_layout->layout()->takeAt(0);
    if (item == NULL)
      break;
    delete item->widget();
    delete item;
  }
}
