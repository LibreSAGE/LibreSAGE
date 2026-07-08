#pragma once

#include <QObject>
#include <QProcess>
#include <QTimer>
#include <qqmlintegration.h>

class QNetworkAccessManager;
class QNetworkReply;

// Wraps SteamCMD: fetches the console tool from Valve on first use, then
// drives it to log in and install/update one of the catalog games.
//
// Each login/install attempt is a single, fully non-interactive SteamCMD
// invocation with everything (force_install_dir, login, app_update, quit)
// passed as command-line arguments -- including the Steam Guard code, as
// the (documented, commonly-used) third argument to `login`, once the user
// has one to give. An earlier version drove SteamCMD interactively over its
// own stdin instead, to avoid putting the password on the command line, but
// that turned out to be unreliable: SteamCMD's console interface is known to
// behave inconsistently when its stdio isn't a real TTY (buffering/prompt
// issues), which showed up as the Guard prompt silently dying mid-session.
// Scripting the whole attempt via argv is what SteamCMD is actually built
// and documented to support, at the cost of the password/code being
// visible in the process's command line for the (brief) life of that
// process -- the same trade-off every steamcmd automation wrapper makes.
class SteamCmdManager : public QObject
{
	Q_OBJECT
	QML_ELEMENT

	Q_PROPERTY(bool bootstrapped READ bootstrapped NOTIFY bootstrappedChanged)
	Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
	Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)
	Q_PROPERTY(int progress READ progress NOTIFY progressChanged)
	Q_PROPERTY(bool steamGuardRequired READ steamGuardRequired NOTIFY steamGuardRequiredChanged)
	Q_PROPERTY(QString consoleOutput READ consoleOutput NOTIFY consoleOutputChanged)

public:
	explicit SteamCmdManager(QObject *parent = nullptr);
	~SteamCmdManager() override;

	bool bootstrapped() const;
	bool busy() const { return m_busy; }
	QString statusText() const { return m_statusText; }
	int progress() const { return m_progress; }
	bool steamGuardRequired() const { return m_steamGuardRequired; }
	// Raw SteamCMD output (stdout+stderr) for the current/last attempt, so
	// the UI can offer a full, unfiltered transcript alongside the
	// summarized statusText/progress. SteamCMD's console interface has no
	// QR-login flow -- that only exists in the graphical Steam client -- so
	// this never contains a scannable code, only text.
	QString consoleOutput() const { return m_consoleOutput; }

	// Downloads and unpacks the official SteamCMD distribution for the
	// current platform into the launcher's app-data folder. No-op if it is
	// already present; call installOrUpdate() directly once bootstrapped.
	Q_INVOKABLE void bootstrap();

	// Logs in with a real Steam account and installs/updates appId into
	// installDir. These games are not distributed anonymously, so a
	// username and password are required. Emits steamGuardRequiredChanged()
	// if this attempt comes back needing a one-time code; answer it with
	// submitSteamGuardCode(), which starts a fresh attempt including it.
	Q_INVOKABLE void installOrUpdate(const QString &appId,
			const QString &installDir,
			const QString &username,
			const QString &password);

	Q_INVOKABLE void submitSteamGuardCode(const QString &code);
	Q_INVOKABLE void cancel();

signals:
	void bootstrappedChanged();
	void busyChanged();
	void statusTextChanged();
	void progressChanged();
	void steamGuardRequiredChanged();
	void consoleOutputChanged();
	void installFinished(bool success, const QString &installDir);
	void bootstrapFinished(bool success);

private:
	QString steamCmdRootDir() const;
	QString steamCmdExecutablePath() const;
	QString bootstrapDownloadUrl() const;

	void setBusy(bool busy);
	void setStatusText(const QString &text);
	void setProgress(int progress);
	void setSteamGuardRequired(bool required);

	void extractBootstrapArchive(const QString &archivePath);
	// One attempt = one SteamCMD process with the full script (login,
	// optionally with guardCode, then app_update) passed as arguments.
	// installOrUpdate() calls this with an empty guardCode; a Guard retry
	// calls it again with one, reusing the credentials from the first call.
	void startAttempt(const QString &appId,
			const QString &installDir,
			const QString &username,
			const QString &password,
			const QString &guardCode);
	void handleProcessOutput(const QString &chunk);
	// Acts on one line of SteamCMD output at a time (also called with the
	// still-unterminated tail line). Operating line-by-line avoids
	// misreading a phrase that arrives split across two separate reads.
	void processLine(const QString &line);

	QNetworkAccessManager *m_network;
	QNetworkReply *m_bootstrapReply = nullptr;
	QProcess *m_process = nullptr;

	QString m_statusText;
	int m_progress = -1;
	bool m_busy = false;
	bool m_steamGuardRequired = false;
	// Set while processing the current attempt's output; read once the
	// process exits to decide what happened.
	bool m_guardNeededThisAttempt = false;
	bool m_successThisAttempt = false;
	QString m_failureReason;

	QString m_pendingInstallDir;
	QString m_pendingAppId;
	QString m_pendingUsername;
	QString m_pendingPassword;
	// A real download makes SteamCMD redraw its progress line many times a
	// second via bare '\r' updates; each is treated as a "line" for parsing.
	// consoleOutput is capped (oldest text dropped) so a long download can't
	// grow it into hundreds of MB, and its changed-signal is coalesced via
	// this timer so the UI (a TextArea bound to it) doesn't try to re-render
	// on every single one of those updates.
	QString m_consoleOutput;
	QString m_pendingLine;
	QTimer m_consoleFlushTimer;
	bool m_consoleOutputDirty = false;
};
