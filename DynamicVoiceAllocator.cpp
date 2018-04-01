//
//  DynamicVoiceAllocator.cpp
//  C700
//
//  Created by osoumen on 2014/11/30.
//
//

#include "DynamicVoiceAllocator.h"


//-----------------------------------------------------------------------------
DynamicVoiceAllocator::VoiceStatusList::VoiceStatusList(int numSlots)
{
	mSlots = new VoiceSlotStatus[numSlots];
	mNumSlots = mVoiceLimit = numSlots;
	reset();
}

//-----------------------------------------------------------------------------
DynamicVoiceAllocator::VoiceStatusList::~VoiceStatusList()
{
	delete [] mSlots;
}

//-----------------------------------------------------------------------------
void DynamicVoiceAllocator::VoiceStatusList::reset()
{
	for (int i=0; i<mNumSlots; ++i) {
		mSlots[i].isAlloced = false;
		mSlots[i].isKeyOn = false;
		mSlots[i].midiCh = 0;
		mSlots[i].priority = 0;
		mSlots[i].uniqueId = 0;
		mSlots[i].timestamp = 0;
	}
	mTimeStamp = 0;
}

//-----------------------------------------------------------------------------
void DynamicVoiceAllocator::VoiceStatusList::allocVoice(int slot, int midiCh, int prio, int uniqueId)
{
	//assert(slot < mNumSlots);
	mSlots[slot].isAlloced = true;
	mSlots[slot].midiCh = midiCh;
	mSlots[slot].priority = prio;
	mSlots[slot].isKeyOn = false;
	mSlots[slot].uniqueId = uniqueId;
	mSlots[slot].timestamp = mTimeStamp;
	mTimeStamp++;
}

//-----------------------------------------------------------------------------
void DynamicVoiceAllocator::VoiceStatusList::removeVoice(int slot)
{
	//assert(slot < mNumSlots);
	mSlots[slot].isAlloced = false;
}

//-----------------------------------------------------------------------------
void DynamicVoiceAllocator::VoiceStatusList::changeVoiceLimit(int voiceLimit)
{
	mVoiceLimit = voiceLimit;
	if (mVoiceLimit > mNumSlots) {
		mVoiceLimit = mNumSlots;
	}
}

//-----------------------------------------------------------------------------
void DynamicVoiceAllocator::VoiceStatusList::setKeyOn(int slot)
{
	//assert(slot < mNumSlots);
	if (mSlots[slot].isAlloced) {
		mSlots[slot].isKeyOn = true;
	}
}

//-----------------------------------------------------------------------------
bool DynamicVoiceAllocator::VoiceStatusList::isKeyOn(int slot)
{
	//assert(slot < mNumSlots);
	return mSlots[slot].isKeyOn;
}

//-----------------------------------------------------------------------------
bool DynamicVoiceAllocator::VoiceStatusList::isAlloced(int slot)
{
	//assert(slot < mNumSlots);
	return mSlots[slot].isAlloced;
}

//-----------------------------------------------------------------------------
void DynamicVoiceAllocator::VoiceStatusList::changeState(int slot, int prio, int uniqueId, bool isKeyOn)
{
	mSlots[slot].priority = prio;
	mSlots[slot].uniqueId = uniqueId;
	mSlots[slot].isKeyOn = isKeyOn;
}

//-----------------------------------------------------------------------------
int DynamicVoiceAllocator::VoiceStatusList::getVoiceMidiCh(int slot)
{
	//assert(slot < mNumSlots);
	return mSlots[slot].midiCh;
}

//-----------------------------------------------------------------------------
int DynamicVoiceAllocator::VoiceStatusList::getVoiceUniqueId(int slot)
{
	//assert(slot < mNumSlots);
	return mSlots[slot].uniqueId;
}

//-----------------------------------------------------------------------------
int	DynamicVoiceAllocator::VoiceStatusList::findFreeVoice(int priorCh)
{
	int time_stamp_max = -1;
	int v = -1;

	if (priorCh == -1) {
		for (int i=0; i<mVoiceLimit; ++i) {
			int time_stamp = mTimeStamp - mSlots[i].timestamp;
			if ((time_stamp > time_stamp_max) &&
				(mSlots[i].isAlloced == false)) {
				time_stamp_max = time_stamp;
				v = i;
			}
		}
	}
	else {
		for (int i=0; i<mVoiceLimit; ++i) {
			if (mSlots[i].midiCh == priorCh) {
				int time_stamp = mTimeStamp - mSlots[i].timestamp;
				if ((time_stamp > time_stamp_max) &&
					(mSlots[i].isAlloced == false)) {
					time_stamp_max = time_stamp;
					v = i;
				}
			}
		}
	}
	return v;
}

//-----------------------------------------------------------------------------
int	DynamicVoiceAllocator::VoiceStatusList::findWeakestVoiceInMidiCh(int midiCh)
{
	// midiChで発音している確保済みのボイスのうち優先度が最小でタイムスタンプが最古のものを探す
	// 同じmidiChのボイスが見つからなければ-1を返す
	
	int time_stamp_max = -1;
	int v = -1;
	int prio_min = 0x7fff;

	for (int i=0; i<mVoiceLimit; ++i) {
		if ((mSlots[i].midiCh == midiCh) &&
			(mSlots[i].isAlloced == true) &&
			(mSlots[i].priority <= prio_min)) {
			int time_stamp = mTimeStamp - mSlots[i].timestamp;
			if (time_stamp > time_stamp_max) {
				v = i;
				time_stamp_max = time_stamp;
				prio_min = mSlots[i].priority;
			}
		}
	}
	return v;
}

//-----------------------------------------------------------------------------
int	DynamicVoiceAllocator::VoiceStatusList::findWeakestVoice()
{
	int v=-1;
	
	// 確保済みの全ボイスのうち優先度が最小で一番古いものを探す
	int time_stamp_max = -1;
	int prio_min = 0x7fff;
	
	for (int i=0; i<mVoiceLimit; ++i) {
		if ((mSlots[i].isAlloced == true) &&
			(mSlots[i].priority <= prio_min)) {
			int time_stamp = mTimeStamp - mSlots[i].timestamp;
			if (time_stamp > time_stamp_max) {
				v = i;
				time_stamp_max = time_stamp;
				prio_min = mSlots[i].priority;
			}
		}
	}
	return v;
}

//-----------------------------------------------------------------------------
DynamicVoiceAllocator::DynamicVoiceAllocator() :
mVoiceList(MAX_VOICE),
mVoiceLimit(8),
mAllocMode(ALLOC_MODE_OLDEST)
{
    
}

//-----------------------------------------------------------------------------
DynamicVoiceAllocator::~DynamicVoiceAllocator()
{
    
}

//-----------------------------------------------------------------------------
void DynamicVoiceAllocator::Initialize(int voiceLimit)
{
    mVoiceLimit = voiceLimit;
	mVoiceList.changeVoiceLimit(voiceLimit);
    for (int i=0; i<16; i++) {
        mChNoteOns[i] = 0;
        mChLimit[i] = 127;
    }
}

//-----------------------------------------------------------------------------
void DynamicVoiceAllocator::Reset()
{
    for (int i=0; i<16; i++) {
        mChNoteOns[i] = 0;
    }
	mVoiceList.reset();
}

//-----------------------------------------------------------------------------
void DynamicVoiceAllocator::ChangeVoiceLimit(int voiceLimit)
{
    if ( voiceLimit < mVoiceLimit ) {
		//空きボイスリストから削除する
		for ( int i=voiceLimit; i<mVoiceLimit; i++ ) {
			mVoiceList.removeVoice(i);
		}
	}
	mVoiceLimit = voiceLimit;
	mVoiceList.changeVoiceLimit(voiceLimit);
}

//-----------------------------------------------------------------------------
void DynamicVoiceAllocator::SetVoiceAllocMode(VoiceAllocMode mode)
{
    mAllocMode = mode;
}

//-----------------------------------------------------------------------------
int DynamicVoiceAllocator::AllocVoice(int prio, int ch, int uniqueID, int forceVo,
                                    int *releasedCh, bool *isLegato)
{
    int v = -1;
    *releasedCh = -1;
    *isLegato = false;
    
    if (forceVo != -1) {
        v = forceVo;     // 固定のchを確保
        if (mVoiceList.isAlloced(v)) {
            if (mVoiceList.getVoiceMidiCh(v) == ch) {
                // レガートで鳴らした音
                // キーオン前に２回叩かれた場合は最後のノートオンだけが有効になるように
                if (mVoiceList.isKeyOn(v)) {
                    *isLegato = true;
                }
            }
            else {
                // 別のchの発音がすでにある場合
				mVoiceList.removeVoice(v);
                *releasedCh = mVoiceList.getVoiceMidiCh(v);
                mChNoteOns[mVoiceList.getVoiceMidiCh(v)]--;
            }
        }
    }
    else {
        if (mChNoteOns[ch] >= mChLimit[ch]) {
            // ch発音数を超えてたら、そのchの音を一つ止めて次の音を鳴らす
            v = mVoiceList.findWeakestVoiceInMidiCh(ch);
            if (v != -1) {
                mVoiceList.removeVoice(v);
                *releasedCh = mVoiceList.getVoiceMidiCh(v);
                mChNoteOns[mVoiceList.getVoiceMidiCh(v)]--;
            }
        }
        else {
            // チャンネルリミットが未設定または超えてない場合は、全ボイスから後着優先で優先度の低い音を消す
			if (mAllocMode == ALLOC_MODE_SAMECH) {
				v = mVoiceList.findFreeVoice(ch);
				if (v == -1) {
					v = mVoiceList.findFreeVoice();
				}
			}
			else {
				v = mVoiceList.findFreeVoice();
			}
			
			if (v == -1) {
				if (mAllocMode == ALLOC_MODE_SAMECH) {
					v = mVoiceList.findWeakestVoiceInMidiCh(ch);
				}
				v = mVoiceList.findWeakestVoice();
				mVoiceList.removeVoice(v);
				*releasedCh = mVoiceList.getVoiceMidiCh(v);
				// 止めたslotの分減らす
				mChNoteOns[mVoiceList.getVoiceMidiCh(v)]--;
			}
        }
    }
    
    if (v != -1) {
        if (*isLegato == false && !mVoiceList.isAlloced(v)) { // モノモードの同時ノートオン対策
            mChNoteOns[ch]++;
        }
		mVoiceList.allocVoice(v, ch, prio, uniqueID);
    }
    return v;
}

//-----------------------------------------------------------------------------
int DynamicVoiceAllocator::ReleaseVoice(int relPrio, int ch, int uniqueID, int *relVo)
{
    int stops = 0;
	
	// uniqueIDが一致する発音のうちどれかを解放する
	// TODO: 一番古いものを探す
	for (int vo=0; vo<MAX_VOICE; ++vo) {
		if (mVoiceList.getVoiceUniqueId(vo) == uniqueID) {
			if (mVoiceList.isKeyOn(vo)) {
				mVoiceList.changeState(vo, relPrio, 0, false);
			}
			mVoiceList.removeVoice(vo);
			stops++;
			mChNoteOns[ch]--;
			*relVo = vo;
			break;  // ２重鳴りはホストに任せる
		}
	}
	return stops;
}

//-----------------------------------------------------------------------------
bool DynamicVoiceAllocator::ReleaseAllVoices(int ch)
{
    bool stoped = false;
	
	for (int vo=0; vo<MAX_VOICE; ++vo) {
		if (mVoiceList.getVoiceMidiCh(vo) == ch) {
			if (mVoiceList.isKeyOn(vo)) {
				mVoiceList.changeState(vo, 0, 0, false);
			}
			mVoiceList.removeVoice(vo);
			stoped = true;
			mChNoteOns[ch]--;
		}
	}
    return stoped;
}

//-----------------------------------------------------------------------------
void DynamicVoiceAllocator::SetChLimit(int ch, int value)
{
    mChLimit[ch & 0xf] = value;
}

//-----------------------------------------------------------------------------
int DynamicVoiceAllocator::GetChLimit(int ch)
{
    return mChLimit[ch & 0xf];
}

//-----------------------------------------------------------------------------
int DynamicVoiceAllocator::GetNoteOns(int ch)
{
    return mChNoteOns[ch & 0xf];
}

//-----------------------------------------------------------------------------
void DynamicVoiceAllocator::SetKeyOn(int vo)
{
	mVoiceList.setKeyOn(vo);
}

//-----------------------------------------------------------------------------
bool DynamicVoiceAllocator::IsKeyOn(int vo)
{
	return mVoiceList.isKeyOn(vo);
}

//-----------------------------------------------------------------------------
bool DynamicVoiceAllocator::IsPlayingVoice(int v)
{
	return mVoiceList.isAlloced(v);
}
