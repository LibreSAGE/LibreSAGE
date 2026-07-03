#include "GameLogic/FPUControl.h"

void setFPMode( void )
{
  // Set floating point round mode to CHOP, which only comes
  // into play when precision is exceeded.  This is necessary
  // for the fast float to int routines used elsewhere in the
  // system.
	//
	// Also set floating point precision to low.  It could be
	// anything as long as it is consistent, really, but this
	// is in the (vain?) hope of any slight speed boost.
	//
#ifdef _WIN32
	_fpreset();

	UnsignedInt curVal = _statusfp();
	UnsignedInt newVal = curVal;
	newVal = (newVal & ~_MCW_RC) | (_RC_NEAR & _MCW_RC);
	//newVal = (newVal & ~_MCW_RC) | (_RC_CHOP & _MCW_RC);
	newVal = (newVal & ~_MCW_PC) | (_PC_24   & _MCW_PC);

	_controlfp(newVal, _MCW_PC | _MCW_RC);
#endif
}