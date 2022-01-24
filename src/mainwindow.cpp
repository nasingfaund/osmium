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

  m_reply = manager->get(request);
  connect(m_reply, SIGNAL(finished()), this, SLOT(handle_reply()));
  connect(m_reply, SIGNAL(error(QNetworkReply::NetworkError)), this,
          SLOT(handle_reply_error(QNetworkReply::NetworkError)));
}

void MainWindow::handle_reply() { qDebug() << QString(m_reply->readAll()); }

void MainWindow::handle_reply_error(QNetworkReply::NetworkError error) {
  qDebug() << error;
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
