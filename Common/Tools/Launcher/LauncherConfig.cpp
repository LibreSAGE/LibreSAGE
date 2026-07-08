#include "LauncherConfig.h"

#include <QCoreApplication>
#include <QDir>
#include <QLocale>
#include <QQmlEngine>
#include <QSettings>
#include <QStandardPaths>
#include <QTranslator>

#include "GameCatalog.h"

namespace
{
QSettings makeSettings()
{
	return QSettings(QSettings::IniFormat,
			QSettings::UserScope,
			QStringLiteral("SAGELauncher"),
			QStringLiteral("Launcher"));
}

QString defaultSteamAppId(const QString &gameId)
{
	for (const GameDefinition &gameDef : gameCatalog())
	{
		if (gameDef.id == gameId)
			return gameDef.steamAppId;
	}
	return QString();
}
} // namespace

LauncherConfig::LauncherConfig(QObject *parent) : QObject(parent)
{
}

LauncherConfig::~LauncherConfig()
{
	QCoreApplication::removeTranslator(&m_translator);
}

LauncherConfig *LauncherConfig::create(QQmlEngine *qmlEngine, QJSEngine *)
{
	auto *config = new LauncherConfig(nullptr);
	config->m_qmlEngine = qmlEngine;
	config->applyTranslator();
	connect(config, &LauncherConfig::effectiveLanguageChanged, config, &LauncherConfig::applyTranslator);
	return config;
}

void LauncherConfig::applyTranslator()
{
	// Same pattern as Qt's "Localized Clock" example: drop whatever was
	// loaded, then reload the same QTranslator instance for the new
	// language and reinstall it.
	QCoreApplication::removeTranslator(&m_translator);

	// English is the .ts source language, so it needs no translator at all
	// -- qsTr()/tr() already return the English source text once none is
	// installed.
	if (effectiveLanguage() == QStringLiteral("de") && m_translator.load(QStringLiteral(":/i18n/launcher_de.qm")))
		QCoreApplication::installTranslator(&m_translator);

	if (m_qmlEngine)
		m_qmlEngine->retranslate();
}

QString LauncherConfig::generalsSteamAppId() const
{
	const QString stored = makeSettings().value(QStringLiteral("SteamAppIds/generals")).toString();
	return stored.isEmpty() ? defaultSteamAppId(QStringLiteral("generals")) : stored;
}

void LauncherConfig::setGeneralsSteamAppId(const QString &appId)
{
	if (appId == generalsSteamAppId())
		return;
	QSettings settings = makeSettings();
	settings.setValue(QStringLiteral("SteamAppIds/generals"), appId);
	emit generalsSteamAppIdChanged();
}

QString LauncherConfig::zeroHourSteamAppId() const
{
	const QString stored = makeSettings().value(QStringLiteral("SteamAppIds/zerohour")).toString();
	return stored.isEmpty() ? defaultSteamAppId(QStringLiteral("zerohour")) : stored;
}

void LauncherConfig::setZeroHourSteamAppId(const QString &appId)
{
	if (appId == zeroHourSteamAppId())
		return;
	QSettings settings = makeSettings();
	settings.setValue(QStringLiteral("SteamAppIds/zerohour"), appId);
	emit zeroHourSteamAppIdChanged();
}

QString LauncherConfig::steamInstallDir() const
{
	QString dir = makeSettings().value(QStringLiteral("SteamCmd/InstallDir")).toString();
	if (dir.isEmpty())
	{
		dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
				+ QStringLiteral("/SAGELauncher/Games");
	}
	return dir;
}

void LauncherConfig::setSteamInstallDir(const QString &dir)
{
	if (dir == steamInstallDir())
		return;
	QSettings settings = makeSettings();
	settings.setValue(QStringLiteral("SteamCmd/InstallDir"), dir);
	emit steamInstallDirChanged();
}

bool LauncherConfig::windowedMode() const
{
	return makeSettings().value(QStringLiteral("Launch/Windowed"), true).toBool();
}

void LauncherConfig::setWindowedMode(bool windowed)
{
	if (windowed == windowedMode())
		return;
	QSettings settings = makeSettings();
	settings.setValue(QStringLiteral("Launch/Windowed"), windowed);
	emit windowedModeChanged();
}

bool LauncherConfig::showInstallPaths() const
{
	return makeSettings().value(QStringLiteral("UI/ShowInstallPaths"), true).toBool();
}

void LauncherConfig::setShowInstallPaths(bool show)
{
	if (show == showInstallPaths())
		return;
	QSettings settings = makeSettings();
	settings.setValue(QStringLiteral("UI/ShowInstallPaths"), show);
	emit showInstallPathsChanged();
}

QString LauncherConfig::language() const
{
	return makeSettings().value(QStringLiteral("UI/Language"), QStringLiteral("system")).toString();
}

void LauncherConfig::setLanguage(const QString &lang)
{
	if (lang == language())
		return;
	const QString oldEffective = effectiveLanguage();
	QSettings settings = makeSettings();
	settings.setValue(QStringLiteral("UI/Language"), lang);
	emit languageChanged();
	if (effectiveLanguage() != oldEffective)
		emit effectiveLanguageChanged();
}

QString LauncherConfig::effectiveLanguage() const
{
	const QString lang = language();
	if (lang == QStringLiteral("en") || lang == QStringLiteral("de"))
		return lang;
	// "system" (or anything unrecognized): derive from the OS locale, since
	// that's what most users expect a freshly-installed app to start in.
	return QLocale::system().name().startsWith(QStringLiteral("de")) ? QStringLiteral("de") : QStringLiteral("en");
}

QString LauncherConfig::steamAppId(const QString &gameId) const
{
	if (gameId == QStringLiteral("generals"))
		return generalsSteamAppId();
	if (gameId == QStringLiteral("zerohour"))
		return zeroHourSteamAppId();
	return QString();
}

QString LauncherConfig::installDirFor(const QString &gameId) const
{
	return QDir(steamInstallDir()).filePath(gameId);
}

QStringList LauncherConfig::manualInstallDirs(const QString &gameId) const
{
	return makeSettings().value(QStringLiteral("ManualInstallDirs/%1").arg(gameId)).toStringList();
}

void LauncherConfig::addManualInstallDir(const QString &gameId, const QUrl &dirUrl)
{
	const QString dir = dirUrl.isLocalFile() ? dirUrl.toLocalFile() : dirUrl.toString();
	if (dir.isEmpty())
		return;

	QStringList dirs = manualInstallDirs(gameId);
	if (dirs.contains(dir))
		return;
	dirs.append(dir);
	QSettings settings = makeSettings();
	settings.setValue(QStringLiteral("ManualInstallDirs/%1").arg(gameId), dirs);
}

bool LauncherConfig::isInstallIgnored(const QString &gameId, const QString &installPath) const
{
	const QStringList ignored = makeSettings().value(QStringLiteral("IgnoredInstalls/%1").arg(gameId)).toStringList();
	for (const QString &path : ignored)
	{
		if (path.compare(installPath, Qt::CaseInsensitive) == 0)
			return true;
	}
	return false;
}

void LauncherConfig::resetInstallation(const QString &gameId, const QString &installPath)
{
	QSettings settings = makeSettings();

	QStringList manualDirs = manualInstallDirs(gameId);
	const qsizetype removed = manualDirs.removeIf(
			[&installPath](const QString &dir) { return dir.compare(installPath, Qt::CaseInsensitive) == 0; });
	if (removed > 0)
		settings.setValue(QStringLiteral("ManualInstallDirs/%1").arg(gameId), manualDirs);

	if (!isInstallIgnored(gameId, installPath))
	{
		QStringList ignored = settings.value(QStringLiteral("IgnoredInstalls/%1").arg(gameId)).toStringList();
		ignored.append(installPath);
		settings.setValue(QStringLiteral("IgnoredInstalls/%1").arg(gameId), ignored);
	}
}
