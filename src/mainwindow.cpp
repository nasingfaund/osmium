#include "mainwindow.h"

#include "ui_mainwindow.h"

MainWindow::MainWindow(char *argv[], QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  // menubar
  connect(ui->actionRefresh, &QAction::triggered, this,
          [&]() { navigate(m_current_url); });
  connect(ui->actionDOMInspector, &QAction::triggered, this,
          [&]() { Dialog::show_dom_inspector(m_current_root); });
  connect(ui->actionCookie_Inspector, &QAction::triggered, this,
          [&]() { Dialog::show_cookie_inspector(m_jar, m_current_url); });
  connect(ui->actionExit, &QAction::triggered, this, QApplication::quit);

  connect(ui->actionProxy, &QAction::triggered, this,
          [&]() { m_proxy = Dialog::show_proxy_config(m_proxy); });

  // ui
  connect(ui->back_button, &QPushButton::clicked, this, [=]() {
    if (m_history.size() < 2)
      return;
    m_history.pop_back();
    navigate(m_history.back());
    m_history.pop_back();
  });

  connect(ui->refresh_button, &QPushButton::clicked, this,
          [=]() { navigate(m_current_url); });

  connect(ui->urlbar, &QLineEdit::returnPressed, this,
          [=]() { navigate(ui->urlbar->text()); });

  ui->scroll_area->setFrameShape(QFrame::NoFrame);
  ui->page_layout->setAlignment(Qt::AlignTop);

  m_jar = new QNetworkCookieJar();
  navigate(argv[1]);
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::navigate(QString url) {
  if (url == "")
    return;
  if (!url.contains("://"))
    url = "http://" + url;
  QNetworkAccessManager *manager = new QNetworkAccessManager();
  Net::apply_proxy(m_proxy, manager);
  if (ui->actionSend_cookies->isChecked())
    manager->setCookieJar(m_jar);

  QNetworkRequest request(url);
  request.setHeader(QNetworkRequest::UserAgentHeader, Net::get_user_agent());

  ui->statusbar->setText("Navigating to " + url + "...");
  manager->get(request);
  connect(manager, &QNetworkAccessManager::finished, this,
          &MainWindow::handle_reply);
}

void MainWindow::handle_reply(QNetworkReply *reply) {
  // https://doc.qt.io/qt-5/qnetworkreply.html#NetworkError-enum
  if (reply->error() && reply->error() != 201 && reply->error() != 203 &&
      reply->error() != 401) {
    ui->statusbar->setText("Network Error: " + reply->errorString());
    return;
  }

  qint64 start_time = QDateTime::currentMSecsSinceEpoch();

  int status_code =
      reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
  if (300 < status_code && status_code < 400) {
    navigate(Net::make_absolute(m_current_url, reply->rawHeader("Location")));
    return;
  }

  QString body = QString(reply->readAll());

  m_current_url = reply->url().toString();

  m_history.push_back(m_current_url);
  setWindowTitle(m_current_url + " - Osmium");
  ui->urlbar->setText(m_current_url);

  ui->statusbar->setText(QString("Parsing %1 bytes...").arg(body.length()));
  Node root(body);
  if (!reply->hasRawHeader("Content-Type") ||
      reply->rawHeader("Content-Type").startsWith("text/html")) {
    root = parse_html(body);
  }
  reply->deleteLater();

  m_current_root = root;
  clear_page(ui->page_layout);
  new_line();
  ui->statusbar->setText(
      QString("Rendering %1 nodes...").arg(m_current_root.count()));
  render(root, Node());
  new_line();

  qint64 total_time = QDateTime::currentMSecsSinceEpoch() - start_time;
  ui->statusbar->setText(QString("Done in %1ms!").arg(total_time));
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
      QLabel *label = new QLabel("•");
      label->setStyleSheet("color: black; font-weight: bold; font-size: 15px");
      append(label);
    } else if (n.text() == "img") {
      QString url = Net::make_absolute(m_current_url, n.attrs()["src"]);
      QPair<bool, QImage> pair = Net::load_image_from_url(url, m_proxy);
      if (!pair.first) {
        qWarning() << "Couldn't load" << url;
        return;
      }

      QLabel *label = new QLabel();
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

    ClickableLabel *label = new ClickableLabel();
    label->setTextFormat(Qt::PlainText);
    label->setText(content);
    label->setAutoFillBackground(true);

    QFont font = label->font();
    font.setPointSize(11);

    QPalette palette = label->palette();
    palette.setColor(QPalette::WindowText, QColor(0, 0, 0));

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
      palette.setColor(QPalette::WindowText, QColor(40, 161, 209));
      label->setStyleSheet("text-decoration: underline");

      QString href = parent.attrs().value("href");
      label->href = Net::make_absolute(m_current_url, href);

      connect(label, &ClickableLabel::clicked, this,
              std::bind([&](ClickableLabel *label) { navigate(label->href); },
                        label));
    } else if (parent.text() == "title") {
      setWindowTitle(n.text() + " - Osmium");
      return;
    } else if (parent.text() == "button") {
      // TODO: render buttons with non-plaintext content
      QPushButton *button = new QPushButton(n.text());
      button->setStyleSheet("color: black");
      append(button);
      return;
    }

    Style::apply_style(parent.style(), palette, font, m_line);

    label->setPalette(palette);
    label->setFont(font);
    append(label);
  }
}

void MainWindow::append(QWidget *d) { m_line->addWidget(d, 0, Qt::AlignLeft); }

void MainWindow::new_line() {
  if (m_line != nullptr) {
    m_line->addWidget(new QLabel(""));
    ui->page_layout->addLayout(m_line);
  }

  m_line = new QHBoxLayout();
  m_line->setAlignment(Qt::AlignLeft);
}

void MainWindow::clear_page(QLayout *layout) {
  if ((layout == nullptr) || layout->isEmpty())
    return;

  QLayoutItem *item;
  while ((item = layout->itemAt(0))) {
    if (item->layout()) {
      clear_page(item->layout());
      delete item->layout();
    } else if (item->widget()) {
      delete item->widget();
    }
  }
}
