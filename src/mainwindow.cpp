#include "mainwindow.h"

#include <QDebug>
#include <QLabel>
#include <QLineEdit>

#include "node.h"
#include "parser.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
  QWidget* widget = new QWidget(this);
  QVBoxLayout* layout = new QVBoxLayout();

  QLineEdit* urlbar = new QLineEdit();
  layout->addWidget(urlbar);

  m_page_layout = new QVBoxLayout();
  m_page_layout->setAlignment(Qt::AlignTop);
  layout->addLayout(m_page_layout);

  widget->setLayout(layout);
  setGeometry(200, 100, 800, 600);
  setCentralWidget(widget);

  navigate("http://www.google.com");
}

void MainWindow::navigate(QString url) {
  QNetworkAccessManager* manager = new QNetworkAccessManager();

  QNetworkRequest request;
  request.setUrl(url);

  manager->get(request);

  connect(manager, &QNetworkAccessManager::finished, this,
          &MainWindow::handle_reply);
}

void MainWindow::handle_reply(QNetworkReply* reply) {
  if (reply->error()) {
    qWarning() << "Error:" << reply->errorString();
    return;
  }
  qDebug() << "Response:" << reply->readAll();
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
