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

// FILE: CColorAlphaDialog.cpp //////////////////////////////////////////////////
// Desc:       Qt6 port of the colour/alpha keyframe editor.
///////////////////////////////////////////////////////////////////////////////

#include "CColorAlphaDialog.h"
#include "CButtonShowColor.h"
#include "ParticleEditorDialog.h"

#include <QColorDialog>
#include <QLineEdit>
#include <QSettings>

#include "ui_CColorAlphaDialog.h"

CColorAlphaDialog::CColorAlphaDialog( DebugWindowDialog *parentDialog )
	: QDialog( parentDialog )
	, m_ui( new Ui::CColorAlphaDialog )
	, m_dialog( parentDialog )
{
	m_ui->setupUi( this );

	CButtonShowColor *colorButtons[MAX_KEYFRAMES] = {
		m_ui->Color1, m_ui->Color2, m_ui->Color3, m_ui->Color4,
		m_ui->Color5, m_ui->Color6, m_ui->Color7, m_ui->Color8,
	};
	QLineEdit *colorFrames[MAX_KEYFRAMES] = {
		m_ui->CF1_Frame, m_ui->CF2_Frame, m_ui->CF3_Frame, m_ui->CF4_Frame,
		m_ui->CF5_Frame, m_ui->CF6_Frame, m_ui->CF7_Frame, m_ui->CF8_Frame,
	};
	QLineEdit *alphaMins[MAX_KEYFRAMES] = {
		m_ui->AF1_Min, m_ui->AF2_Min, m_ui->AF3_Min, m_ui->AF4_Min,
		m_ui->AF5_Min, m_ui->AF6_Min, m_ui->AF7_Min, m_ui->AF8_Min,
	};
	QLineEdit *alphaMaxs[MAX_KEYFRAMES] = {
		m_ui->AF1_Max, m_ui->AF2_Max, m_ui->AF3_Max, m_ui->AF4_Max,
		m_ui->AF5_Max, m_ui->AF6_Max, m_ui->AF7_Max, m_ui->AF8_Max,
	};
	QLineEdit *alphaFrames[MAX_KEYFRAMES] = {
		m_ui->AF1_Frame, m_ui->AF2_Frame, m_ui->AF3_Frame, m_ui->AF4_Frame,
		m_ui->AF5_Frame, m_ui->AF6_Frame, m_ui->AF7_Frame, m_ui->AF8_Frame,
	};

	for (int i = 0; i < MAX_KEYFRAMES; ++i) {
		m_colorButton[i] = colorButtons[i];
		m_colorFrame[i] = colorFrames[i];
		m_alphaMin[i] = alphaMins[i];
		m_alphaMax[i] = alphaMaxs[i];
		m_alphaFrame[i] = alphaFrames[i];

		connect( m_colorButton[i], &CButtonShowColor::clicked, this, &CColorAlphaDialog::onColorPressed );
		connect( m_colorFrame[i], &QLineEdit::editingFinished, this, &CColorAlphaDialog::onParticleSystemEdit );
		connect( m_alphaMin[i], &QLineEdit::editingFinished, this, &CColorAlphaDialog::onParticleSystemEdit );
		connect( m_alphaMax[i], &QLineEdit::editingFinished, this, &CColorAlphaDialog::onParticleSystemEdit );
		connect( m_alphaFrame[i], &QLineEdit::editingFinished, this, &CColorAlphaDialog::onParticleSystemEdit );
	}
}

CColorAlphaDialog::~CColorAlphaDialog() { delete m_ui; }

void CColorAlphaDialog::InitPanel( void )
{
	// Seed the colour dialog's custom colour slots from the persisted settings,
	// mirroring the MFC editor which loaded 16 custom colours from the profile.
	QSettings settings;
	for (int i = 0; i < 16; ++i) {
		int rgb = settings.value( QString( "CustomColors/Color%1" ).arg( i + 1 ), 0 ).toInt();
		QColorDialog::setCustomColor( i, QColor::fromRgb( (QRgb)rgb ) );
	}
}

void CColorAlphaDialog::performUpdate( bool toUI )
{
	if (!m_dialog) {
		return;
	}

	{	// update the colors
		for (int i = 0; i < MAX_KEYFRAMES; ++i) {
			RGBColorKeyframe colorFrame;

			if (toUI) {
				m_dialog->getColorValueFromSystem( i, colorFrame );

				m_colorButton[i]->setColor( colorFrame.color );
				m_colorFrame[i]->setText( QString::number( colorFrame.frame ) );
			} else {
				colorFrame.color = m_colorButton[i]->getColor();
				colorFrame.frame = m_colorFrame[i]->text().toUInt();
				m_dialog->updateColorValueToSystem( i, colorFrame );
			}
		}
	}

	{	// update the values
		for (int i = 0; i < MAX_KEYFRAMES; ++i) {
			ParticleSystemInfo::RandomKeyframe keyFrame;

			m_dialog->getAlphaRangeFromSystem( i, keyFrame );

			{	// Minimum first
				if (toUI) {
					m_alphaMin[i]->setText( QString::asprintf( FORMAT_STRING, keyFrame.var.getMinimumValue() ) );
				} else {
					keyFrame.var.m_low = m_alphaMin[i]->text().toDouble();
				}
			}

			{	// then maximum
				if (toUI) {
					m_alphaMax[i]->setText( QString::asprintf( FORMAT_STRING, keyFrame.var.getMaximumValue() ) );
				} else {
					keyFrame.var.m_high = m_alphaMax[i]->text().toDouble();
				}
			}

			{	// then the frame
				if (toUI) {
					m_alphaFrame[i]->setText( QString::number( keyFrame.frame ) );
				} else {
					keyFrame.frame = m_alphaFrame[i]->text().toUInt();
				}
			}

			if (toUI) {
				// We're all done.
			} else {
				m_dialog->updateAlphaRangeToSystem( i, keyFrame );
			}
		}
	}
}

void CColorAlphaDialog::onColorPressed()
{
	CButtonShowColor *btn = qobject_cast<CButtonShowColor*>( sender() );
	if (!btn) {
		return;
	}

	int index = -1;
	for (int i = 0; i < MAX_KEYFRAMES; ++i) {
		if (m_colorButton[i] == btn) {
			index = i;
			break;
		}
	}
	if (index < 0) {
		return;
	}

	// CButtonShowColor stores its colour as 0x00RRGGBB; build the matching QColor.
	QColor current = QColor::fromRgb( (QRgb)( 0xFF000000 | (btn->getColor().getAsInt() & 0x00FFFFFF) ) );

	QColor chosen = QColorDialog::getColor( current, this );
	if (chosen.isValid()) {
		// QColor::rgb() returns 0xFFRRGGBB; mask the alpha off for setColor(Int).
		btn->setColor( (Int)( chosen.rgb() & 0x00FFFFFF ) );

		// Persist the dialog's custom colours back to the settings.
		QSettings settings;
		for (int i = 0; i < 16; ++i) {
			settings.setValue( QString( "CustomColors/Color%1" ).arg( i + 1 ),
				(int)QColorDialog::customColor( i ).rgb() );
		}

		onParticleSystemEdit();
	}
}

void CColorAlphaDialog::onParticleSystemEdit()
{
	if (m_dialog) {
		m_dialog->signalParticleSystemEdit();
	}
}
