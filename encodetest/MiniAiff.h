
/*

	MiniAiff
	(c) 2003-2009 Stephan M. Bernsee
	http://www.dspdimension.com
	
	Version 2.0.1
	26.05.2009
	
	Abstract: This is a super-mini implementation of AIFF file read and write functionality.
	The routines provided read sample data from an AIFF file and write sample data to another
	AIFF file. You don't have to worry about *anything*, these routines are as simple as they
	can be.
	
	All routines provided here are inherently cross-platform and are provided as MacOS X Universal
	Binaty and Windows 32 MSVC 6 static library calls.
	
	They provide a basis on which you can quickly test audio stuff, and functionality with
	which you can dump any data you choose to generate into an audio file. Be careful though
	when you are listening to data that have nothing to do with audio - they may harm your 
	ears and speakers if you listen to them at high volumes.
	
	Supports reading from all AIFF file formats (no compression).
	
	Below are the function prototypes and an explanation



	Version History:
	===============

	Version 2.0.1
	26.05.2009
 
	Removed inconsistencies in the API
 
 
	Version 2.0.0
	23.05.2009
 
	Got rid of calls to system specific file selection dialogs
	Changed the calls to be ANSI C compatible
	Fixed tons of bugs
 
	Version 1.5.0
	26.04.2005
	
	Added libMiniAiffLib.a (MiniAiff Xcode Carbon library). Made some minor changes and
	fixes.


	Version 1.4.0
	02.11.2003
	
	Added multi-channel support for mAiffReadData(). Added mAiffGetFilename() and
	mAiffPutFilename() on all supported platforms to overcome the need for nasty filename
	extraction on the Mac and to streamline operation. Make sure you include the COMdlg32.lib
	in your project when compiling for Windows, otherwise you will get a link error.


	Version 1.3.0
	18.08.2003
	
	Added mAiffAddData(). Very cool for doing OLA-type DSP algorithms because you can do it
	without all the output buffer bookkeeping.


	Version 1.2.2
	08.06.2003
	
	Fixed a bug evident when reading from 24bit files. Added new set of routines to support
	getting more information from a sound file (mAiffGetLowKey, mAiffGetHighKey, mAiffGetRootKey
	mAiffGetGain, mAiffGetDetune). These routines require an Instrument chunk being present
	in the file.


	Version 1.2.1
	04.06.2003
	
	Extended error checking for read and write routines.


	Version 1.2.0
	03.06.2003
	
	mAiffReadAppData() and mAiffWriteAppData() now correctly identify the MiniAiff application
	signature when reading or writing application specific data,


	Version 1.1.0
	23.05.2003
	
	Included support for sample rates other than 44.1kHz and for reading 8, 24 and 32 bit
	Files, mono and stereo. Included routines to analyze the format and return information.
	numFrames is no longer limited to a specific value.


	Version 1.0.0
	18.05.2003
	
	First release: Only 16bit, 44.1kHz, mono, uncompressed AIFF files are supported.
	For all numFrames variables, a maximum of 16384 frames are supported.
	


	Here is a list of return error codes:
	====================================

	0,			// no error encountered during processing
	-1,			// processing is done
	-2,			// processing was cancelled
	-3,			// file structure is bad (wrong file type or format unsupported
	-4,			// error during read()
	-5,			// error during write()
	-6,			// internal error
	-7,			// Sample rate needs to be 44.1kHz
	-8,			// Only 16bit files are supported
	-9,			// Only mono files are supported
	-10,		// No data to process
	-108,		// Out of memory



*/


#ifdef __cplusplus
extern "C" {
#endif
	

//	-----------------------------------------------------------------------------------------
//	Returns a pointer to a string containing the current library version
//
const char *mAiffGetVersion(void);
//	-----------------------------------------------------------------------------------------
	
	
//	-----------------------------------------------------------------------------------------
//	Allocates an empty audio buffer to hold numChannels channels and numFrames frames
//	float audioBuffer[numChannels][numFrames]
//
float **mAiffAllocateAudioBuffer(int numChannels, int numFrames);
//	-----------------------------------------------------------------------------------------
	
	
//	-----------------------------------------------------------------------------------------
//	Destroys an audio buffer
//
void mAiffDeallocateAudioBuffer(float **audio, int numChannels);
//	-----------------------------------------------------------------------------------------
	

//	-----------------------------------------------------------------------------------------
//	Reads data from the position "startFrame" in the AIFF file specified in *filename,
//	either for numChannels channels or a single channel, 
//	converts it to the range [-1.0, +1.0) and returns it as float in data[0...numChannels][0...numFrames-1].
//	If an error occurs, mAiffReadData() returns a negative error code. If all
//	goes well, mAiffReadData() returns the number of frames actually read. If it returns
//	with 0, the end of the file has been reached or exceeded by your request.
//	If there are fewer channels in the file than you have requested in numChannels, the unused channel
//	data is set to zero. data[][] needs its first dimension to be numChannels elements
//
int mAiffReadData(char *filename, float **data, unsigned long startFrame, long numFrames, long numChannels);
int mAiffReadDataFromChannel(char *filename, float *data, unsigned long startFrame, long numFrames, long channel);
//	-----------------------------------------------------------------------------------------


//	-----------------------------------------------------------------------------------------
//	Opens and clears a new file for writing. If you do not call this function prior to writing
//	to an audio file with mAiffWriteData(), the data you write will be appended to the file.
//	If the file does not exist and you do not call mAiffInitFile() prior to calling mAiffWriteData()
//	writing to the file will fail.
//
int mAiffInitFile(char *filename, float sampleRate, long sampleSize, long numChannels);
//	-----------------------------------------------------------------------------------------


//	-----------------------------------------------------------------------------------------
//	Appends the data in data[0...numFrames-1] to the audio file specified with *filename.
//	If it completes successfully, it returns the actual number of frames appended to the file.
// 	If it fails, it returns an error code.
//
int mAiffWriteData(char *filename, float **data, long numFrames, long numChannels);
//	-----------------------------------------------------------------------------------------



//	-----------------------------------------------------------------------------------------
//	Returns the number of sample frames contained in the specified file.
// 	If it fails, it returns an error code.
//
int mAiffGetNumberOfFrames(char *filename);
//	-----------------------------------------------------------------------------------------


//	-----------------------------------------------------------------------------------------
//	Returns the number of audio channels contained in the specified file.
// 	If it fails, it returns an error code.
//
int mAiffGetNumberOfChannels(char *filename);
//	-----------------------------------------------------------------------------------------


//	-----------------------------------------------------------------------------------------
//	Returns the wordlength of the audio data contained in the specified file. Possible
//	wordlengths are 8bit, 16bit, 24bit and 32bit.
// 	If it fails, it returns an error code.
//
int mAiffGetWordlength(char *filename);
//	-----------------------------------------------------------------------------------------


//	-----------------------------------------------------------------------------------------
//	Returns the sample rate of the audio data contained in the specified file. Although
//	MiniAiff reads all sample rates, the files written will all be 44.1kHz at the moment.
//	If your original sound file wasn't recorded at 44.1kHz, you will experience a change
//	in pitch of your audio material.
// 	If it fails, it returns an error code.
//
int mAiffGetSampleRate(char *filename);
//	-----------------------------------------------------------------------------------------


//	-----------------------------------------------------------------------------------------
//	Writes a chunk of data to an AIFF file's application specific chunk (APPL). No byte
//	reversing is carried out - this is rather experimental and should only be used if you know
//	what you're doing. It is great for saving settings or other stuff to a file.
//	An existing file with the same name will be overwritten.
//	Only one chunk of appspecific data per file is supported - each call to this function
//	initializes the file.
// 	If it fails, it returns an error code.
//
int mAiffWriteAppData(char *filename, void *data, long numBytes);
//	-----------------------------------------------------------------------------------------


//	-----------------------------------------------------------------------------------------
//	Reads a chunk of data from an AIFF file's application specific chunk (APPL). No byte
//	reversing is carried out - this is rather experimental and should only be used if you know
//	what you're doing. It is great for loading settings or other stuff from a file.
// 	If it fails, it returns an error code.
//
int mAiffReadAppData(char *filename, void *data, unsigned long start, long numBytes);
//	-----------------------------------------------------------------------------------------


//	-----------------------------------------------------------------------------------------
//	Return the specified information from an AIFF files' instrument chunk. If an error
//	occurs, an error code (s.a.) is returned. If no instrument chunk is present, the routines
//	return 0
//
int mAiffGetLowKey(char *filename);
int mAiffGetHighKey(char *filename);
int mAiffGetRootKey(char *filename);
int mAiffGetGain(char *filename);
int mAiffGetDetune(char *filename);
//	-----------------------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


