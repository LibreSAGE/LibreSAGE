#pragma once

#include <QAbstractListModel>
#include <qqmlintegration.h>

// Static list of every game the launcher knows about (see GameCatalog.h),
// independent of whether it is currently installed. QML pairs each row up
// with InstallationModel::findByGameId() to decide whether to show a "Play"
// or "Install" affordance.
class GameCatalogModel : public QAbstractListModel
{
	Q_OBJECT
	QML_ELEMENT

public:
	enum Role
	{
		GameIdRole = Qt::UserRole + 1,
		DisplayNameRole,
		CoverArtRole,
	};
	Q_ENUM(Role)

	explicit GameCatalogModel(QObject *parent = nullptr);

	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	QVariant data(const QModelIndex &index, int role) const override;
	QHash<int, QByteArray> roleNames() const override;

	Q_INVOKABLE QVariantMap get(int row) const;

	// gameId of the base game this one needs preloaded to run (e.g. Zero
	// Hour -> Generals), or an empty string if it's self-contained.
	Q_INVOKABLE QString requiresBaseGameId(const QString &gameId) const;
	Q_INVOKABLE QString displayNameForId(const QString &gameId) const;
	// False for games with no Steam release at all -- hides the SteamCMD
	// install UI rather than offering a control that can never work.
	Q_INVOKABLE bool hasSteamRelease(const QString &gameId) const;
};
