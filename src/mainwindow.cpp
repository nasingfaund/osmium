#include "mainwindow.h"

#include <QDebug>
#include <QFont>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QShortcut>

#include "dominspector.h"
#include "parser.h"

MainWindow::MainWindow(char* argv[], QWidget* parent) : QMainWindow(parent) {
  QWidget* widget = new QWidget(this);
  QVBoxLayout* layout = new QVBoxLayout();

  QHBoxLayout* bar_layout = new QHBoxLayout();

  QPushButton* back_button = new QPushButton("←");
  back_button->setFont(QFont("Fira Code", 20));
  back_button->setMaximumWidth(40);
  back_button->setMaximumHeight(40);
  connect(back_button, &QPushButton::clicked, this, [=]() {
    if (m_history.size() < 2)
      return;
    m_history.pop_back();
    navigate(m_history.back());
    m_history.pop_back();
  });
  bar_layout->addWidget(back_button);

  QPushButton* refresh_button = new QPushButton("↻");
  refresh_button->setFont(QFont("Fira Code", 20));
  refresh_button->setMaximumWidth(40);
  refresh_button->setMaximumHeight(40);
  connect(refresh_button, &QPushButton::clicked, this,
          [=]() { navigate(m_current_url); });
  bar_layout->addWidget(refresh_button);

  m_urlbar = new QLineEdit();
  m_urlbar->setMinimumHeight(40);
  connect(m_urlbar, &QLineEdit::returnPressed, this,
          [=]() { navigate(m_urlbar->text()); });
  bar_layout->addWidget(m_urlbar);

  layout->addLayout(bar_layout);

  QWidget* page_widget = new QWidget();
  QScrollArea* scroll_area = new QScrollArea();
  scroll_area->setFrameShape(QFrame::NoFrame);
  scroll_area->setWidgetResizable(true);
  scroll_area->setWidget(page_widget);

  m_page_layout = new QVBoxLayout();
  m_page_layout->setAlignment(Qt::AlignTop);
  page_widget->setLayout(m_page_layout);
  layout->addWidget(scroll_area);

  QShortcut* f5 = new QShortcut(Qt::Key_F5, this);
  connect(f5, &QShortcut::activated, this, [&]() { navigate(m_current_url); });

  QShortcut* f12 = new QShortcut(Qt::Key_F12, this);
  connect(f12, &QShortcut::activated, this, [&]() {
    DOMInspector* inspector = new DOMInspector(m_current_root);
    inspector->show();
  });

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
  // https://doc.qt.io/qt-5/qnetworkreply.html#NetworkError-enum
  if (reply->error() && reply->error() != 203 && reply->error() != 401) {
    qWarning() << "Error:" << reply->errorString();
    return;
  }

  int status_code =
      reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
  if (300 < status_code && status_code < 400) {
    navigate(make_absolute(m_current_url, reply->rawHeader("Location")));
    return;
  }

  QString body = QString(reply->readAll());
  Node root = parse(body);

  m_current_url = reply->url().toString();
  m_current_root = root;

  m_history.push_back(m_current_url);
  setWindowTitle(m_current_url + " - Osmium");
  m_urlbar->setText(m_current_url);

  clear_page();
  render(root, Node());
}

void MainWindow::render(Node n, Node parent) {
  if (n.type() == NodeType::Element) {
    if (n.text() == "br") {
      m_page_layout->addWidget(new QLabel());
    } else if (n.text() == "img") {
      QString url = make_absolute(m_current_url, n.attrs()["src"]);

      QNetworkAccessManager manager;
      QNetworkRequest req = QNetworkRequest(QUrl(url));

      QNetworkReply* reply = manager.get(req);
      QEventLoop loop;
      connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
      loop.exec();

      int status_code =
          reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
      if (status_code != 200) {
        qWarning() << "Failed to download" << url;
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
      label->setHref(make_absolute(m_current_url, href));

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

// FIXME: move it somewhere
QString MainWindow::make_absolute(QString current_url, QString url) {
  if (url.contains("://"))
    return url;

  return QUrl(current_url).resolved(QUrl(url)).toString();
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
