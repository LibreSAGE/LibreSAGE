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

// FILE: ParticleEditorDialog.h /////////////////////////////////////////////////
//
// Desc:       Qt6 port of the particle editor main window.  The class is still
//             named DebugWindowDialog to match the rest of the editor source
//             (the sub-panels talk to their parent through this interface).  This
//             window owns the live ParticleSystemTemplate the game handed us and
//             dances on its data directly, exactly like the MFC original.
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <list>
#include <string>
#include <vector>

#include <QMainWindow>

#include "GameClient/ParticleSys.h"

// BaseType.h (pulled in by ParticleSys.h) defines min/max as macros which
// collide with Qt/STL templates used downstream; get rid of them here.
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#define FORMAT_STRING "%.2f"
#define NONE_STRING   "(None)"

struct RGBColorKeyframe;

class CColorAlphaDialog;
class CSwitchesDialog;
class MoreParmsDialog;
class ISwapablePanel;

namespace Ui
{
	class ParticleEditorDialog;
}

#define NUM_EMISSION_TYPES 5
#define NUM_VELOCITY_TYPES 5
#define NUM_PARTICLE_TYPES 3
#define NUM_SHADER_TYPES   3

enum SwitchType
{
	ST_HOLLOW = 0,
	ST_ONESHOT,
	ST_ALIGNXY,
	ST_EMITABOVEGROUNDONLY,
	ST_PARTICLEUPTOWARDSEMITTER,
};

class DebugWindowDialog : public QMainWindow
{
	Q_OBJECT

public:
	explicit DebugWindowDialog( QWidget *parent = nullptr );
	~DebugWindowDialog() override;

	void InitPanel( void );

	// --- interface used by the export boundary -------------------------------
	void addParticleSystem( const char *particleSystem );
	void addThingTemplate( const char *thingTemplate );
	void clearAllParticleSystems( void );
	void clearAllThingTemplates( void );
	Bool hasSelectionChanged( void );
	void getSelectedSystemName( char *bufferToCopyInto ) const;
	void getSelectedParticleAsciiStringParm( int parmNum, char *bufferToCopyInto ) const;
	void updateParticleAsciiStringParm( int parmNum, const char *bufferToCopyFrom );
	void updateCurrentParticleCap( int particleCap );
	void updateCurrentNumParticles( int particleCount );
	int getNewParticleCap( void );

	void updateCurrentParticleSystem( ParticleSystemTemplate *particleTemplate );
	void updateSystemUseParameters( ParticleSystemTemplate *particleTemplate );
	void signalParticleSystemEdit( void );

	// The purpose of these is to add as few friends as possible to the particle
	// system classes.  This class has ALL the access to ParticleSystems and dances
	// on the data directly.  Child panels make calls here.
	void getColorValueFromSystem( Int systemNum, RGBColorKeyframe &colorFrame ) const;
	void updateColorValueToSystem( Int systemNum, const RGBColorKeyframe &colorFrame );

	void getAlphaRangeFromSystem( Int systemNum, ParticleSystemInfo::RandomKeyframe &randomVar ) const;
	void updateAlphaRangeToSystem( Int systemNum, const ParticleSystemInfo::RandomKeyframe &randomVar );

	void getHalfSizeFromSystem( Int coordNum, Real& halfSize ) const;          // 0:X, 1:Y, 2:Z
	void updateHalfSizeToSystem( Int coordNum, const Real &halfSize );

	void getSphereRadiusFromSystem( Real &radius ) const;
	void updateSphereRadiusToSystem( const Real &radius );

	void getCylinderRadiusFromSystem( Real &radius ) const;
	void updateCylinderRadiusToSystem( const Real &radius );

	void getCylinderLengthFromSystem( Real &length ) const;
	void updateCylinderLengthToSystem( const Real &length );

	void getLineFromSystem( Int coordNum, Real& linePoint ) const;             // 0:X1..5:Z2
	void updateLineToSystem( Int coordNum, const Real &linePoint );

	void getVelSphereFromSystem( Int velNum, Real &radius ) const;             // 0:min 1:max
	void updateVelSphereToSystem( Int velNum, const Real &radius );

	void getVelHemisphereFromSystem( Int velNum, Real &radius ) const;         // 0:min 1:max
	void updateVelHemisphereToSystem( Int velNum, const Real &radius );

	void getVelOrthoFromSystem( Int coordNum, Real& ortho ) const;            // 0:Xmin..5:Zmax
	void updateVelOrthoToSystem( Int coordNum, const Real& ortho );

	void getVelCylinderFromSystem( Int coordNum, Real& ortho ) const;
	void updateVelCylinderToSystem( Int coordNum, const Real& ortho );

	void getVelOutwardFromSystem( Int coordNum, Real& ortho ) const;
	void updateVelOutwardToSystem( Int coordNum, const Real& ortho );

	void getParticleNameFromSystem( char *buffer, int buffLen ) const;
	void updateParticleNameToSystem( const char *buffer );
	void getDrawableNameFromSystem( char *buffer, int buffLen ) const;
	void updateDrawableNameToSystem( const char *buffer );

	void getInitialDelayFromSystem( Int parmNum, Real& initialDelay ) const;
	void updateInitialDelayToSystem( Int parmNum, const Real& initialDelay );

	void getBurstDelayFromSystem( Int parmNum, Real& burstDelay ) const;
	void updateBurstDelayToSystem( Int parmNum, const Real& burstDelay );

	void getBurstCountFromSystem( Int parmNum, Real& burstCount ) const;
	void updateBurstCountToSystem( Int parmNum, const Real& burstCount );

	void getColorScaleFromSystem( Int parmNum, Real& colorScale ) const;
	void updateColorScaleToSystem( Int parmNum, const Real& colorScale );

	void getParticleLifetimeFromSystem( Int parmNum, Real& particleLifetime ) const;
	void updateParticleLifetimeToSystem( Int parmNum, const Real& particleLifetime );

	void getParticleSizeFromSystem( Int parmNum, Real& particleSize ) const;
	void updateParticleSizeToSystem( Int parmNum, const Real& particleSize );

	void getStartSizeRateFromSystem( Int parmNum, Real& startSizeRate ) const;
	void updateStartSizeRateToSystem( Int parmNum, const Real& startSizeRate );

	void getSizeRateFromSystem( Int parmNum, Real& sizeRate ) const;
	void updateSizeRateToSystem( Int parmNum, const Real& sizeRate );

	void getSizeDampingFromSystem( Int parmNum, Real& sizeDamping ) const;
	void updateSizeDampingToSystem( Int parmNum, const Real& sizeDamping );

	void getSystemLifetimeFromSystem( Real& systemLifetime ) const;
	void updateSystemLifetimeToSystem( const Real& systemLifetime );

	void getSlaveOffsetFromSystem( Int parmNum, Real& slaveOffset ) const;
	void updateSlaveOffsetToSystem( Int parmNum, const Real& slaveOffset );

	void getDriftVelocityFromSystem( Int parmNum, Real& driftVelocity ) const;
	void updateDriftVelocityToSystem( Int parmNum, const Real& driftVelocity );

	void getSwitchFromSystem( SwitchType switchType, Bool& switchVal ) const;
	void updateSwitchToSystem( SwitchType switchType, const Bool& switchVal );

	void getSlaveSystemFromSystem( char *buffer, Int bufferSize ) const;
	void updateSlaveSystemToSystem( const char *buffer );

	void getPerParticleSystemFromSystem( char *buffer, Int bufferSize ) const;
	void updatePerParticleSystemToSystem( const char *buffer );

	void getWindMotionFromSystem( ParticleSystemInfo::WindMotion& windMotion ) const;
	void updateWindMotionToSystem( const ParticleSystemInfo::WindMotion& windMotion );

	void getPingPongStartAngleFromSystem( Int parmNum, Real& angle ) const;
	void updatePingPongStartAngleToSystem( Int parmNum, const Real& angle );

	void getPingPongEndAngleFromSystem( Int parmNum, Real& angle ) const;
	void updatePingPongEndAngleToSystem( Int parmNum, const Real& angle );

	void getWindAngleChangeFromSystem( Int parmNum, Real& angle ) const;
	void updateWindAngleChangeToSystem( Int parmNum, const Real& angle );

	Bool shouldWriteINI( void );
	Bool hasRequestedReload( void );
	Bool shouldBusyWait( void );
	Bool shouldUpdateParticleCap( void );
	Bool shouldReloadTextures( void );
	Bool shouldKillAllParticleSystems( void );

	const std::list<std::string> &getAllThingTemplates( void ) const { return m_listOfThingTemplates; }
	const std::list<std::string> &getAllParticleSystems( void ) const { return m_listOfParticleSystems; }

	ParticleSystemTemplate *getCurrentParticleSystem( void ) { return m_particleSystem; }

private slots:
	void OnParticleSystemChange();  // the current particle system isn't the same as the previous system
	void OnParticleSystemEdit();    // this system has been edited
	void OnKillAllParticleSystems();
	void OnEditColorAlpha();
	void OnEditMoreParms();
	void OnEditSwitches();
	void OnPushSave();
	void OnReloadTextures();
	void OnReloadSystem();
	void OnReloadCurrent();
	void OnReloadAll();
	void OnSaveCurrent();
	void OnSaveAll();
	void OnParticleCapEdit();

private:
	void appendParticleSystemToList( const std::string &rString );
	void appendThingTemplateToList( const std::string &rString );

	// if true, updates the UI from the Particle System.
	// if false, updates the Particle System from the UI
	void performUpdate( Bool toUI );

	Ui::ParticleEditorDialog *m_ui;

	Bool m_changeHasOcurred;
	ParticleSystemTemplate *m_particleSystem;
	std::list<std::string> m_listOfThingTemplates;
	std::vector<std::string> m_particleParmValues;
	std::list<std::string> m_listOfParticleSystems;

	Bool m_shouldWriteINI;
	Bool m_showColorDlg;
	Bool m_showSwitchesDlg;
	Bool m_showMoreParmsDlg;
	Bool m_shouldReload;
	Bool m_shouldBusyWait;
	Bool m_shouldUpdateParticleCap;
	Bool m_shouldReloadTextures;
	Bool m_shouldKillAllParticleSystems;

	CColorAlphaDialog *m_colorAlphaDialog;
	CSwitchesDialog *m_switchesDialog;
	MoreParmsDialog *m_moreParmsDialog;

	int m_activeEmissionPage;
	int m_activeVelocityPage;
	int m_activeParticlePage;

	ISwapablePanel *m_emissionTypePanels[NUM_EMISSION_TYPES];
	ISwapablePanel *m_velocityTypePanels[NUM_VELOCITY_TYPES];
	ISwapablePanel *m_particleTypePanels[NUM_PARTICLE_TYPES];
};
