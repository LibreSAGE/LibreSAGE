#include "GameCatalogModel.h"

#include "GameCatalog.h"

GameCatalogModel::GameCatalogModel(QObject *parent) : QAbstractListModel(parent)
{
}

int GameCatalogModel::rowCount(const QModelIndex &parent) const
{
	if (parent.isValid())
		return 0;
	return gameCatalog().size();
}

QVariant GameCatalogModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid() || index.row() < 0 || index.row() >= gameCatalog().size())
		return QVariant();

	const GameDefinition &gameDef = gameCatalog().at(index.row());
	switch (role)
	{
		case GameIdRole:
			return gameDef.id;
		case DisplayNameRole:
			return gameDef.displayName;
		case CoverArtRole:
			return gameDef.coverArt;
		default:
			return QVariant();
	}
}

QVariantMap GameCatalogModel::get(int row) const
{
	if (row < 0 || row >= gameCatalog().size())
		return {};

	const QModelIndex idx = index(row, 0);
	return {
			{QStringLiteral("gameId"), data(idx, GameIdRole)},
			{QStringLiteral("displayName"), data(idx, DisplayNameRole)},
			{QStringLiteral("coverArt"), data(idx, CoverArtRole)},
	};
}

QString GameCatalogModel::requiresBaseGameId(const QString &gameId) const
{
	const GameDefinition *gameDef = findGameDefinition(gameId);
	return gameDef ? gameDef->requiresBaseGameId : QString();
}

QString GameCatalogModel::displayNameForId(const QString &gameId) const
{
	const GameDefinition *gameDef = findGameDefinition(gameId);
	return gameDef ? gameDef->displayName : QString();
}

bool GameCatalogModel::hasSteamRelease(const QString &gameId) const
{
	const GameDefinition *gameDef = findGameDefinition(gameId);
	return gameDef && gameDef->hasSteamRelease;
}

QHash<int, QByteArray> GameCatalogModel::roleNames() const
{
	return {
			{GameIdRole, "gameId"},
			{DisplayNameRole, "displayName"},
			{CoverArtRole, "coverArt"},
	};
}
