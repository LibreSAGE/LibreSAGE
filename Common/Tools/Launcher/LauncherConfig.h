#pragma once

#include <QObject>
#include <QString>
#include <QTranslator>
#include <QUrl>
#include <qqmlintegration.h>

class QQmlEngine;
class QJSEngine;

// Small persisted settings surface exposed to QML: lets the user override the
// SteamCMD app ids (the catalog ships with these blank, see GameCatalog.h)
// and remember manually-added install folders without recompiling.
//
// Also owns runtime language switching: as a QML_SINGLETON with a create()
// factory (see below), it receives the QQmlEngine pointer at construction,
// which is what lets it call QQmlEngine::retranslate() to make every
// qsTr()-based QML binding re-evaluate immediately when the language
// changes, with no restart required.
class LauncherConfig : public QObject
{
	Q_OBJECT
	QML_ELEMENT
	QML_SINGLETON

	Q_PROPERTY(QString generalsSteamAppId READ generalsSteamAppId WRITE setGeneralsSteamAppId NOTIFY generalsSteamAppIdChanged)
	Q_PROPERTY(QString zeroHourSteamAppId READ zeroHourSteamAppId WRITE setZeroHourSteamAppId NOTIFY zeroHourSteamAppIdChanged)
	Q_PROPERTY(QString steamInstallDir READ steamInstallDir WRITE setSteamInstallDir NOTIFY steamInstallDirChanged)
	Q_PROPERTY(bool windowedMode READ windowedMode WRITE setWindowedMode NOTIFY windowedModeChanged)
	Q_PROPERTY(bool showInstallPaths READ showInstallPaths WRITE setShowInstallPaths NOTIFY showInstallPathsChanged)
	Q_PROPERTY(QString language READ language WRITE setLanguage NOTIFY languageChanged)
	Q_PROPERTY(QString effectiveLanguage READ effectiveLanguage NOTIFY effectiveLanguageChanged)

public:
	// No default argument, deliberately: Qt's QML_SINGLETON factory
	// resolution checks std::is_default_constructible *before* checking for
	// a create(QQmlEngine*, QJSEngine*) factory, so a defaulted parent here
	// would make it silently win over create() below, permanently skipping
	// the translator/retranslate() setup that runtime language switching
	// depends on. Existing C++-side instances (see InstallationFinder) just
	// need to pass nullptr explicitly.
	explicit LauncherConfig(QObject *parent);
	~LauncherConfig() override;

	// QML_SINGLETON factory: called by the QML engine instead of the plain
	// constructor because it exists, handing us the QQmlEngine we need for
	// retranslate().
	static LauncherConfig *create(QQmlEngine *qmlEngine, QJSEngine *jsEngine);

	QString generalsSteamAppId() const;
	void setGeneralsSteamAppId(const QString &appId);

	QString zeroHourSteamAppId() const;
	void setZeroHourSteamAppId(const QString &appId);

	QString steamInstallDir() const;
	void setSteamInstallDir(const QString &dir);

	// Whether GameLauncher passes "-win" (windowed mode) when starting a
	// game. Defaults to on since a fullscreen game grabbing the display is
	// disruptive during dev/test use; the UI lets the user opt out.
	bool windowedMode() const;
	void setWindowedMode(bool windowed);

	// Whether the detail panel shows the on-disk path of a found install.
	// Off by default would be surprising, so this defaults to on; some users
	// just don't want that path visible (e.g. when sharing their screen).
	bool showInstallPaths() const;
	void setShowInstallPaths(bool show);

	// User's language preference: "system", "en", or "de". Defaults to
	// "system", which effectiveLanguage() resolves to "de" or "en" from the
	// OS locale. Stored separately from the resolved value so switching the
	// OS language later is still picked up unless the user explicitly chose
	// a language override.
	QString language() const;
	void setLanguage(const QString &lang);

	// The actual language QML/status text should render in right now: never
	// "system" -- always a concrete, supported code ("en" or "de").
	QString effectiveLanguage() const;

	Q_INVOKABLE QString steamAppId(const QString &gameId) const;

	// Where SteamCMD should install/find a given game: <steamInstallDir>/<gameId>.
	// Kept in sync with InstallationFinder::scanConfiguredDirs().
	Q_INVOKABLE QString installDirFor(const QString &gameId) const;

	// Manually browsed-to folders, scoped per game -- both Generals and Zero
	// Hour ship an executable literally named "generals.exe", so a single
	// shared list would leave InstallationFinder unable to tell whose folder
	// is whose.
	Q_INVOKABLE QStringList manualInstallDirs(const QString &gameId) const;
	Q_INVOKABLE void addManualInstallDir(const QString &gameId, const QUrl &dir);

	// True if the user has previously "reset" (forgotten) this exact install
	// path for this game -- checked by InstallationFinder so a reset install
	// doesn't just reappear on the next scan.
	Q_INVOKABLE bool isInstallIgnored(const QString &gameId, const QString &installPath) const;

	// Forgets a found installation: removes it from the manually-added list
	// if it came from there, and marks the path ignored either way so
	// registry/Steam-library scans don't just re-surface it. Lets the user
	// start over (re-browse or re-install) without it needing to vanish
	// from disk first.
	Q_INVOKABLE void resetInstallation(const QString &gameId, const QString &installPath);

signals:
	void generalsSteamAppIdChanged();
	void zeroHourSteamAppIdChanged();
	void steamInstallDirChanged();
	void windowedModeChanged();
	void showInstallPathsChanged();
	void languageChanged();
	void effectiveLanguageChanged();

private:
	// Installs (or removes, for English -- the .ts source language, needing
	// no translation file at all) the QTranslator matching effectiveLanguage(),
	// then retranslates the QML engine so the change is visible immediately.
	// Follows the pattern from Qt's "Localized Clock" example: reload the
	// same QTranslator instance rather than replacing it.
	void applyTranslator();

	QQmlEngine *m_qmlEngine = nullptr;
	QTranslator m_translator;
};
