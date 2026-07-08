#pragma once

#include <QList>
#include <QObject>
#include <qqmlintegration.h>

#include "GameCatalog.h"
#include "InstallationModel.h"
#include "LauncherConfig.h"

class InstallationFinder : public QObject
{
	Q_OBJECT
	QML_ELEMENT

	Q_PROPERTY(InstallationModel *model READ model CONSTANT)
	Q_PROPERTY(bool scanning READ scanning NOTIFY scanningChanged)

public:
	explicit InstallationFinder(QObject *parent = nullptr);

	InstallationModel *model() const { return m_model; }
	bool scanning() const { return m_scanning; }

	// Rescans the Windows registry, any manually-added folders and the
	// SteamCMD download directory for known games, replacing the model
	// contents. Safe to call repeatedly (e.g. after a SteamCMD install
	// completes).
	Q_INVOKABLE void refresh();

signals:
	void scanningChanged();
	void refreshed();

private:
	void scanRegistry(QList<Installation *> &results) const;
	void scanConfiguredDirs(QList<Installation *> &results) const;
	void scanSteamLibraries(QList<Installation *> &results) const;
	void setScanning(bool scanning);

	// Shared by both the exact-path and versioned-subkey registry lookups:
	// resolves installPath to an executable and, if found and not already
	// present, appends it to results.
	void tryAddRegistryInstall(const GameDefinition &gameDef, const QString &installPath, QList<Installation *> &results) const;

	static QString findExecutable(const GameDefinition &gameDef, const QString &dir);
	static bool alreadyFound(const QList<Installation *> &results, const QString &installPath);

	InstallationModel *m_model;
	// A private instance, distinct from the QML-exposed LauncherConfig
	// singleton -- this one is only ever used internally for plain settings
	// reads, so it goes through the normal constructor directly rather than
	// the singleton's create(QQmlEngine*, QJSEngine*) factory.
	LauncherConfig m_config{nullptr};
	bool m_scanning = false;
};
