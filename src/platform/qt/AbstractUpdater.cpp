/* Copyright (c) 2013-2019 Jeffrey Pfau
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "AbstractUpdater.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>

using namespace QGBA;

AbstractUpdater::AbstractUpdater(QObject* parent)
	: QObject(parent)
	, m_netman(new QNetworkAccessManager(this))
{
}

void AbstractUpdater::checkUpdate() {
	QNetworkReply* reply = m_netman->get(QNetworkRequest(manifestLocation()));
	chaseRedirects(reply, &AbstractUpdater::manifestDownloaded);
}

void AbstractUpdater::downloadUpdate() {
	if (m_isUpdating) {
		return;
	}
	if (m_manifest.isEmpty()) {
		m_isUpdating = true;
		checkUpdate();
		return;
	}
	QUrl url = parseManifest(m_manifest);
	if (!url.isValid()) {
		emit updateDone(false);
		return;
	}
	m_isUpdating = true;
	QNetworkReply* reply = m_netman->get(QNetworkRequest(url));
	chaseRedirects(reply, &AbstractUpdater::updateDownloaded);
}

void AbstractUpdater::chaseRedirects(QNetworkReply* reply, void (AbstractUpdater::*cb)(QNetworkReply*)) {
	connect(reply, &QNetworkReply::finished, this, [this, reply, cb]() {
		// TODO: check domains, etc
		if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() / 100 == 3) {
			QNetworkReply* newReply = m_netman->get(QNetworkRequest(reply->header(QNetworkRequest::LocationHeader).toString()));
			chaseRedirects(newReply, cb);
		} else {
			(this->*cb)(reply);
		}
	});
}

void AbstractUpdater::manifestDownloaded(QNetworkReply* reply) {
	m_manifest = reply->readAll();
	QUrl url = parseManifest(m_manifest);
	if (m_isUpdating) {
		if (!url.isValid()) {
			emit updateDone(false);
		} else {
			QNetworkReply* reply = m_netman->get(QNetworkRequest(url));
			chaseRedirects(reply, &AbstractUpdater::updateDownloaded);
		}
	} else {
		emit updateAvailable(url.isValid());
	}
}

void AbstractUpdater::updateDownloaded(QNetworkReply* reply) {
	m_isUpdating = false;
	if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() / 100 != 2) {
		emit updateDone(false);
		return;
	}
	QFile f(destination());
	if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
		emit updateDone(false);
		return;
	}
	while (true) {
		QByteArray bytes = reply->read(4096);
		if (!bytes.size()) {
			break;
		}
		f.write(bytes);
	}
	f.flush();
	f.close();
	emit updateDone(true);
}
