#include "GameLauncher.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QProcess>

GameLauncher::GameLauncher(QObject *parent) : QObject(parent)
{
}

QString GameLauncher::workspaceBinaryPath(const QString &gameId)
{
	// The launcher, Generals, and GeneralsMD are all built into the same
	// preset's bin/ tree (bin/Common/Launcher, bin/Generals/RTS,
	// bin/GeneralsMD/RTS), so walking up from our own binary finds the
	// build that actually matches this launcher, regardless of which
	// preset/config was used.
	QDir binDir(QCoreApplication::applicationDirPath());
	binDir.cdUp();

	if (gameId == QStringLiteral("generals"))
		return binDir.filePath(QStringLiteral("Generals/RTS"));
	if (gameId == QStringLiteral("zerohour"))
		return binDir.filePath(QStringLiteral("GeneralsMD/RTS"));
	return QString();
}

bool GameLauncher::launch(const QString &gameId,
		const QString &installPath,
		const QString &originalExecutablePath,
		bool windowed,
		const QString &baseGameInstallPath)
{
	const QString workspaceExe = workspaceBinaryPath(gameId);
	const bool useWorkspaceBinary = !workspaceExe.isEmpty() && QFileInfo::exists(workspaceExe);

	const QString exePath = useWorkspaceBinary ? workspaceExe : originalExecutablePath;
	const QFileInfo info(exePath);
	if (exePath.isEmpty() || !info.exists() || !info.isExecutable())
	{
		emit launchFailed(exePath,
				!workspaceExe.isEmpty()
						? tr("Workspace build for this game was not found (expected at \"%1\"). Build it first.").arg(workspaceExe)
						: tr("Could not find an executable to launch at \"%1\".").arg(exePath));
		return false;
	}

	QStringList args;
	if (useWorkspaceBinary)
	{
		// We control this binary's command line (it's built from this same
		// source tree), so it's safe to drive it explicitly.
		args << QStringLiteral("-dir") << installPath;
		if (windowed)
			args << QStringLiteral("-win");
		if (!baseGameInstallPath.isEmpty())
			args << QStringLiteral("-bigdir") << baseGameInstallPath;
	}
	// Otherwise this is some other game's original executable, of unknown
	// provenance -- launch it plain rather than guessing at flags it may
	// not support.

	const bool started = QProcess::startDetached(info.absoluteFilePath(), args, info.absolutePath());
	if (!started)
		emit launchFailed(exePath, tr("The operating system refused to start the process."));
	return started;
}
