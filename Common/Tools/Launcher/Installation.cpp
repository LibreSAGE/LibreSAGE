#include "Installation.h"

Installation::Installation(QString gameId,
		QString displayName,
		QString installPath,
		QString executablePath,
		QString coverArt,
		Source source,
		QObject *parent)
	: QObject(parent),
	  m_gameId(std::move(gameId)),
	  m_displayName(std::move(displayName)),
	  m_installPath(std::move(installPath)),
	  m_executablePath(std::move(executablePath)),
	  m_coverArt(std::move(coverArt)),
	  m_source(source)
{
}

QString Installation::source() const
{
	switch (m_source)
	{
		case Source::Registry:
			return QStringLiteral("Registry");
		case Source::Steam:
			return QStringLiteral("Steam");
		case Source::Manual:
		default:
			return QStringLiteral("Manual");
	}
}
