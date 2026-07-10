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

// FILE: MoreParmsDialog.cpp /////////////////////////////////////////////////
// Desc:       Qt6 port of the additional particle system parameters dialog.
///////////////////////////////////////////////////////////////////////////////

#define DEFINE_PARTICLE_SYSTEM_NAMES 1

#include "MoreParmsDialog.h"
#include "ParticleEditorDialog.h"

#include <QLineEdit>
#include <QComboBox>

#include "ui_MoreParmsDialog.h"

static QString fmtReal( Real v ) { return QString::asprintf( FORMAT_STRING, v ); }

// MoreParmsDialog //////////////////////////////////////////////////////////////
MoreParmsDialog::MoreParmsDialog( DebugWindowDialog *parentDialog )
	: QDialog( parentDialog )
	, m_ui( new Ui::MoreParmsDialog )
	, m_dialog( parentDialog )
{
	m_ui->setupUi( this );

	connect( m_ui->InitialDelayMin, &QLineEdit::editingFinished, this, &MoreParmsDialog::onParticleSystemEdit );
	connect( m_ui->InitialDelayMax, &QLineEdit::editingFinished, this, &MoreParmsDialog::onParticleSystemEdit );
	connect( m_ui->BurstDelayMin, &QLineEdit::editingFinished, this, &MoreParmsDialog::onParticleSystemEdit );
	connect( m_ui->BurstDelayMax, &QLineEdit::editingFinished, this, &MoreParmsDialog::onParticleSystemEdit );
	connect( m_ui->BurstCountMin, &QLineEdit::editingFinished, this, &MoreParmsDialog::onParticleSystemEdit );
	connect( m_ui->BurstCountMax, &QLineEdit::editingFinished, this, &MoreParmsDialog::onParticleSystemEdit );
	connect( m_ui->ColorScaleMin, &QLineEdit::editingFinished, this, &MoreParmsDialog::onParticleSystemEdit );
	connect( m_ui->ColorScaleMax, &QLineEdit::editingFinished, this, &MoreParmsDialog::onParticleSystemEdit );
	connect( m_ui->ParticleLifetimeMin, &QLineEdit::editingFinished, this, &MoreParmsDialog::onParticleSystemEdit );
	connect( m_ui->ParticleLifetimeMax, &QLineEdit::editingFinished, this, &MoreParmsDialog::onParticleSystemEdit );
	connect( m_ui->SizeMin, &QLineEdit::editingFinished, this, &MoreParmsDialog::onParticleSystemEdit );
	connect( m_ui->SizeMax, &QLineEdit::editingFinished, this, &MoreParmsDialog::onParticleSystemEdit );
	connect( m_ui->StartSizeRateMin, &QLineEdit::editingFinished, this, &MoreParmsDialog::onParticleSystemEdit );
	connect( m_ui->StartSizeRateMax, &QLineEdit::editingFinished, this, &MoreParmsDialog::onParticleSystemEdit );
	connect( m_ui->SizeRateMin, &QLineEdit::editingFinished, this, &MoreParmsDialog::onParticleSystemEdit );
	connect( m_ui->SizeRateMax, &QLineEdit::editingFinished, this, &MoreParmsDialog::onParticleSystemEdit );
	connect( m_ui->SizeDampingMin, &QLineEdit::editingFinished, this, &MoreParmsDialog::onParticleSystemEdit );
	connect( m_ui->SizeDampingMax, &QLineEdit::editingFinished, this, &MoreParmsDialog::onParticleSystemEdit );
	connect( m_ui->SystemLifetime, &QLineEdit::editingFinished, this, &MoreParmsDialog::onParticleSystemEdit );
	connect( m_ui->SlaveOffsetX, &QLineEdit::editingFinished, this, &MoreParmsDialog::onParticleSystemEdit );
	connect( m_ui->SlaveOffsetY, &QLineEdit::editingFinished, this, &MoreParmsDialog::onParticleSystemEdit );
	connect( m_ui->SlaveOffsetZ, &QLineEdit::editingFinished, this, &MoreParmsDialog::onParticleSystemEdit );
	connect( m_ui->DriftVelocityX, &QLineEdit::editingFinished, this, &MoreParmsDialog::onParticleSystemEdit );
	connect( m_ui->DriftVelocityY, &QLineEdit::editingFinished, this, &MoreParmsDialog::onParticleSystemEdit );
	connect( m_ui->DriftVelocityZ, &QLineEdit::editingFinished, this, &MoreParmsDialog::onParticleSystemEdit );
	connect( m_ui->WindAngleChangeMin, &QLineEdit::editingFinished, this, &MoreParmsDialog::onParticleSystemEdit );
	connect( m_ui->WindAngleChangeMax, &QLineEdit::editingFinished, this, &MoreParmsDialog::onParticleSystemEdit );
	connect( m_ui->WindPingPongStartAngleMin, &QLineEdit::editingFinished, this, &MoreParmsDialog::onParticleSystemEdit );
	connect( m_ui->WindPingPongStartAngleMax, &QLineEdit::editingFinished, this, &MoreParmsDialog::onParticleSystemEdit );
	connect( m_ui->WindPingPongEndAngleMin, &QLineEdit::editingFinished, this, &MoreParmsDialog::onParticleSystemEdit );
	connect( m_ui->WindPingPongEndAngleMax, &QLineEdit::editingFinished, this, &MoreParmsDialog::onParticleSystemEdit );

	connect( m_ui->SlaveSystem, &QComboBox::activated, this, &MoreParmsDialog::onParticleSystemEdit );
	connect( m_ui->PerParticleSystem, &QComboBox::activated, this, &MoreParmsDialog::onParticleSystemEdit );
	connect( m_ui->WindMotion, &QComboBox::activated, this, &MoreParmsDialog::onParticleSystemEdit );
}

MoreParmsDialog::~MoreParmsDialog() { delete m_ui; }

void MoreParmsDialog::InitPanel( void )
{
	for (int i = 1; WindMotionNames[i]; ++i) {
		m_ui->WindMotion->addItem( WindMotionNames[i] );
	}
	m_ui->WindMotion->setCurrentIndex( 0 );
}

void MoreParmsDialog::performUpdate( bool toUI )
{
	if (!m_dialog) {
		return;
	}

	{	// initial delay
		struct { QLineEdit *edit; int parm; } fields[] = {
			{ m_ui->InitialDelayMin, 0 }, { m_ui->InitialDelayMax, 1 },
		};
		for (auto &f : fields) {
			Real initialDelay;
			if (toUI) {
				m_dialog->getInitialDelayFromSystem( f.parm, initialDelay );
				f.edit->setText( fmtReal( initialDelay ) );
			} else {
				initialDelay = f.edit->text().toDouble();
				m_dialog->updateInitialDelayToSystem( f.parm, initialDelay );
			}
		}
	}

	{	// burst delay
		struct { QLineEdit *edit; int parm; } fields[] = {
			{ m_ui->BurstDelayMin, 0 }, { m_ui->BurstDelayMax, 1 },
		};
		for (auto &f : fields) {
			Real burstDelay;
			if (toUI) {
				m_dialog->getBurstDelayFromSystem( f.parm, burstDelay );
				f.edit->setText( fmtReal( burstDelay ) );
			} else {
				burstDelay = f.edit->text().toDouble();
				m_dialog->updateBurstDelayToSystem( f.parm, burstDelay );
			}
		}
	}

	{	// burst count
		struct { QLineEdit *edit; int parm; } fields[] = {
			{ m_ui->BurstCountMin, 0 }, { m_ui->BurstCountMax, 1 },
		};
		for (auto &f : fields) {
			Real burstCount;
			if (toUI) {
				m_dialog->getBurstCountFromSystem( f.parm, burstCount );
				f.edit->setText( fmtReal( burstCount ) );
			} else {
				burstCount = f.edit->text().toDouble();
				m_dialog->updateBurstCountToSystem( f.parm, burstCount );
			}
		}
	}

	{	// color scale
		struct { QLineEdit *edit; int parm; } fields[] = {
			{ m_ui->ColorScaleMin, 0 }, { m_ui->ColorScaleMax, 1 },
		};
		for (auto &f : fields) {
			Real colorScale;
			if (toUI) {
				m_dialog->getColorScaleFromSystem( f.parm, colorScale );
				f.edit->setText( fmtReal( colorScale ) );
			} else {
				colorScale = f.edit->text().toDouble();
				m_dialog->updateColorScaleToSystem( f.parm, colorScale );
			}
		}
	}

	{	// particle lifetime
		struct { QLineEdit *edit; int parm; } fields[] = {
			{ m_ui->ParticleLifetimeMin, 0 }, { m_ui->ParticleLifetimeMax, 1 },
		};
		for (auto &f : fields) {
			Real particleLifetime;
			if (toUI) {
				m_dialog->getParticleLifetimeFromSystem( f.parm, particleLifetime );
				f.edit->setText( fmtReal( particleLifetime ) );
			} else {
				particleLifetime = f.edit->text().toDouble();
				m_dialog->updateParticleLifetimeToSystem( f.parm, particleLifetime );
			}
		}
	}

	{	// particle size
		struct { QLineEdit *edit; int parm; } fields[] = {
			{ m_ui->SizeMin, 0 }, { m_ui->SizeMax, 1 },
		};
		for (auto &f : fields) {
			Real particleSize;
			if (toUI) {
				m_dialog->getParticleSizeFromSystem( f.parm, particleSize );
				f.edit->setText( fmtReal( particleSize ) );
			} else {
				particleSize = f.edit->text().toDouble();
				m_dialog->updateParticleSizeToSystem( f.parm, particleSize );
			}
		}
	}

	{	// start size rate
		struct { QLineEdit *edit; int parm; } fields[] = {
			{ m_ui->StartSizeRateMin, 0 }, { m_ui->StartSizeRateMax, 1 },
		};
		for (auto &f : fields) {
			Real startSizeRate;
			if (toUI) {
				m_dialog->getStartSizeRateFromSystem( f.parm, startSizeRate );
				f.edit->setText( fmtReal( startSizeRate ) );
			} else {
				startSizeRate = f.edit->text().toDouble();
				m_dialog->updateStartSizeRateToSystem( f.parm, startSizeRate );
			}
		}
	}

	{	// size rate
		struct { QLineEdit *edit; int parm; } fields[] = {
			{ m_ui->SizeRateMin, 0 }, { m_ui->SizeRateMax, 1 },
		};
		for (auto &f : fields) {
			Real sizeRate;
			if (toUI) {
				m_dialog->getSizeRateFromSystem( f.parm, sizeRate );
				f.edit->setText( fmtReal( sizeRate ) );
			} else {
				sizeRate = f.edit->text().toDouble();
				m_dialog->updateSizeRateToSystem( f.parm, sizeRate );
			}
		}
	}

	{	// size damping
		struct { QLineEdit *edit; int parm; } fields[] = {
			{ m_ui->SizeDampingMin, 0 }, { m_ui->SizeDampingMax, 1 },
		};
		for (auto &f : fields) {
			Real sizeDamping;
			if (toUI) {
				m_dialog->getSizeDampingFromSystem( f.parm, sizeDamping );
				f.edit->setText( fmtReal( sizeDamping ) );
			} else {
				sizeDamping = f.edit->text().toDouble();
				m_dialog->updateSizeDampingToSystem( f.parm, sizeDamping );
			}
		}
	}

	{	// system lifetime
		Real systemLifetime;
		if (toUI) {
			m_dialog->getSystemLifetimeFromSystem( systemLifetime );
			m_ui->SystemLifetime->setText( fmtReal( systemLifetime ) );
		} else {
			systemLifetime = m_ui->SystemLifetime->text().toDouble();
			m_dialog->updateSystemLifetimeToSystem( systemLifetime );
		}
	}

	{	// slave position offset
		struct { QLineEdit *edit; int parm; } fields[] = {
			{ m_ui->SlaveOffsetX, 0 }, { m_ui->SlaveOffsetY, 1 }, { m_ui->SlaveOffsetZ, 2 },
		};
		for (auto &f : fields) {
			Real slaveOffset;
			if (toUI) {
				m_dialog->getSlaveOffsetFromSystem( f.parm, slaveOffset );
				f.edit->setText( fmtReal( slaveOffset ) );
			} else {
				slaveOffset = f.edit->text().toDouble();
				m_dialog->updateSlaveOffsetToSystem( f.parm, slaveOffset );
			}
		}
	}

	{	// drift velocity
		struct { QLineEdit *edit; int parm; } fields[] = {
			{ m_ui->DriftVelocityX, 0 }, { m_ui->DriftVelocityY, 1 }, { m_ui->DriftVelocityZ, 2 },
		};
		for (auto &f : fields) {
			Real driftVelocity;
			if (toUI) {
				m_dialog->getDriftVelocityFromSystem( f.parm, driftVelocity );
				f.edit->setText( fmtReal( driftVelocity ) );
			} else {
				driftVelocity = f.edit->text().toDouble();
				m_dialog->updateDriftVelocityToSystem( f.parm, driftVelocity );
			}
		}
	}

	{	// slave system
		QComboBox *pCombo = m_ui->SlaveSystem;
		if (pCombo->count() == 0) {
			// This is done here because InitPanel is called before Particle Systems have been sent over.
			pCombo->addItem( NONE_STRING );
			const std::list<std::string> &r = m_dialog->getAllParticleSystems();
			for (std::list<std::string>::const_iterator cit = r.begin(); cit != r.end(); ++cit) {
				pCombo->addItem( QString::fromStdString( *cit ) );
			}
		}

		if (toUI) {
			char buff[128];
			m_dialog->getSlaveSystemFromSystem( buff, 127 );
			if (buff[0] == 0) {
				pCombo->setCurrentText( NONE_STRING );
			} else {
				pCombo->setCurrentText( buff );
			}
		} else {
			if (pCombo->currentIndex() >= 0) {
				QString text = pCombo->currentText();
				if (text == NONE_STRING) {
					m_dialog->updateSlaveSystemToSystem( "" );
				} else {
					m_dialog->updateSlaveSystemToSystem( text.toLatin1().constData() );
				}
			}
		}
	}

	{	// per-particle system
		QComboBox *pCombo = m_ui->PerParticleSystem;
		if (pCombo->count() == 0) {
			// This is done here because InitPanel is called before Particle Systems have been sent over.
			pCombo->addItem( NONE_STRING );
			const std::list<std::string> &r = m_dialog->getAllParticleSystems();
			for (std::list<std::string>::const_iterator cit = r.begin(); cit != r.end(); ++cit) {
				pCombo->addItem( QString::fromStdString( *cit ) );
			}
		}

		if (toUI) {
			char buff[128];
			m_dialog->getPerParticleSystemFromSystem( buff, 127 );
			if (buff[0] == 0) {
				int ndx = pCombo->findText( NONE_STRING );
				pCombo->setCurrentIndex( ndx );
			} else {
				int ndx = pCombo->findText( buff );
				pCombo->setCurrentIndex( ndx );
			}
		} else {
			if (pCombo->currentIndex() >= 0) {
				QString text = pCombo->currentText();
				if (text == NONE_STRING) {
					m_dialog->updatePerParticleSystemToSystem( "" );
				} else {
					m_dialog->updatePerParticleSystemToSystem( text.toLatin1().constData() );
				}
			}
		}
	}

	{	// ping pong wind start angle
		struct { QLineEdit *edit; int parm; } fields[] = {
			{ m_ui->WindPingPongStartAngleMin, 0 }, { m_ui->WindPingPongStartAngleMax, 1 },
		};
		for (auto &f : fields) {
			Real angle;
			if (toUI) {
				m_dialog->getPingPongStartAngleFromSystem( f.parm, angle );
				f.edit->setText( fmtReal( angle ) );
			} else {
				angle = f.edit->text().toDouble();
				m_dialog->updatePingPongStartAngleToSystem( f.parm, angle );
			}
		}
	}

	{	// ping pong wind end angle
		struct { QLineEdit *edit; int parm; } fields[] = {
			{ m_ui->WindPingPongEndAngleMin, 0 }, { m_ui->WindPingPongEndAngleMax, 1 },
		};
		for (auto &f : fields) {
			Real angle;
			if (toUI) {
				m_dialog->getPingPongEndAngleFromSystem( f.parm, angle );
				f.edit->setText( fmtReal( angle ) );
			} else {
				angle = f.edit->text().toDouble();
				m_dialog->updatePingPongEndAngleToSystem( f.parm, angle );
			}
		}
	}

	{	// wind angle change
		struct { QLineEdit *edit; int parm; } fields[] = {
			{ m_ui->WindAngleChangeMin, 0 }, { m_ui->WindAngleChangeMax, 1 },
		};
		for (auto &f : fields) {
			Real angle;
			if (toUI) {
				m_dialog->getWindAngleChangeFromSystem( f.parm, angle );
				f.edit->setText( fmtReal( angle ) );
			} else {
				angle = f.edit->text().toDouble();
				m_dialog->updateWindAngleChangeToSystem( f.parm, angle );
			}
		}
	}

	{	// wind motion
		QComboBox *pCombo = m_ui->WindMotion;
		if (toUI) {
			ParticleSystemInfo::WindMotion windMotion;
			m_dialog->getWindMotionFromSystem( windMotion );
			int ndx = pCombo->findText( WindMotionNames[(long) windMotion] );
			pCombo->setCurrentIndex( ndx );
		} else {
			int selndx = pCombo->currentIndex();
			if (selndx >= 0) {
				m_dialog->updateWindMotionToSystem( (ParticleSystemInfo::WindMotion)(selndx + 1) );
			}
		}
	}
}

void MoreParmsDialog::onParticleSystemEdit()
{
	if (m_dialog) {
		m_dialog->signalParticleSystemEdit();
	}
}
