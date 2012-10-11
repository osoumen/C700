#include <AudioUnit/AudioUnit.r>

#include "Chip700Version.h"

// Note that resource IDs must be spaced 2 apart for the 'STR ' name and description
#define kAudioUnitResID_Chip700				1000
#define kAudioUnitResID_Chip700View			2000

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Chip700~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#define RES_ID			kAudioUnitResID_Chip700
#define COMP_TYPE		kAudioUnitType_MusicDevice
#define COMP_SUBTYPE	Chip700_COMP_SUBTYPE
#define COMP_MANUF		Chip700_COMP_MANF	

#define VERSION			kChip700Version
#define NAME			"osoumen: C700"
#define DESCRIPTION		"C700 AU"
#define ENTRY_POINT		"Chip700Entry"

#include "AUResources.r"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Chip700View~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define RES_ID			kAudioUnitResID_Chip700View
#define COMP_TYPE		kAudioUnitCarbonViewComponentType
#define COMP_SUBTYPE	Chip700_COMP_SUBTYPE
#define COMP_MANUF		Chip700_COMP_MANF	

#define VERSION			kChip700Version
#define NAME			"osoumen: C700"
#define DESCRIPTION		"C700 Carbon AU View"
#define ENTRY_POINT		"Chip700ViewEntry"

#include "AUResources.r"