#include "mainwindow.h"

#include <QDebug>
#include <QFont>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QShortcut>

#include "dominspector.h"
#include "net.h"
#include "parser.h"

MainWindow::MainWindow(char* argv[], QWidget* parent) : QMainWindow(parent) {
  QWidget* widget = new QWidget(this);
  QVBoxLayout* layout = new QVBoxLayout();

  // bar layout
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

  // page layout
  QWidget* page_widget = new QWidget();
  QScrollArea* scroll_area = new QScrollArea();
  scroll_area->setFrameShape(QFrame::NoFrame);
  scroll_area->setWidgetResizable(true);
  scroll_area->setWidget(page_widget);

  m_page_layout = new QVBoxLayout();
  m_page_layout->setAlignment(Qt::AlignTop);
  page_widget->setLayout(m_page_layout);
  layout->addWidget(scroll_area);

  // shortcuts
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

  clear_page(m_page_layout);
  new_line();
  render(root, Node());
  new_line();
}

void MainWindow::render(Node n, Node parent) {
  if (n.type() == NodeType::Element) {
    if (kNewLineBefore.contains(n.text()))
      new_line();

    if (n.text() == "br") {
      new_line();
    } else if (n.text() == "li") {
      append(new QLabel("•"));
    } else if (n.text() == "img") {
      QString url = make_absolute(m_current_url, n.attrs()["src"]);
      QPair<bool, QImage> pair = load_image_from_url(url);
      if (!pair.first) {
        qWarning() << "Couldn't load" << url;
        return;
      }

      QLabel* label = new QLabel();
      label->setPixmap(QPixmap::fromImage(pair.second));
      append(label);
    }

    for (auto c : n.children())
      render(c, n);

    if (kNewLineAfter.contains(n.text()))
      new_line();
  } else if (n.type() == NodeType::TextNode) {
    if (kRenderBlacklist.contains(parent.text()))
      return;

    QString content = n.text().replace("\n", "").trimmed();

    ClickableLabel* label = new ClickableLabel();
    label->setTextFormat(Qt::PlainText);
    label->setText(content);

    QFont font = label->font();
    font.setPointSize(14);
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
      return;
    }

    label->setFont(font);
    append(label);
  } else {
    assert(false);
  }
}

void MainWindow::append(QWidget* d) { m_line->addWidget(d, 0, Qt::AlignLeft); }

void MainWindow::new_line() {
  m_page_layout->addLayout(m_line);
  m_line = new QHBoxLayout();
  m_line->setAlignment(Qt::AlignLeft);
}

void MainWindow::clear_page(QLayout* layout) {
  if ((layout == nullptr) || (layout->isEmpty()))
    return;

  QLayoutItem* item;
  while ((item = layout->itemAt(0))) {
    if (item->layout()) {
      clear_page(item->layout());
      delete item->layout();
    } else if (item->widget()) {
      delete item->widget();
    }
  }
}
