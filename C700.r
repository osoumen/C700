#include <AudioUnit/AudioUnit.r>

#include "C700Version.h"

// Note that resource IDs must be spaced 2 apart for the 'STR ' name and description
#define kAudioUnitResID_C700				1000

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ C700~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#define RES_ID			kAudioUnitResID_C700
#define COMP_TYPE		kAudioUnitType_MusicDevice
#define COMP_SUBTYPE	C700_COMP_SUBTYPE
#define COMP_MANUF		C700_COMP_MANF	

#define VERSION			kC700Version
#define NAME			"osoumen: C700"
#define DESCRIPTION		"C700 AU"
#define ENTRY_POINT		"C700Entry"

#include "AUResources.r"
