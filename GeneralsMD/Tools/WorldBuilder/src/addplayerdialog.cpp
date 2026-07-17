/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**  Copyright 2026 Stephan Vedder
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// addplayerdialog.cpp : the "add a player to the map" dialog (Qt6 port).

#include <QComboBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QVBoxLayout>

#include "addplayerdialog.h"

#include "CUndoable.h"
#include "WorldBuilderDoc.h"
#include "Common/PlayerTemplate.h"
#include "Common/WellKnownKeys.h"
#include "GameLogic/SidesList.h"

AddPlayerDialog::AddPlayerDialog(AsciiString side, QWidget *parent) :
	QDialog(parent),
	m_side(side),
	m_factions(NULL)
{
	setWindowTitle("Add Player");
	setModal(true);

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->addWidget(new QLabel("Add a player of faction:", this));

	m_factions = new QComboBox(this);
	m_factions->setEditable(true);
	layout->addWidget(m_factions);

	if (ThePlayerTemplateStore)
	{
		for (int i = 0; i < ThePlayerTemplateStore->getPlayerTemplateCount(); i++)
		{
			const PlayerTemplate* pt = ThePlayerTemplateStore->getNthPlayerTemplate(i);
			if (!pt)
				continue;
			if (m_side.isEmpty() || m_side == pt->getSide())
				m_factions->addItem(QString::fromUtf8(pt->getName().str()));
		}
	}
	if (m_factions->count() > 0)
		m_factions->setCurrentIndex(0);

	QDialogButtonBox *buttons = new QDialogButtonBox(
		QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	connect(buttons, &QDialogButtonBox::accepted, this, &AddPlayerDialog::accept);
	connect(buttons, &QDialogButtonBox::rejected, this, &AddPlayerDialog::reject);
	layout->addWidget(buttons);
}

void AddPlayerDialog::accept()
{
	if (!ThePlayerTemplateStore)
	{
		QDialog::reject();
		return;
	}

	AsciiString name(m_factions->currentText().toUtf8().constData());

	const PlayerTemplate* pt = ThePlayerTemplateStore->findPlayerTemplate(NAMEKEY(name));
	if (pt)
	{
		m_addedSide = pt->getName();
		SidesList newSides = *TheSidesList;
		newSides.addPlayerByTemplate(m_addedSide);
		Bool modified = newSides.validateSides();
		DEBUG_ASSERTLOG(!modified, ("had to clean up sides in AddPlayerDialog::accept"));

		CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
		if (pDoc) {
			SidesListUndoable *pUndo = new SidesListUndoable(newSides, pDoc);
			pDoc->AddAndDoUndoable(pUndo);
			REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
		}
	}

	QDialog::accept();
}

void AddPlayerDialog::reject()
{
	m_addedSide.clear();
	QDialog::reject();
}
