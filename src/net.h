#pragma once
#include <QImage>
#include <QObject>
#include <QPair>
#include <QString>
#include <QSysInfo>
#include <QUrl>
#include <QtNetwork/QtNetwork>

namespace Net {
inline QString get_user_agent() {
  QString arch = QSysInfo::currentCpuArchitecture();
  QString os = QSysInfo::kernelType()[0].toUpper() +
               QSysInfo::kernelType().mid(1).toLower();

  return QString("Mozilla/5.0 (%1 %2) Osmium/1.0").arg(os).arg(arch);
}

inline QString make_absolute(QString current_url, QString url) {
  if (url.contains("://"))
    return url;

  return QUrl(current_url).resolved(QUrl(url)).toString();
}

inline void apply_proxy(QString proxy, QNetworkAccessManager* nam) {
  if (proxy.length() == 0)
    return;

  auto split = proxy.split(":");

  if (split.length() != 2) {
    qWarning() << "Invalid proxy (use host:port format)";
    return;
  }

  QNetworkProxy p;
  p.setType(QNetworkProxy::HttpProxy);
  p.setHostName(split.at(0));
  p.setPort(split.at(1).toUShort());
  nam->setProxy(p);
}

inline QPair<bool, QImage> load_image_from_url(QString url, QString proxy) {
  QNetworkAccessManager manager;
  apply_proxy(proxy, &manager);
  QNetworkRequest req = QNetworkRequest(QUrl(url));
  req.setHeader(QNetworkRequest::UserAgentHeader, get_user_agent());

  QNetworkReply* reply = manager.get(req);
  QEventLoop loop;
  QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
  loop.exec();

  QImage img;

  QByteArray bytes = reply->readAll();
  bool ok = img.loadFromData(bytes);

  return {ok, img};
}
}  // namespace Net
