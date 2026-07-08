#include "InstallationFinder.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QSettings>
#include <QStandardPaths>
#include <QTextStream>

#ifdef _WIN32
#include <QSettings>
#endif

InstallationFinder::InstallationFinder(QObject *parent)
	: QObject(parent), m_model(new InstallationModel(this))
{
}

void InstallationFinder::setScanning(bool scanning)
{
	if (m_scanning == scanning)
		return;
	m_scanning = scanning;
	emit scanningChanged();
}

void InstallationFinder::refresh()
{
	setScanning(true);

	QList<Installation *> results;
	scanRegistry(results);
	scanConfiguredDirs(results);
	scanSteamLibraries(results);

	// Drop anything the user has explicitly reset via LauncherConfig, so a
	// "forgotten" install doesn't just reappear on the next scan.
	QList<Installation *> kept;
	kept.reserve(results.size());
	for (Installation *installation : std::as_const(results))
	{
		if (m_config.isInstallIgnored(installation->gameId(), installation->installPath()))
			delete installation;
		else
			kept.append(installation);
	}

	m_model->setInstallations(kept);

	setScanning(false);
	emit refreshed();
}

QString InstallationFinder::findExecutable(const GameDefinition &gameDef, const QString &dir)
{
	if (dir.isEmpty())
		return QString();

	const QDir installDir(dir);
	if (!installDir.exists())
		return QString();

	// Case-insensitive: real-world installs are commonly "Generals.exe", but
	// Windows' case-insensitive filesystem lets that pass unnoticed there.
	// On Linux (and this is a Linux port) an exact-case QFileInfo::exists()
	// check silently never matches, so nothing is ever found as installed.
	const QStringList entries = installDir.entryList(QDir::Files);
	for (const QString &candidate : gameDef.executableCandidates)
	{
		for (const QString &entry : entries)
		{
			if (entry.compare(candidate, Qt::CaseInsensitive) == 0)
				return QDir::toNativeSeparators(installDir.filePath(entry));
		}
	}
	return QString();
}

bool InstallationFinder::alreadyFound(const QList<Installation *> &results, const QString &installPath)
{
	for (const Installation *installation : results)
	{
		if (installation->installPath().compare(installPath, Qt::CaseInsensitive) == 0)
			return true;
	}
	return false;
}

void InstallationFinder::tryAddRegistryInstall(const GameDefinition &gameDef, const QString &installPath, QList<Installation *> &results) const
{
	if (installPath.isEmpty())
		return;

	const QString executable = findExecutable(gameDef, installPath);
	if (executable.isEmpty())
		return;

	if (alreadyFound(results, installPath))
		return;

	results.append(new Installation(gameDef.id,
			gameDef.displayName,
			QDir::toNativeSeparators(installPath),
			executable,
			gameDef.coverArt,
			Installation::Source::Registry));
}

void InstallationFinder::scanRegistry(QList<Installation *> &results) const
{
#ifdef _WIN32
	for (const GameDefinition &gameDef : gameCatalog())
	{
		for (const QString &registryPath : gameDef.registryPaths)
		{
			QSettings key(registryPath, QSettings::NativeFormat);
			tryAddRegistryInstall(gameDef, key.value(gameDef.registryValueName).toString(), results);
		}

		// Some installers (e.g. EA's older Battle for Middle-earth titles)
		// key each install under a per-version subkey (like "1.06.0000")
		// rather than a single fixed path, so the parent key itself has no
		// value -- its child group names have to be enumerated instead.
		for (const QString &parentPath : gameDef.registryVersionedParentPaths)
		{
			QSettings parentKey(parentPath, QSettings::NativeFormat);
			const QStringList versions = parentKey.childGroups();
			for (const QString &version : versions)
			{
				QSettings versionKey(parentPath + QStringLiteral("\\") + version, QSettings::NativeFormat);
				tryAddRegistryInstall(gameDef, versionKey.value(gameDef.registryValueName).toString(), results);
			}
		}
	}
#else
	Q_UNUSED(results);
	// Only the Windows registry carries EA/Westwood install records; other
	// platforms rely on manually-added folders and Steam library scanning.
#endif
}

void InstallationFinder::scanConfiguredDirs(QList<Installation *> &results) const
{
	const QDir steamCmdRoot(m_config.steamInstallDir());

	for (const GameDefinition &gameDef : gameCatalog())
	{
		// Each candidate dir here is already scoped to this specific game --
		// either browsed-to explicitly by the user or SteamCMD's own
		// per-game subfolder -- unlike Steam-library scanning, which has to
		// disambiguate games sharing an executable name via the app id.
		QStringList candidateDirs = m_config.manualInstallDirs(gameDef.id);
		candidateDirs.append(steamCmdRoot.filePath(gameDef.id));

		for (const QString &dir : std::as_const(candidateDirs))
		{
			const QString executable = findExecutable(gameDef, dir);
			if (executable.isEmpty())
				continue;
			if (alreadyFound(results, dir))
				continue;

			results.append(new Installation(gameDef.id,
					gameDef.displayName,
					QDir::toNativeSeparators(dir),
					executable,
					gameDef.coverArt,
					Installation::Source::Manual));
		}
	}
}

void InstallationFinder::scanSteamLibraries(QList<Installation *> &results) const
{
	QStringList steamRoots;
#ifdef _WIN32
	QSettings steamKey(QStringLiteral("HKEY_CURRENT_USER\\Software\\Valve\\Steam"), QSettings::NativeFormat);
	const QString steamPath = steamKey.value(QStringLiteral("SteamPath")).toString();
	if (!steamPath.isEmpty())
		steamRoots.append(steamPath);
#else
	const QString home = QDir::homePath();
	steamRoots.append(home + QStringLiteral("/.local/share/Steam"));
	steamRoots.append(home + QStringLiteral("/.steam/steam"));
	steamRoots.append(home + QStringLiteral("/Library/Application Support/Steam"));
#endif

	for (const QString &steamRoot : std::as_const(steamRoots))
	{
		const QString libraryFolders = QDir(steamRoot).filePath(QStringLiteral("steamapps/libraryfolders.vdf"));
		QStringList libraries = {steamRoot};

		QFile vdf(libraryFolders);
		if (vdf.open(QIODevice::ReadOnly | QIODevice::Text))
		{
			QTextStream stream(&vdf);
			static const QRegularExpression pathLine(QStringLiteral("\"path\"\\s*\"([^\"]+)\""));
			while (!stream.atEnd())
			{
				const QString line = stream.readLine();
				const QRegularExpressionMatch match = pathLine.match(line);
				if (match.hasMatch())
					libraries.append(match.captured(1));
			}
		}

		for (const QString &library : std::as_const(libraries))
		{
			const QString commonDir = QDir(library).filePath(QStringLiteral("steamapps/common"));
			for (const GameDefinition &gameDef : gameCatalog())
			{
				if (!gameDef.hasSteamRelease)
					continue;

				const QString appId = m_config.steamAppId(gameDef.id);
				if (appId.isEmpty())
					continue;

				const QString manifest = QDir(library).filePath(
						QStringLiteral("steamapps/appmanifest_%1.acf").arg(appId));
				QFile manifestFile(manifest);
				if (!manifestFile.open(QIODevice::ReadOnly | QIODevice::Text))
					continue;

				QTextStream stream(&manifestFile);
				static const QRegularExpression installDirLine(QStringLiteral("\"installdir\"\\s*\"([^\"]+)\""));
				QString installDirName;
				while (!stream.atEnd())
				{
					const QRegularExpressionMatch match = installDirLine.match(stream.readLine());
					if (match.hasMatch())
					{
						installDirName = match.captured(1);
						break;
					}
				}
				if (installDirName.isEmpty())
					continue;

				const QString installPath = QDir(commonDir).filePath(installDirName);
				const QString executable = findExecutable(gameDef, installPath);
				if (executable.isEmpty())
					continue;
				if (alreadyFound(results, installPath))
					continue;

				results.append(new Installation(gameDef.id,
						gameDef.displayName,
						QDir::toNativeSeparators(installPath),
						executable,
						gameDef.coverArt,
						Installation::Source::Steam));
			}
		}
	}
}
