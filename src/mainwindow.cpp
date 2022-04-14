#include "mainwindow.h"

#include "ui_mainwindow.h"

MainWindow::MainWindow(char *argv[], QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  // menubar
  connect(ui->actionRefresh, &QAction::triggered, this,
          [&]() { navigate(m_current_url); });

  connect(ui->actionDOMInspector, &QAction::triggered, this, [&]() {
    if (m_current_root.type() != NodeType::Null)
      show_dom_inspector();
  });

  connect(ui->actionExit, &QAction::triggered, this, QApplication::quit);

  connect(ui->actionProxy, &QAction::triggered, this,
          &MainWindow::show_proxy_config);

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
  apply_proxy(m_proxy, manager);
  if (ui->actionSend_cookies->isChecked())
    manager->setCookieJar(m_jar);

  QNetworkRequest request(url);
  request.setHeader(QNetworkRequest::UserAgentHeader, get_user_agent());

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
    navigate(make_absolute(m_current_url, reply->rawHeader("Location")));
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
      append(new QLabel("•"));
    } else if (n.text() == "img") {
      QString url = make_absolute(m_current_url, n.attrs()["src"]);
      QPair<bool, QImage> pair = load_image_from_url(url, m_proxy);
      if (!pair.first) {
        qWarning() << "Couldn't load" << url;
        return;
      }

      QLabel *label = new QLabel();
      label->setPixmap(QPixmap::fromImage(pair.second));
      append(label);
    } else if (n.text() == "button") {
      QString content = "";
      if (n.children().length() > 0)
        content = n.children()[0].text();
      append(new QPushButton(content));
      return;
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

    QFont font = label->font();
    font.setPointSize(11);

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
      palette.setColor(QPalette::WindowText, QColor(40, 161, 209));
      label->setPalette(palette);

      QString href = parent.attrs().value("href");
      label->href = make_absolute(m_current_url, href);

      connect(label, &ClickableLabel::clicked, this,
              std::bind([&](ClickableLabel *label) { navigate(label->href); },
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

void MainWindow::show_dom_inspector() {
  QTreeWidget *tree = new QTreeWidget();
  tree->setHeaderHidden(true);
  tree->addTopLevelItem(render_dom_tree(m_current_root));

  QVBoxLayout *layout = new QVBoxLayout();
  layout->addWidget(tree);

  QDialog *dialog = new QDialog();
  dialog->setLayout(layout);
  dialog->setWindowTitle("DOM Inspector");
  dialog->setGeometry(300, 100, 600, 600);
  dialog->show();
}

QTreeWidgetItem *MainWindow::render_dom_tree(Node n) {
  QTreeWidgetItem *item = new QTreeWidgetItem();
  QString content = n.text();

  switch (n.type()) {
    case NodeType::Element:
      item->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));

      for (auto p : n.attrs().keys())
        content += " " + p + "=" + n.attrs()[p];
      item->setText(0, content);
      break;
    case NodeType::TextNode:
      item->setIcon(0, style()->standardIcon(QStyle::SP_FileIcon));
      item->setText(0, content);
      break;
    case NodeType::Null:
      break;
  }

  for (auto c : n.children())
    item->addChild(render_dom_tree(c));

  return item;
}

void MainWindow::show_cookie_inspector() {
  QTableWidget *table = new QTableWidget();

  table->setColumnCount(4);
  table->setHorizontalHeaderLabels({"Name", "Value", "Path", "HTTP only?"});
  table->setEditTriggers(QAbstractItemView::NoEditTriggers);

  auto cookies = m_jar->cookiesForUrl(m_current_url);
  table->setRowCount(cookies.length());

  for (auto cookie : cookies) {
    int i = cookies.indexOf(cookie);
    table->setItem(i, 0, new QTableWidgetItem(QString(cookie.name())));
    table->setItem(i, 1, new QTableWidgetItem(QString(cookie.value())));
    table->setItem(i, 2, new QTableWidgetItem(cookie.path()));
    table->setItem(i, 3,
                   new QTableWidgetItem(cookie.isHttpOnly() ? "Yes" : "No"));
  }

  QDialog *dialog = new QDialog();
  QVBoxLayout *layout = new QVBoxLayout();
  layout->addWidget(table);
  dialog->setLayout(layout);
  dialog->setGeometry(350, 150, 800, 300);
  dialog->setWindowTitle("Cookie Inspector");
  dialog->show();
}

void MainWindow::show_proxy_config() {
  bool ok;
  QString proxy =
      QInputDialog::getText(this, "Change Proxy", "Enter new proxy (host:port)",
                            QLineEdit::Normal, m_proxy, &ok);
  if (ok)
    m_proxy = proxy;
}
