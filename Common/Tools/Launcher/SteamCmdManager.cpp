#include "SteamCmdManager.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QStandardPaths>

namespace
{
// Valve's official, stable SteamCMD distribution endpoints, documented at
// https://developer.valvesoftware.com/wiki/SteamCMD -- kept as a single
// constant so a future URL change only needs updating here.
constexpr const char *kSteamCmdBaseUrl = "https://steamcdn-a.akamaihd.net/client/installer/";

QString platformArchiveName()
{
#if defined(_WIN32)
	return QStringLiteral("steamcmd.zip");
#elif defined(Q_OS_MACOS)
	return QStringLiteral("steamcmd_osx.tar.gz");
#else
	return QStringLiteral("steamcmd_linux.tar.gz");
#endif
}

// Bounds how much raw SteamCMD output the "SteamCMD output" panel keeps
// around, regardless of how long a download runs for.
constexpr qsizetype kMaxConsoleOutputChars = 200'000;
} // namespace

SteamCmdManager::SteamCmdManager(QObject *parent)
	: QObject(parent), m_network(new QNetworkAccessManager(this))
{
	m_consoleFlushTimer.setSingleShot(true);
	m_consoleFlushTimer.setInterval(150);
	connect(&m_consoleFlushTimer, &QTimer::timeout, this, [this]() {
		if (m_consoleOutputDirty)
		{
			m_consoleOutputDirty = false;
			emit consoleOutputChanged();
		}
	});
}

SteamCmdManager::~SteamCmdManager()
{
	if (m_process && m_process->state() != QProcess::NotRunning)
	{
		m_process->kill();
		m_process->waitForFinished(2000);
	}
}

QString SteamCmdManager::steamCmdRootDir() const
{
	return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/SAGELauncher/steamcmd");
}

QString SteamCmdManager::steamCmdExecutablePath() const
{
#if defined(_WIN32)
	return steamCmdRootDir() + QStringLiteral("/steamcmd.exe");
#else
	return steamCmdRootDir() + QStringLiteral("/steamcmd.sh");
#endif
}

QString SteamCmdManager::bootstrapDownloadUrl() const
{
	return QString::fromLatin1(kSteamCmdBaseUrl) + platformArchiveName();
}

bool SteamCmdManager::bootstrapped() const
{
	return QFileInfo::exists(steamCmdExecutablePath());
}

void SteamCmdManager::setBusy(bool busy)
{
	if (m_busy == busy)
		return;
	m_busy = busy;
	emit busyChanged();
}

void SteamCmdManager::setStatusText(const QString &text)
{
	if (m_statusText == text)
		return;
	m_statusText = text;
	emit statusTextChanged();
}

void SteamCmdManager::setProgress(int progress)
{
	if (m_progress == progress)
		return;
	m_progress = progress;
	emit progressChanged();
}

void SteamCmdManager::setSteamGuardRequired(bool required)
{
	if (m_steamGuardRequired == required)
		return;
	m_steamGuardRequired = required;
	emit steamGuardRequiredChanged();
}

void SteamCmdManager::bootstrap()
{
	if (bootstrapped())
	{
		emit bootstrapFinished(true);
		return;
	}
	if (m_busy)
		return;

	QDir().mkpath(steamCmdRootDir());

	setBusy(true);
	setStatusText(tr("Downloading SteamCMD..."));
	setProgress(0);

	QNetworkRequest request{QUrl(bootstrapDownloadUrl())};
	m_bootstrapReply = m_network->get(request);

	connect(m_bootstrapReply, &QNetworkReply::downloadProgress, this, [this](qint64 received, qint64 total) {
		if (total > 0)
			setProgress(static_cast<int>((received * 100) / total));
	});

	connect(m_bootstrapReply, &QNetworkReply::finished, this, [this]() {
		QNetworkReply *reply = m_bootstrapReply;
		m_bootstrapReply = nullptr;

		if (reply->error() != QNetworkReply::NoError)
		{
			setStatusText(tr("Failed to download SteamCMD: %1").arg(reply->errorString()));
			setBusy(false);
			reply->deleteLater();
			emit bootstrapFinished(false);
			return;
		}

		const QString archivePath = steamCmdRootDir() + QStringLiteral("/") + platformArchiveName();
		QFile archive(archivePath);
		if (archive.open(QIODevice::WriteOnly))
		{
			archive.write(reply->readAll());
			archive.close();
		}
		reply->deleteLater();

		extractBootstrapArchive(archivePath);
	});
}

void SteamCmdManager::extractBootstrapArchive(const QString &archivePath)
{
	setStatusText(tr("Extracting SteamCMD..."));

	auto *extractProcess = new QProcess(this);
	extractProcess->setWorkingDirectory(steamCmdRootDir());

#if defined(_WIN32)
	// bsdtar (bundled as "tar" since Windows 10) understands zip archives,
	// which avoids pulling in a separate zip-extraction dependency.
	extractProcess->setProgram(QStringLiteral("tar"));
	extractProcess->setArguments({QStringLiteral("-xf"), archivePath});
#else
	extractProcess->setProgram(QStringLiteral("tar"));
	extractProcess->setArguments({QStringLiteral("-xzf"), archivePath});
#endif

	connect(extractProcess,
			QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
			this,
			[this, extractProcess](int exitCode, QProcess::ExitStatus) {
				extractProcess->deleteLater();
				QFile::remove(steamCmdRootDir() + QStringLiteral("/") + platformArchiveName());

#if !defined(_WIN32)
				QFile::setPermissions(steamCmdExecutablePath(),
						QFile::permissions(steamCmdExecutablePath()) | QFileDevice::ExeOwner | QFileDevice::ExeGroup
								| QFileDevice::ExeOther);
#endif

				const bool success = exitCode == 0 && bootstrapped();
				setStatusText(success ? tr("SteamCMD ready.") : tr("Failed to extract SteamCMD."));
				setBusy(false);
				emit bootstrappedChanged();
				emit bootstrapFinished(success);
			});

	extractProcess->start();
}

void SteamCmdManager::installOrUpdate(const QString &appId, const QString &installDir, const QString &username, const QString &password)
{
	startAttempt(appId, installDir, username, password, QString());
}

void SteamCmdManager::submitSteamGuardCode(const QString &code)
{
	startAttempt(m_pendingAppId, m_pendingInstallDir, m_pendingUsername, m_pendingPassword, code);
}

void SteamCmdManager::startAttempt(const QString &appId,
		const QString &installDir,
		const QString &username,
		const QString &password,
		const QString &guardCode)
{
	if (m_busy)
		return;
	if (appId.isEmpty())
	{
		setStatusText(tr("No Steam App ID configured for this game."));
		emit installFinished(false, installDir);
		return;
	}
	if (username.isEmpty() || password.isEmpty())
	{
		setStatusText(tr("A Steam username and password are required."));
		emit installFinished(false, installDir);
		return;
	}
	if (!bootstrapped())
	{
		setStatusText(tr("SteamCMD is not installed yet."));
		emit installFinished(false, installDir);
		return;
	}

	QDir().mkpath(installDir);

	setBusy(true);
	setSteamGuardRequired(false);
	// Stays -1 until SteamCMD's own "progress: NN" lines during app_update
	// push it to 0+, i.e. only once login (and any Guard code) succeeded.
	setProgress(-1);
	setStatusText(guardCode.isEmpty() ? tr("Logging in...") : tr("Verifying Steam Guard code..."));
	m_pendingInstallDir = installDir;
	m_pendingAppId = appId;
	m_pendingUsername = username;
	m_pendingPassword = password;
	m_guardNeededThisAttempt = false;
	m_successThisAttempt = false;
	m_failureReason.clear();
	m_consoleOutput.clear();
	m_pendingLine.clear();
	m_consoleFlushTimer.stop();
	m_consoleOutputDirty = false;
	emit consoleOutputChanged();

	// The whole attempt -- including the Guard code, as documented, as a
	// third argument to `login` -- is scripted via arguments so SteamCMD
	// runs fully non-interactively; see the class comment for why.
	m_process = new QProcess(this);
	m_process->setProgram(steamCmdExecutablePath());
	m_process->setProcessChannelMode(QProcess::MergedChannels);

	QStringList args;
	args << QStringLiteral("+force_install_dir") << installDir;
	args << QStringLiteral("+login") << username << password;
	if (!guardCode.isEmpty())
		args << guardCode;
	args << QStringLiteral("+app_update") << appId << QStringLiteral("validate");
	args << QStringLiteral("+quit");
	m_process->setArguments(args);

	connect(m_process, &QProcess::readyReadStandardOutput, this, [this]() {
		handleProcessOutput(QString::fromLocal8Bit(m_process->readAllStandardOutput()));
	});

	connect(m_process,
			QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
			this,
			[this](int exitCode, QProcess::ExitStatus exitStatus) {
				m_consoleFlushTimer.stop();
				if (m_consoleOutputDirty)
				{
					m_consoleOutputDirty = false;
					emit consoleOutputChanged();
				}

				setBusy(false);
				m_process->deleteLater();
				m_process = nullptr;

				if (m_successThisAttempt)
				{
					setSteamGuardRequired(false);
					setStatusText(tr("Install complete."));
					emit installFinished(true, m_pendingInstallDir);
				}
				else if (m_guardNeededThisAttempt)
				{
					// Not a final failure -- pause here and let the UI collect
					// a code, then start a fresh attempt with it.
					setStatusText(m_failureReason.isEmpty() ? tr("Steam Guard code required.") : m_failureReason);
					setSteamGuardRequired(true);
				}
				else
				{
					setSteamGuardRequired(false);
					if (!m_failureReason.isEmpty())
					{
						setStatusText(m_failureReason);
					}
					else if (exitStatus == QProcess::CrashExit)
					{
						// SteamCMD's own binary segfaulting partway through login
						// (commonly around "Waiting for client config...") is a
						// long-standing, widely-reported SteamCMD-on-Linux issue,
						// not something this launcher's command construction
						// controls. Expand "SteamCMD output" below for the exact
						// point it reached; retrying, or clearing ~/Steam and
						// ~/.steam, are the commonly reported workarounds.
						setStatusText(tr("SteamCMD crashed (a known SteamCMD/Linux issue, not specific to this launcher). "
										  "Try again, or see \"SteamCMD output\" below for where it stopped."));
					}
					else
					{
						setStatusText(exitCode == 0 ? tr("SteamCMD exited unexpectedly.") : tr("SteamCMD exited with an error."));
					}
					emit installFinished(false, m_pendingInstallDir);
				}
			});

	m_process->start();
}

void SteamCmdManager::handleProcessOutput(const QString &chunk)
{
	m_consoleOutput += chunk;
	if (m_consoleOutput.size() > kMaxConsoleOutputChars)
		m_consoleOutput = m_consoleOutput.right(kMaxConsoleOutputChars);
	m_consoleOutputDirty = true;
	if (!m_consoleFlushTimer.isActive())
		m_consoleFlushTimer.start();

	// SteamCMD isn't guaranteed to flush whole lines in one read. Buffer
	// into lines (also re-scanning the still-open tail) so a phrase split
	// across two reads can't be half-matched.
	m_pendingLine += chunk;
	const QStringList lines = m_pendingLine.split(QRegularExpression(QStringLiteral("[\\r\\n]")));
	for (qsizetype i = 0; i < lines.size() - 1; ++i)
		processLine(lines.at(i));
	m_pendingLine = lines.last();
	processLine(m_pendingLine);
}

void SteamCmdManager::processLine(const QString &line)
{
	static const QRegularExpression progressPattern(QStringLiteral("progress:\\s*([0-9]+(?:\\.[0-9]+)?)"));
	const QRegularExpressionMatch match = progressPattern.match(line);
	if (match.hasMatch())
		setProgress(static_cast<int>(match.captured(1).toDouble()));

	if (line.contains(QStringLiteral("Invalid Login Auth Code"), Qt::CaseInsensitive)
			|| line.contains(QStringLiteral("Two-factor code mismatch"), Qt::CaseInsensitive))
	{
		m_failureReason = tr("Wrong Steam Guard code -- try again.");
		m_guardNeededThisAttempt = true;
		return;
	}

	if (line.contains(QStringLiteral("Invalid Password"), Qt::CaseInsensitive))
	{
		m_failureReason = tr("Login failed: invalid password.");
		return;
	}

	if (line.contains(QStringLiteral("Steam Guard"), Qt::CaseInsensitive)
			|| line.contains(QStringLiteral("Enter the current code"), Qt::CaseInsensitive)
			|| line.contains(QStringLiteral("Two-factor code"), Qt::CaseInsensitive))
	{
		m_guardNeededThisAttempt = true;
		return;
	}

	if (line.contains(QStringLiteral("Update state"), Qt::CaseInsensitive))
		setStatusText(tr("Downloading..."));

	if (line.contains(QStringLiteral("Success! App"), Qt::CaseInsensitive))
	{
		m_successThisAttempt = true;
		setProgress(100);
	}
}

void SteamCmdManager::cancel()
{
	if (m_bootstrapReply)
		m_bootstrapReply->abort();
	if (m_process && m_process->state() != QProcess::NotRunning)
		m_process->kill();
	setBusy(false);
	setStatusText(tr("Cancelled."));
}
