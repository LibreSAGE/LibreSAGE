/*
**	Command & Conquer Generals(tm)
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

#pragma once

#include <string>
#include <utility>
#include <vector>

#include <QDialog>

typedef std::pair<std::string, std::string> PairString;
typedef std::vector<PairString> VecPairString;
typedef std::vector<std::string> VecString;

namespace Ui
{
	class DebugWindowDialog;
}

class DebugWindowDialog : public QDialog
{
	Q_OBJECT

public:
	explicit DebugWindowDialog( QWidget *parent = nullptr );
	~DebugWindowDialog() override;

	bool CanProceed( void );
	bool RunAppFast( void );
	void AppendMessage( const std::string& messageToAppend );
	void AdjustVariable( const std::string& varName, const std::string& varValue );
	void SetFrameNumber( int frameNumber );
	void ForcePause( void );
	void ForceContinue( void );

private slots:
	void OnPause();
	void OnStep();
	void OnRunFast();
	void OnStepTen();
	void OnClearWindows();

private:
	void _RebuildVarsString( void );
	void _RebuildMesgString( void );
	void _UpdatePauseButton( void );

	Ui::DebugWindowDialog *m_ui;

	int mNumberOfStepsAllowed;	///< -1 = go forever, 0 = stop now, positive = decremented to 0 one frame at a time
	bool mStepping;
	bool mRunFast;

	VecPairString mVariables;
	VecString mMessages;
};
