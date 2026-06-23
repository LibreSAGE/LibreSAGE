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

#include "DebugWindowDialog.h"

#include <QPlainTextEdit>
#include <QScrollBar>

#include "ui_DebugWindowDialog.h"

DebugWindowDialog::DebugWindowDialog( QWidget *parent )
	: QDialog( parent )
	, m_ui( new Ui::DebugWindowDialog )
{
	m_ui->setupUi( this );

	mStepping = false;
	mRunFast = false;
	mNumberOfStepsAllowed = -1;

	connect( m_ui->Pause, &QPushButton::clicked, this, &DebugWindowDialog::OnPause );
	connect( m_ui->Step, &QPushButton::clicked, this, &DebugWindowDialog::OnStep );
	connect( m_ui->RUN_FAST, &QPushButton::clicked, this, &DebugWindowDialog::OnRunFast );
	connect( m_ui->StepTen, &QPushButton::clicked, this, &DebugWindowDialog::OnStepTen );
	connect( m_ui->ClearWindows, &QPushButton::clicked, this, &DebugWindowDialog::OnClearWindows );
}

DebugWindowDialog::~DebugWindowDialog()
{
	delete m_ui;
}

void DebugWindowDialog::ForcePause(void)
{
	mNumberOfStepsAllowed = 0;
	_UpdatePauseButton();
}

void DebugWindowDialog::ForceContinue(void)
{
	mNumberOfStepsAllowed = -1;
	_UpdatePauseButton();
}

void DebugWindowDialog::OnPause()
{
	if (mNumberOfStepsAllowed < 0) {
		mNumberOfStepsAllowed = 0;
	} else {
		mNumberOfStepsAllowed = -1;
	}
	_UpdatePauseButton();
}

void DebugWindowDialog::OnStep()
{
	mStepping = true;
	mNumberOfStepsAllowed = 1;
	_UpdatePauseButton();
}

void DebugWindowDialog::OnRunFast()
{
	mRunFast = m_ui->RUN_FAST->isChecked();
}

void DebugWindowDialog::OnStepTen()
{
	mStepping = true;
	mNumberOfStepsAllowed = 10;
	_UpdatePauseButton();
}

void DebugWindowDialog::OnClearWindows()
{
	mMessages.clear();
	mVariables.clear();
	_RebuildMesgString();
	_RebuildVarsString();
}

bool DebugWindowDialog::CanProceed(void)
{
	if (mNumberOfStepsAllowed < 0) {
		return true;
	} else if (mNumberOfStepsAllowed == 0) {
		if (mStepping) {
			mStepping = false;
			_UpdatePauseButton();
		}
		return false;
	}

	--mNumberOfStepsAllowed;
	return true;
}

bool DebugWindowDialog::RunAppFast(void)
{
	return mRunFast;
}

void DebugWindowDialog::AppendMessage(const std::string& messageToAppend)
{
	mMessages.push_back(messageToAppend);
	_RebuildMesgString();
}

void DebugWindowDialog::AdjustVariable(const std::string& varName, const std::string& varValue)
{
	for (PairString &var : mVariables) {
		if (var.first == varName) {
			var.second = varValue;
			_RebuildVarsString();
			return;
		}
	}

	PairString newPair;
	newPair.first = varName;
	newPair.second = varValue;
	mVariables.push_back(newPair);

	_RebuildVarsString();
}

void DebugWindowDialog::SetFrameNumber(int frameNumber)
{
	m_ui->FrameNumber->setText(QString::number(frameNumber));
}

void DebugWindowDialog::_RebuildVarsString(void)
{
	std::string varsString;
	for (const PairString &var : mVariables) {
		varsString += var.first;
		varsString += " = ";
		varsString += var.second;
		varsString += "\n";
	}

	m_ui->Variables->setPlainText( QString::fromStdString( varsString ) );
}

void DebugWindowDialog::_RebuildMesgString(void)
{
	std::string mesgString;
	for (const std::string &msg : mMessages) {
		mesgString += msg;
		mesgString += "\n";
	}

	m_ui->Messages->setPlainText( QString::fromStdString( mesgString ) );
	// Scroll to the bottom so the newest message is visible.
	m_ui->Messages->verticalScrollBar()->setValue( m_ui->Messages->verticalScrollBar()->maximum() );
}

void DebugWindowDialog::_UpdatePauseButton( void )
{
	// The state of the button should reflect !mNumberOfStepsAllowed.
	m_ui->Pause->setChecked( mNumberOfStepsAllowed == 0 );
}
