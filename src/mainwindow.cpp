#include "mainwindow.h"

#include <QDebug>
#include <QFont>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QShortcut>

#include "dominspector.h"
#include "net.h"
#include "parsers/cssparser.h"
#include "parsers/htmlparser.h"

MainWindow::MainWindow(char* argv[], QWidget* parent) : QMainWindow(parent) {
  QWidget* widget = new QWidget(this);
  QVBoxLayout* layout = new QVBoxLayout();

  setup_menubar();

  // urlbar
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

  // statusbar
  m_statusbar = new QLabel();
  layout->addWidget(m_statusbar);

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

  widget->setLayout(layout);
  setWindowTitle("Osmium");
  setGeometry(200, 100, 800, 600);
  setCentralWidget(widget);

  m_jar = new QNetworkCookieJar();
  navigate(argv[1]);
}

void MainWindow::setup_menubar() {
  QMenuBar* menubar = new QMenuBar();
  QMenu* osmium_menu = new QMenu("Osmium");
  QMenu* settings_menu = new QMenu("Settings");

  QAction* refresh_action = new QAction("Refresh");
  refresh_action->setShortcut(QKeySequence(Qt::Key_F5));
  connect(refresh_action, &QAction::triggered, this,
          [&]() { navigate(m_current_url); });
  osmium_menu->addAction(refresh_action);

  QAction* dom_inspector_action = new QAction("DOM Inspector");
  dom_inspector_action->setShortcut(QKeySequence(Qt::Key_F12));
  connect(dom_inspector_action, &QAction::triggered, this, [&]() {
    DOMInspector* inspector = new DOMInspector(m_current_root);
    inspector->show();
  });
  osmium_menu->addAction(dom_inspector_action);

  osmium_menu->addSeparator();

  QAction* exit_action = new QAction("Exit");
  exit_action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q));
  connect(exit_action, &QAction::triggered, this, QApplication::quit);
  osmium_menu->addAction(exit_action);

  m_cookie_checkbox = new QAction("Send cookies");
  m_cookie_checkbox->setCheckable(true);
  m_cookie_checkbox->setChecked(true);
  settings_menu->addAction(m_cookie_checkbox);

  menubar->addMenu(osmium_menu);
  menubar->addMenu(settings_menu);
  setMenuBar(menubar);
}

void MainWindow::navigate(QString url) {
  if (url == "")
    return;
  if (!url.contains("://"))
    url = "http://" + url;
  QNetworkAccessManager* manager = new QNetworkAccessManager();
  if (m_cookie_checkbox->isChecked())
    manager->setCookieJar(m_jar);

  QNetworkRequest request(url);
  request.setHeader(QNetworkRequest::UserAgentHeader, kUserAgent);

  m_statusbar->setText("Navigating to " + url + "...");
  manager->get(request);
  connect(manager, &QNetworkAccessManager::finished, this,
          &MainWindow::handle_reply);
}

void MainWindow::handle_reply(QNetworkReply* reply) {
  // https://doc.qt.io/qt-5/qnetworkreply.html#NetworkError-enum
  if (reply->error() && reply->error() != 201 && reply->error() != 203 &&
      reply->error() != 401) {
    m_statusbar->setText("Network Error: " + reply->errorString());
    return;
  }

  qint64 start_time = QDateTime::currentMSecsSinceEpoch();

  int status_code =
      reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
  if (300 < status_code && status_code < 400) {
    navigate(make_absolute(m_current_url, reply->rawHeader("Location")));
    return;
  }

  QString body = QString(reply->readAll());

  m_current_url = reply->url().toString();

  m_history.push_back(m_current_url);
  setWindowTitle(m_current_url + " - Osmium");
  m_urlbar->setText(m_current_url);

  m_statusbar->setText(QString("Parsing %1 bytes...").arg(body.length()));
  Node root(body);
  if (!reply->hasRawHeader("Content-Type") ||
      reply->rawHeader("Content-Type").startsWith("text/html")) {
    root = parse(body);
  }
  reply->deleteLater();

  m_current_root = root;
  clear_page(m_page_layout);
  new_line();
  m_statusbar->setText(
      QString("Rendering %1 nodes...").arg(m_current_root.count()));
  render(root, Node());
  new_line();

  qint64 total_time = QDateTime::currentMSecsSinceEpoch() - start_time;
  m_statusbar->setText(QString("Done in %1ms!").arg(total_time));
}

void MainWindow::render(Node n, Node parent) {
  if (n.attrs().count("style"))
    n.set_style(CSSParser(n.attrs()["style"]).parse_definitions());

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

    QRegularExpression ws_regex("\\s+");
    QString content =
        n.text().replace(ws_regex, " ").trimmed().replace("&nbsp;", " ");

    ClickableLabel* label = new ClickableLabel();
    label->setTextFormat(Qt::PlainText);
    label->setText(content);

    QFont font = label->font();
    font.setPointSize(11);
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
      palette.setColor(QPalette::WindowText, QColor(0, 0, 238));
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

    // style rendering
    QPalette palette = label->palette();

    if (parent.style().count("color"))
      palette.setColor(QPalette::WindowText, QColor(parent.style()["color"]));

    if (parent.style().count("background-color")) {
      palette.setColor(label->backgroundRole(),
                       QColor(parent.style()["background-color"]));
      label->setAutoFillBackground(true);
    }

    if (parent.style().count("font-family"))
      font.setFamily(parent.style()["font-family"]);

    if (parent.style().count("font-size")) {
      QString size = parent.style()["font-size"];
      if (size.endsWith("px")) {
        size = size.left(size.length() - 2);
        font.setPointSizeF(size.toFloat());
      }
    }

    if (parent.style().count("text-align")) {
      QString text_align = parent.style()["text-align"];
      if (text_align == "left")
        m_line->setAlignment(Qt::AlignLeft);
      else if (text_align == "center")
        m_line->setAlignment(Qt::AlignCenter);
      else if (text_align == "right")
        m_line->setAlignment(Qt::AlignRight);
    }

    label->setPalette(palette);
    label->setFont(font);
    append(label);
  } else {
    assert(false);
  }
}

void MainWindow::append(QWidget* d) { m_line->addWidget(d, 0, Qt::AlignLeft); }

void MainWindow::new_line() {
  if (m_line != nullptr) {
    m_line->addWidget(new QLabel(""));
    m_page_layout->addLayout(m_line);
  }

  m_line = new QHBoxLayout();
  m_line->setAlignment(Qt::AlignLeft);
}

void MainWindow::clear_page(QLayout* layout) {
  if ((layout == nullptr) || layout->isEmpty())
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
