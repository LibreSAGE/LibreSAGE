#pragma once

#include <QAbstractListModel>
#include <QList>
#include <qqmlintegration.h>

#include "Installation.h"

class InstallationModel : public QAbstractListModel
{
	Q_OBJECT
	QML_ELEMENT
	QML_UNCREATABLE("InstallationModel is provided by LauncherBackend")

public:
	enum Role
	{
		GameIdRole = Qt::UserRole + 1,
		DisplayNameRole,
		InstallPathRole,
		ExecutablePathRole,
		CoverArtRole,
		SourceRole,
	};
	Q_ENUM(Role)

	explicit InstallationModel(QObject *parent = nullptr);

	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	QVariant data(const QModelIndex &index, int role) const override;
	QHash<int, QByteArray> roleNames() const override;

	Q_INVOKABLE Installation *get(int row) const;
	Q_INVOKABLE Installation *findByGameId(const QString &gameId) const;

	void setInstallations(QList<Installation *> installations);
	void clear();

private:
	QList<Installation *> m_installations;
};
