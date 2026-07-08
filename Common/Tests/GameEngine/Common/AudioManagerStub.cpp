// Minimal AudioManager so Common-only tests can link against code that
// touches TheAudio (e.g. the INI AudioEvent/MusicTrack/Dialog parsers). The
// real AudioManager implementation lives per-game (Generals/GeneralsMD) and
// is intentionally not part of the commonized GameEngine library.
//
// AudioManager declares its constructor/destructor out-of-line, so defining
// them here forces the compiler to emit AudioManager's own vtable, which in
// turn requires a body for every virtual method declared in the class (not
// just the pure ones) -- hence the long list of trivial definitions below.
// newAudioEventInfo()/findAudioEventInfo() get real bodies since INI parsing
// tests exercise them; everything else is an inert stand-in.
#include "Common/AudioEventInfo.h"
#include "Common/GameAudio.h"

// AudioEventRTS and AudioRequest are only ever used as pointer parameters
// here (never dereferenced), so the forward declarations already provided by
// GameAudio.h are enough -- deliberately not including their full headers,
// since AudioEventRTS.h pulls in a MemoryPoolObject-based nested class whose
// implicit destructor needs AudioEventRTS::~AudioEventRTS(), which (like the
// rest of AudioEventRTS.cpp) is per-game-only and not linked into this test.

namespace
{

class StubAudioManager : public AudioManager
{
public:
#if defined(_DEBUG) || defined(_INTERNAL)
	virtual void audioDebugDisplay(DebugDisplayInterface *dd, void *userData, FILE *fp) override {}
#endif
	virtual void stopAudio( AudioAffect which ) override {}
	virtual void pauseAudio( AudioAffect which ) override {}
	virtual void resumeAudio( AudioAffect which ) override {}
	virtual void pauseAmbient( Bool shouldPause ) override {}
	virtual void killAudioEventImmediately( AudioHandle audioEvent ) override {}
	virtual void nextMusicTrack( void ) override {}
	virtual void prevMusicTrack( void ) override {}
	virtual Bool isMusicPlaying( void ) const override { return FALSE; }
	virtual Bool hasMusicTrackCompleted( const AsciiString& trackName, Int numberOfTimes ) const override { return FALSE; }
	virtual AsciiString getMusicTrackName( void ) const override { return AsciiString::TheEmptyString; }
	virtual void openDevice( void ) override {}
	virtual void closeDevice( void ) override {}
	virtual void *getDevice( void ) override { return NULL; }
	virtual void notifyOfAudioCompletion( UnsignedInt audioCompleted, UnsignedInt flags ) override {}
	virtual UnsignedInt getProviderCount( void ) const override { return 0; }
	virtual AsciiString getProviderName( UnsignedInt providerNum ) const override { return AsciiString::TheEmptyString; }
	virtual UnsignedInt getProviderIndex( AsciiString providerName ) const override { return 0; }
	virtual void selectProvider( UnsignedInt providerNdx ) override {}
	virtual void unselectProvider( void ) override {}
	virtual UnsignedInt getSelectedProvider( void ) const override { return 0; }
	virtual void setSpeakerType( UnsignedInt speakerType ) override {}
	virtual UnsignedInt getSpeakerType( void ) override { return 0; }
	virtual UnsignedInt getNum2DSamples( void ) const override { return 0; }
	virtual UnsignedInt getNum3DSamples( void ) const override { return 0; }
	virtual UnsignedInt getNumStreams( void ) const override { return 0; }
	virtual Bool doesViolateLimit( AudioEventRTS *event ) const override { return FALSE; }
	virtual Bool isPlayingLowerPriority( AudioEventRTS *event ) const override { return FALSE; }
	virtual Bool isPlayingAlready( AudioEventRTS *event ) const override { return FALSE; }
	virtual Bool isObjectPlayingVoice( UnsignedInt objID ) const override { return FALSE; }
	virtual void adjustVolumeOfPlayingAudio(AsciiString eventName, Real newVolume) override {}
	virtual void removePlayingAudio( AsciiString eventName ) override {}
	virtual void removeAllDisabledAudio() override {}
	virtual Bool has3DSensitiveStreamsPlaying( void ) const override { return FALSE; }
	virtual void *getHandleForBink( void ) override { return NULL; }
	virtual void releaseHandleForBink( void ) override {}
	virtual void friend_forcePlayAudioEventRTS(const AudioEventRTS* eventToPlay) override {}
	virtual void setPreferredProvider(AsciiString providerNdx) override {}
	virtual void setPreferredSpeaker(AsciiString speakerType) override {}
	virtual Real getFileLengthMS( AsciiString strToLoad ) const override { return 0.0f; }
	virtual void closeAnySamplesUsingFile( const void *fileToClose ) override {}

protected:
	virtual void setDeviceListenerPosition( void ) override {}
};

StubAudioManager gStubAudioManager;

} // namespace

AudioManager::AudioManager() {}
AudioManager::~AudioManager() {}

void AudioManager::addTrackName( const AsciiString& trackName ) {}

// SubsystemInterface overrides
void AudioManager::init() {}
void AudioManager::postProcessLoad() {}
void AudioManager::reset() {}
void AudioManager::update() {}

void AudioManager::loseFocus( void ) {}
void AudioManager::regainFocus( void ) {}

AudioHandle AudioManager::addAudioEvent( const AudioEventRTS *eventToAdd ) { return 0; }
void AudioManager::removeAudioEvent( AudioHandle audioEvent ) {}

Bool AudioManager::isValidAudioEvent( const AudioEventRTS *eventToCheck ) const { return FALSE; }
Bool AudioManager::isValidAudioEvent( AudioEventRTS *eventToCheck ) const { return FALSE; }

void AudioManager::setAudioEventEnabled( AsciiString eventToAffect, Bool enable ) {}
void AudioManager::setAudioEventVolumeOverride( AsciiString eventToAffect, Real newVolume ) {}
void AudioManager::removeAudioEvent( AsciiString eventToRemove ) {}
void AudioManager::removeDisabledEvents() {}

void AudioManager::getInfoForAudioEvent( const AudioEventRTS *eventToFindAndFill ) const {}

Bool AudioManager::isCurrentlyPlaying( AudioHandle handle ) { return FALSE; }

UnsignedInt AudioManager::translateSpeakerTypeToUnsignedInt( const AsciiString& speakerType ) { return 0; }
AsciiString AudioManager::translateUnsignedIntToSpeakerType( UnsignedInt speakerType ) { return AsciiString::TheEmptyString; }

Bool AudioManager::isOn( AudioAffect whichToGet ) const { return FALSE; }
void AudioManager::setOn( Bool turnOn, AudioAffect whichToAffect ) {}

void AudioManager::setVolume( Real volume, AudioAffect whichToAffect ) {}
Real AudioManager::getVolume( AudioAffect whichToGet ) { return 0.0f; }

void AudioManager::set3DVolumeAdjustment( Real volumeAdjustment ) {}

void AudioManager::setListenerPosition( const Coord3D *newListenerPos, const Coord3D *newListenerOrientation ) {}
const Coord3D *AudioManager::getListenerPosition( void ) const { return NULL; }

AudioRequest *AudioManager::allocateAudioRequest( Bool useAudioEvent ) { return NULL; }
void AudioManager::releaseAudioRequest( AudioRequest *requestToRelease ) {}
void AudioManager::appendAudioRequest( AudioRequest *m_request ) {}
void AudioManager::processRequestList( void ) {}

AudioEventInfo *AudioManager::newAudioEventInfo( AsciiString newEventName )
{
	AudioEventInfo *eventInfo = findAudioEventInfo(newEventName);
	if (eventInfo)
		return eventInfo;

	eventInfo = newInstance(AudioEventInfo);
	m_allAudioEventInfo[newEventName] = eventInfo;
	return eventInfo;
}

void AudioManager::addAudioEventInfo( AudioEventInfo *newEventInfo )
{
	if (!newEventInfo)
		return;

	m_allAudioEventInfo[newEventInfo->m_audioName] = newEventInfo;
}

AudioEventInfo *AudioManager::findAudioEventInfo( AsciiString eventName ) const
{
	AudioEventInfoHash::const_iterator it = m_allAudioEventInfo.find(eventName);
	if (it == m_allAudioEventInfo.end())
		return NULL;

	return (*it).second;
}

void AudioManager::releaseAudioEventRTS( AudioEventRTS *eventToRelease ) {}

void AudioManager::refreshCachedVariables() {}

Real AudioManager::getAudioLengthMS( const AudioEventRTS *event ) { return 0.0f; }

Bool AudioManager::isMusicAlreadyLoaded(void) const { return FALSE; }

void AudioManager::findAllAudioEventsOfType( AudioType audioType, std::vector<AudioEventInfo*>& allEvents ) {}

Bool AudioManager::isCurrentProviderHardwareAccelerated() { return FALSE; }
Bool AudioManager::isCurrentSpeakerTypeSurroundSound() { return FALSE; }
Bool AudioManager::shouldPlayLocally(const AudioEventRTS *audioEvent) { return FALSE; }
AudioHandle AudioManager::allocateNewHandle( void ) { return 0; }
void AudioManager::removeLevelSpecificAudioEventInfos( void ) {}

AudioManager *TheAudio = &gStubAudioManager;
