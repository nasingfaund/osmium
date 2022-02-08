#pragma once
#include <QImage>
#include <QObject>
#include <QPair>
#include <QString>
#include <QUrl>
#include <QtNetwork/QtNetwork>

const QString kUserAgent = "Mozilla/5.0 (Unknown) Osmium";

QString make_absolute(QString current_url, QString url) {
  if (url.contains("://"))
    return url;

  return QUrl(current_url).resolved(QUrl(url)).toString();
}

QPair<bool, QImage> load_image_from_url(QString url) {
  QNetworkAccessManager manager;
  QNetworkRequest req = QNetworkRequest(QUrl(url));
  req.setHeader(QNetworkRequest::UserAgentHeader, kUserAgent);

  QNetworkReply* reply = manager.get(req);
  QEventLoop loop;
  QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
  loop.exec();

  QImage img;

  QByteArray bytes = reply->readAll();
  bool ok = img.loadFromData(bytes);

  return {ok, img};
}
