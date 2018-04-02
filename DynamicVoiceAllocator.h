/**
 * @file dynamic_voice_allocator.h
 * @brief MIDI発音管理
 * @author osoumen
 * @date 2014/11/30
 */

#ifndef __DynamicVoiceAllocator__
#define __DynamicVoiceAllocator__

class DynamicVoiceAllocator
{
public:
	enum VoiceAllocMode {
		ALLOC_MODE_OLDEST = 0,
		ALLOC_MODE_SAMECH
	};
	DynamicVoiceAllocator();
	~DynamicVoiceAllocator();
	
	void    Initialize(int voiceLimit);
	void    Reset();
	void    ChangeVoiceLimit(int voiceLimit);
	void    SetVoiceAllocMode(VoiceAllocMode mode);
	int     GetVoiceLimit() { return mVoiceLimit; }
	// forceVo=VOICE_UNSETでないときは固定のボイスを確保する
	int     AllocVoice(int prio, int ch, int uniqueID, int forceVo,
					   int *releasedCh, bool *isLegato);
	int     ReleaseVoice(int relPrio, int ch, int uniqueID, int *relVo);
	bool    ReleaseAllVoices(int ch);
	bool    IsPlayingVoice(int v);
	void    SetChLimit(int ch, int value);
	int     GetChLimit(int ch);
	int     GetNoteOns(int ch);
	int     GetVoiceMidiCh(int vo) { return mVoiceList.getVoiceMidiCh(vo); }
	int     GetVoiceUniqueID(int vo) { return mVoiceList.getVoiceUniqueId(vo); }
	void    SetKeyOn(int vo);
	bool    IsKeyOn(int vo);
	
private:
	static const int MAX_VOICE = 16;
	static const int VOICE_UNSET = -1;
	static const int CHANNEL_UNSET = -1;
	
	typedef struct {
		int		midiCh;
		int		priority;
		int		uniqueId;
		unsigned int timestamp;
		bool	isKeyOn;	// 確保された直後の未発音状態を表すため
		bool	isAlloced;
	} VoiceSlotStatus;
	
	class VoiceStatusList {
	public:
		VoiceStatusList(int numSlots);
		~VoiceStatusList();
		
		void reset();
		void allocVoice(int slot, int midiCh, int prio, int uniqueId);
		void removeVoice(int slot);
		void changeVoiceLimit(int voiceLimit);
		void setKeyOn(int slot);
		bool isKeyOn(int slot);
		bool isAlloced(int slot);
		void changeState(int slot, int prio, int uniqueId, bool isKeyOn);
		int getVoiceMidiCh(int slot);
		int getVoiceUniqueId(int slot);
		int	findFreeVoice(int priorCh=CHANNEL_UNSET);
		int	findWeakestVoiceInMidiCh(int midiCh);
		int	findWeakestVoice();
	private:
		VoiceStatusList();
		int				mNumSlots;
		int				mVoiceLimit;
		VoiceSlotStatus	*mSlots;
		unsigned int	mTimeStamp;
	};
	
	VoiceStatusList	mVoiceList;
	int mVoiceLimit;
	int mChNoteOns[16];
	int mChLimit[16];
	VoiceAllocMode mAllocMode;
};

#endif /* defined(__DynamicVoiceAllocator__) */
