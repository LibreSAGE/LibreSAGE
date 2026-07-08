#pragma once

#include <QObject>
#include <qqmlintegration.h>

// Launches games. For ones this workspace actually builds (Generals, Zero
// Hour), it runs the in-tree "RTS" binaries -- bin/Generals/ and
// bin/GeneralsMD/, siblings of the launcher's own binary under bin/Common/
// -- rather than any Win32 .exe an Installation happens to have been
// detected from, since this is a dev/test tool for driving in-tree builds.
// For games this workspace has no source for (e.g. the Battle for
// Middle-earth titles), there is no such binary to run, so it falls back to
// launching the originally-discovered executable directly, with no extra
// arguments injected -- we have no way to know what that binary supports.
class GameLauncher : public QObject
{
	Q_OBJECT
	QML_ELEMENT

public:
	explicit GameLauncher(QObject *parent = nullptr);

	// Starts gameId's workspace binary if one exists (see class comment),
	// passing "-dir installPath" so it loads assets from that install's
	// Data/ tree regardless of where the binary itself lives, "-win" if
	// windowed (so the game doesn't grab the display fullscreen), and
	// "-bigdir baseGameInstallPath" if that's non-empty (a game that needs
	// another one's assets preloaded, e.g. Zero Hour needs Generals).
	// Otherwise falls back to originalExecutablePath launched plain.
	Q_INVOKABLE bool launch(const QString &gameId,
			const QString &installPath,
			const QString &originalExecutablePath,
			bool windowed,
			const QString &baseGameInstallPath);

signals:
	void launchFailed(const QString &executablePath, const QString &reason);

private:
	static QString workspaceBinaryPath(const QString &gameId);
};
