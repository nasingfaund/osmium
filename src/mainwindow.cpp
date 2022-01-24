#include "mainwindow.h"

#include <QDebug>
#include <QFont>
#include <QLabel>
#include <QScrollArea>

#include "parser.h"

MainWindow::MainWindow(char* argv[], QWidget* parent) : QMainWindow(parent) {
  QWidget* widget = new QWidget(this);
  QVBoxLayout* layout = new QVBoxLayout();

  m_urlbar = new QLineEdit();
  connect(m_urlbar, &QLineEdit::returnPressed, this,
          [=]() { navigate(m_urlbar->text()); });
  layout->addWidget(m_urlbar);

  // TODO: add back and refresh button

  QWidget* page_widget = new QWidget();
  QScrollArea* scroll_area = new QScrollArea();
  scroll_area->setFrameShape(QFrame::NoFrame);
  scroll_area->setWidgetResizable(true);
  scroll_area->setWidget(page_widget);

  m_page_layout = new QVBoxLayout();
  m_page_layout->setAlignment(Qt::AlignTop);
  page_widget->setLayout(m_page_layout);
  layout->addWidget(scroll_area);

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
    if (n.text() == "br") {
      m_page_layout->addWidget(new QLabel());
    } else if (n.text() == "img") {
      QNetworkAccessManager manager;
      QNetworkRequest req(QUrl(n.attrs()["src"]));

      QNetworkReply* reply = manager.get(req);
      QEventLoop loop;
      connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
      loop.exec();

      int status_code =
          reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
      if (status_code != 200) {
        qWarning() << "Failed to download" << n.attrs()["src"];
        return;
      }

      QByteArray bytes = reply->readAll();
      QImage img;
      img.loadFromData(bytes);
      QLabel* label = new QLabel();
      label->setPixmap(QPixmap::fromImage(img));
      m_page_layout->addWidget(label);
    }

    for (auto c : n.children())
      render(c, n);
  } else if (n.type() == NodeType::TextNode) {
    if (kRenderBlacklist.contains(parent.text()))
      return;

    QString content = n.text().replace("\n", "");

    ClickableLabel* label = new ClickableLabel();
    label->setTextFormat(Qt::PlainText);
    label->setText(content);

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
    } else if (parent.text() == "a" && parent.attrs().contains("href")) {
      QPalette palette = label->palette();
      palette.setColor(QPalette::WindowText, Qt::blue);
      label->setPalette(palette);

      QString href = parent.attrs().value("href");
      label->setHref(href);

      connect(label, &ClickableLabel::clicked, this,
              std::bind([&](ClickableLabel* label) { navigate(label->href()); },
                        label));
    } else if (parent.text() == "title") {
      setWindowTitle(n.text() + " - Osmium");
    }

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
