/*
**	Command & Conquer Generals(tm)
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

#pragma once

#include <QDialog>

class DebugWindowDialog;

namespace Ui
{
	class CSwitchesDialog;
}

class CSwitchesDialog : public QDialog
{
	Q_OBJECT

public:
	explicit CSwitchesDialog( DebugWindowDialog *parentDialog );
	~CSwitchesDialog() override;

	void InitPanel( void );

	// if true, updates the UI from the Particle System.
	// if false, updates the Particle System from the UI
	void performUpdate( bool toUI );

private slots:
	void onParticleSystemEdit();

private:
	Ui::CSwitchesDialog *m_ui;
	DebugWindowDialog *m_dialog;
};
