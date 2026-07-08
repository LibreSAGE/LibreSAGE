#pragma once

#include <QList>
#include <QString>
#include <QStringList>

// Static description of a supported game: where to look for an existing
// install, what the executable is called and which SteamCMD app id can be
// used to download it. Steam app ids are left blank by default since the
// exact ids depend on which storefront release is being targeted; they can
// be filled in via LauncherConfig (see LauncherConfig.h) without touching
// this catalog.
struct GameDefinition
{
	QString id;
	QString displayName;
	QString coverArt;
	QStringList executableCandidates;
	// Each entry is an exact registry path under HKEY_LOCAL_MACHINE, checked
	// directly for registryValueName. Only ever consulted on Windows (see
	// InstallationFinder).
	QStringList registryPaths;
	// Parent keys whose *subkeys* (typically per-version, e.g. "1.06.0000")
	// should each be checked for registryValueName, for installers that
	// don't write to a fixed, version-independent path.
	QStringList registryVersionedParentPaths;
	QString registryValueName;
	QString steamAppId;
	// gameId of another catalog entry whose assets this game's engine needs
	// preloaded via "-bigdir" to run at all (e.g. Zero Hour needs Generals'
	// .big archives), or that it otherwise cannot run without. Empty if this
	// game is self-contained.
	QString requiresBaseGameId;
	// False for games with no Steam release at all, as opposed to one whose
	// app id just isn't known yet -- hides the SteamCMD install UI entirely
	// rather than offering a control that can never work.
	bool hasSteamRelease = false;
};

inline const QList<GameDefinition> &gameCatalog()
{
	static const QList<GameDefinition> catalog = {
			GameDefinition{
					.id = QStringLiteral("generals"),
					.displayName = QStringLiteral("Command & Conquer: Generals"),
					.coverArt = QStringLiteral("qrc:/qt/qml/SAGELauncher/assets/covers/generals.jpg"),
					.executableCandidates = {QStringLiteral("generals.exe")},
					.registryPaths =
							{
									QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\WOW6432Node\\Electronic Arts\\EA Games\\Command and Conquer Generals"),
									QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Electronic Arts\\EA Games\\Command and Conquer Generals"),
									QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\WOW6432Node\\Westwood\\Generals"),
									QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Westwood\\Generals"),
							},
					.registryValueName = QStringLiteral("InstallPath"),
					.steamAppId = QStringLiteral("2229870"),
					.hasSteamRelease = true,
			},
			GameDefinition{
					.id = QStringLiteral("zerohour"),
					.displayName = QStringLiteral("Command & Conquer: Generals - Zero Hour"),
					.coverArt = QStringLiteral("qrc:/qt/qml/SAGELauncher/assets/covers/zerohour.jpg"),
					.executableCandidates = {QStringLiteral("generals.exe")},
					.registryPaths =
							{
									QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\WOW6432Node\\Electronic Arts\\EA Games\\Command and Conquer Generals Zero Hour"),
									QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Electronic Arts\\EA Games\\Command and Conquer Generals Zero Hour"),
									QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\WOW6432Node\\Westwood\\Generals Zero Hour"),
									QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Westwood\\Generals Zero Hour"),
							},
					.registryValueName = QStringLiteral("InstallPath"),
					.steamAppId = QStringLiteral("2732960"),
					.requiresBaseGameId = QStringLiteral("generals"),
					.hasSteamRelease = true,
			},
			GameDefinition{
					.id = QStringLiteral("bfme1"),
					.displayName = QStringLiteral("The Lord of the Rings: The Battle for Middle-earth"),
					.coverArt = QStringLiteral("qrc:/qt/qml/SAGELauncher/assets/covers/bfme1.jpg"),
					.executableCandidates = {QStringLiteral("lotrbfme.exe")},
					// EA's older installers key each version under its own
					// subkey (e.g. "1.03.0000") rather than a fixed path.
					.registryVersionedParentPaths =
							{
									QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\WOW6432Node\\Electronic Arts\\Electronic Arts\\The Lord of the Rings, The Battle for Middle-earth"),
									QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Electronic Arts\\Electronic Arts\\The Lord of the Rings, The Battle for Middle-earth"),
							},
					.registryValueName = QStringLiteral("InstallPath"),
					.hasSteamRelease = false,
			},
			GameDefinition{
					.id = QStringLiteral("bfme2"),
					.displayName = QStringLiteral("The Lord of the Rings: The Battle for Middle-earth II"),
					.coverArt = QStringLiteral("qrc:/qt/qml/SAGELauncher/assets/covers/bfme2.jpg"),
					.executableCandidates = {QStringLiteral("lotrbfme2.exe")},
					.registryVersionedParentPaths =
							{
									QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\WOW6432Node\\Electronic Arts\\Electronic Arts\\The Lord of the Rings, The Battle for Middle-earth II"),
									QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Electronic Arts\\Electronic Arts\\The Lord of the Rings, The Battle for Middle-earth II"),
							},
					.registryValueName = QStringLiteral("InstallPath"),
					.hasSteamRelease = false,
			},
			GameDefinition{
					.id = QStringLiteral("rotwk"),
					.displayName = QStringLiteral("The Lord of the Rings: The Rise of the Witch-king"),
					.coverArt = QStringLiteral("qrc:/qt/qml/SAGELauncher/assets/covers/rotwk.jpg"),
					.executableCandidates = {QStringLiteral("lotrbfme2ep1.exe")},
					.registryVersionedParentPaths =
							{
									QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\WOW6432Node\\Electronic Arts\\Electronic Arts\\The Lord of the Rings, The Rise of the Witch-king"),
									QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Electronic Arts\\Electronic Arts\\The Lord of the Rings, The Rise of the Witch-king"),
							},
					.registryValueName = QStringLiteral("InstallPath"),
					.requiresBaseGameId = QStringLiteral("bfme2"),
					.hasSteamRelease = false,
			},
	};
	return catalog;
}

inline const GameDefinition *findGameDefinition(const QString &gameId)
{
	for (const GameDefinition &gameDef : gameCatalog())
	{
		if (gameDef.id == gameId)
			return &gameDef;
	}
	return nullptr;
}
