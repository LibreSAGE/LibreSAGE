#include "InstallationModel.h"

InstallationModel::InstallationModel(QObject *parent) : QAbstractListModel(parent)
{
}

int InstallationModel::rowCount(const QModelIndex &parent) const
{
	if (parent.isValid())
		return 0;
	return m_installations.size();
}

QVariant InstallationModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid() || index.row() < 0 || index.row() >= m_installations.size())
		return QVariant();

	const Installation *installation = m_installations.at(index.row());
	switch (role)
	{
		case GameIdRole:
			return installation->gameId();
		case DisplayNameRole:
			return installation->displayName();
		case InstallPathRole:
			return installation->installPath();
		case ExecutablePathRole:
			return installation->executablePath();
		case CoverArtRole:
			return installation->coverArt();
		case SourceRole:
			return installation->source();
		default:
			return QVariant();
	}
}

QHash<int, QByteArray> InstallationModel::roleNames() const
{
	return {
			{GameIdRole, "gameId"},
			{DisplayNameRole, "displayName"},
			{InstallPathRole, "installPath"},
			{ExecutablePathRole, "executablePath"},
			{CoverArtRole, "coverArt"},
			{SourceRole, "source"},
	};
}

Installation *InstallationModel::get(int row) const
{
	if (row < 0 || row >= m_installations.size())
		return nullptr;
	return m_installations.at(row);
}

Installation *InstallationModel::findByGameId(const QString &gameId) const
{
	for (Installation *installation : m_installations)
	{
		if (installation->gameId() == gameId)
			return installation;
	}
	return nullptr;
}

void InstallationModel::setInstallations(QList<Installation *> installations)
{
	beginResetModel();
	qDeleteAll(m_installations);
	m_installations = std::move(installations);
	for (Installation *installation : std::as_const(m_installations))
		installation->setParent(this);
	endResetModel();
}

void InstallationModel::clear()
{
	setInstallations({});
}
