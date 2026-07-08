#pragma once

#include <QObject>
#include <QString>
#include <qqmlintegration.h>

class Installation : public QObject
{
	Q_OBJECT
	QML_ELEMENT
	QML_UNCREATABLE("Installation instances are created by InstallationFinder")

	Q_PROPERTY(QString gameId READ gameId CONSTANT)
	Q_PROPERTY(QString displayName READ displayName CONSTANT)
	Q_PROPERTY(QString installPath READ installPath CONSTANT)
	Q_PROPERTY(QString executablePath READ executablePath CONSTANT)
	Q_PROPERTY(QString coverArt READ coverArt CONSTANT)
	Q_PROPERTY(QString source READ source CONSTANT)

public:
	enum class Source
	{
		Registry,
		Steam,
		Manual,
	};

	Installation(QString gameId,
			QString displayName,
			QString installPath,
			QString executablePath,
			QString coverArt,
			Source source,
			QObject *parent = nullptr);

	const QString &gameId() const { return m_gameId; }
	const QString &displayName() const { return m_displayName; }
	const QString &installPath() const { return m_installPath; }
	const QString &executablePath() const { return m_executablePath; }
	const QString &coverArt() const { return m_coverArt; }
	QString source() const;
	Source sourceEnum() const { return m_source; }

private:
	QString m_gameId;
	QString m_displayName;
	QString m_installPath;
	QString m_executablePath;
	QString m_coverArt;
	Source m_source;
};
