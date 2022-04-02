#pragma once
#include <QImage>
#include <QObject>
#include <QPair>
#include <QString>
#include <QUrl>
#include <QtNetwork/QtNetwork>

const QString kUserAgent = "Mozilla/5.0 (Unknown) Osmium";

inline QString make_absolute(QString current_url, QString url) {
  if (url.contains("://"))
    return url;

  return QUrl(current_url).resolved(QUrl(url)).toString();
}

inline void apply_proxy(QString proxy, QNetworkAccessManager* nam) {
  if (proxy.length() == 0)
    return;

  QNetworkProxy p;
  p.setType(QNetworkProxy::HttpProxy);
  p.setHostName(proxy.split(":")[0]);
  p.setPort(proxy.split(":")[1].toUInt());
  nam->setProxy(p);
}

inline QPair<bool, QImage> load_image_from_url(QString url, QString proxy) {
  QNetworkAccessManager manager;
  apply_proxy(proxy, &manager);
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
