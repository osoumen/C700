//
//  DynamicVoiceManager.cpp
//  C700
//
//  Created by osoumen on 2014/11/30.
//
//

#include "DynamicVoiceManager.h"

//-----------------------------------------------------------------------------
DynamicVoiceManager::DynamicVoiceManager() :
mVoiceLimit(8)
{
    
}

//-----------------------------------------------------------------------------
DynamicVoiceManager::~DynamicVoiceManager()
{
    
}

//-----------------------------------------------------------------------------
void DynamicVoiceManager::Initialize(int voiceLimit)
{
    mVoiceLimit = voiceLimit;
    for (int i=0; i<16; i++) {
        mChNoteOns[i] = 0;
        mChLimit[i] = 127;
    }
}

//-----------------------------------------------------------------------------
void DynamicVoiceManager::Reset()
{
    mPlayVo.clear();
	mWaitVo.clear();
    for(int i=0;i<mVoiceLimit;i++){
        pushWaitVo(i);
	}
    for (int i=0; i<MAX_VOICE; i++) {
        mVoCh[i] = 0;
        mVoPrio[i] = 64;
        mVoUniqueID[i] = 0;
        mVoKeyOn[i] = false;
    }
}

//-----------------------------------------------------------------------------
void DynamicVoiceManager::ChangeVoiceLimit(int voiceLimit)
{
    if ( voiceLimit < mVoiceLimit ) {
		//空きボイスリストから削除する
		for ( int i=voiceLimit; i<mVoiceLimit; i++ ) {
			mWaitVo.remove(i);
		}
	}
	if ( voiceLimit > mVoiceLimit ) {
		//空きボイスを追加する
		for ( int i=mVoiceLimit; i<voiceLimit; i++ ) {
			pushWaitVo(i);
		}
	}
	mVoiceLimit = voiceLimit;
}

//-----------------------------------------------------------------------------
int DynamicVoiceManager::AllocVoice(int prio, int ch, int uniqueID, bool monoMode,
                                    int *releasedCh, bool *isLegato)
{
    int v = -1;
    *releasedCh = -1;
    *isLegato = false;
    
    if (monoMode) {
        v = ch & 0x07;     // 固定のchを確保
        if (IsPlayingVoice(v)) {
            if (mVoCh[v] == ch) {
                // レガートで鳴らした音
                // キーオン前に２回叩かれた場合は最後のノートオンだけが有効になるように
                if (mVoKeyOn[v]) {
                    *isLegato = true;
                }
            }
            else {
                // 別のchの発音がすでにある場合
                mPlayVo.remove(v);
                *releasedCh = mVoCh[v];
                mChNoteOns[mVoCh[v]]--;
            }
        }
        else {
            mWaitVo.remove(v);
        }
    }
    else {
        if (mChNoteOns[ch] >= mChLimit[ch]) {
            // ch発音数を超えてたら、そのchの音を一つ止めて次の音を鳴らす
            v = StealVoice(ch);
            if (v != -1) {
                mPlayVo.remove(v);
                *releasedCh = mVoCh[v];
                mChNoteOns[mVoCh[v]]--;
            }
        }
        else {
            // 超えてない場合は、後着優先で優先度の低い音を消す
            v = FindVoice();
            if (v >= MAX_VOICE) {  //空きがなくてどこかを止めた
                v -= MAX_VOICE;
                mPlayVo.remove(v);
                *releasedCh = mVoCh[v];
                mChNoteOns[mVoCh[v]]--;
            }
            else if (v >= 0) {
                mWaitVo.remove(v);
            }
        }
    }
    
    if (v != -1) {
        mVoCh[v] = ch;
        mVoPrio[v] = prio;
        mVoKeyOn[v] = false;
        mVoUniqueID[v] = uniqueID;
        if (*isLegato == false) {
            mPlayVo.push_back(v);
            mChNoteOns[ch]++;
        }
    }
    return v;
}

//-----------------------------------------------------------------------------
int DynamicVoiceManager::ReleaseVoice(int relPrio, int ch, int uniqueID, int *relVo)
{
    int stops = 0;
    std::list<int>::iterator	it = mPlayVo.begin();
    while (it != mPlayVo.end()) {
        int	vo = *it;
        
        if ( mVoUniqueID[vo] == uniqueID ) {
            if (mVoKeyOn[vo]) {
                mVoUniqueID[vo] = 0;
                mVoPrio[vo] = relPrio;
                mVoKeyOn[vo] = false;
            }
            if ( vo < mVoiceLimit ) {
                mWaitVo.push_back(vo);
            }
            it = mPlayVo.erase(it);
            stops++;
            mChNoteOns[ch]--;
            *relVo = vo;
            break;  // ２重鳴りはホストに任せる
            //continue;
        }
        it++;
    }
    
	return stops;
}

//-----------------------------------------------------------------------------
void DynamicVoiceManager::SetChLimit(int ch, int value)
{
    mChLimit[ch & 0xf] = value;
}

//-----------------------------------------------------------------------------
int DynamicVoiceManager::GetChLimit(int ch)
{
    return mChLimit[ch & 0xf];
}

//-----------------------------------------------------------------------------
int DynamicVoiceManager::GetNoteOns(int ch)
{
    return mChNoteOns[ch & 0xf];
}

//-----------------------------------------------------------------------------
void DynamicVoiceManager::SetKeyOn(int vo)
{
    // TODO: voがAlloc済みかどうかチェック
    mVoKeyOn[vo] = true;
}

//-----------------------------------------------------------------------------
bool DynamicVoiceManager::IsKeyOn(int vo)
{
    return mVoKeyOn[vo];
}

//-----------------------------------------------------------------------------
void DynamicVoiceManager::pushWaitVo(int vo)
{
    mWaitVo.push_back(vo);
    mVoCh[vo] = 0;
    mVoPrio[vo] = 0;
}

//-----------------------------------------------------------------------------
int DynamicVoiceManager::FindFreeVoice()
{
	int	v=-1;
    
	//空きボイスを探す
    if ( mWaitVo.size() > 0 ) {
		v = mWaitVo.front();
		mWaitVo.pop_front();
	}
    return v;
}

//-----------------------------------------------------------------------------
bool DynamicVoiceManager::IsPlayingVoice(int v)
{
    std::list<int>::iterator	it = mPlayVo.begin();
    while (it != mPlayVo.end()) {
        int	vo = *it;
        if (vo == v) {
            return true;
        }
        it++;
    }
    return false;
}
//-----------------------------------------------------------------------------
int DynamicVoiceManager::StealVoice(int ch)
{
    int v=-1;
    int prio_min = 0x7fff;
    
    std::list<int>::reverse_iterator  it = mPlayVo.rbegin();
    while (it != mPlayVo.rend()) {
        int vo = *it;
        if ( (mVoPrio[vo] <= prio_min) && (mVoCh[vo] == ch) ) {
            prio_min = mVoPrio[vo];
            v = vo;
        }
        it++;
    }
    
    return v;
}

//-----------------------------------------------------------------------------
int DynamicVoiceManager::FindVoice(int ch)
{
    int v=-1;
    int prio_min = 0x7fff;
    
    std::list<int>::reverse_iterator  it = mPlayVo.rbegin();
    while (it != mPlayVo.rend()) {
        int vo = *it;
        bool    chMatch = (mVoCh[vo] == ch) ? true:false;
        if (ch == -1) {
            chMatch = true;
        }
        if ( (mVoPrio[vo] <= prio_min) && chMatch ) {
            prio_min = mVoPrio[vo];
            v = vo + MAX_VOICE;
        }
        it++;
    }
    it = mWaitVo.rbegin();
    while (it != mWaitVo.rend()) {
        int vo = *it;
        if (mVoPrio[vo] <= prio_min) {
            prio_min = mVoPrio[vo];
            v = vo;
        }
        it++;
    }
    
    return v;
}
