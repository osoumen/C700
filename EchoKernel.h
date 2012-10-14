/*
 *  EchoKernel.h
 *  Chip700
 *
 *  Created by osoumen on 12/10/12.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

class EchoKernel
{
public:
	EchoKernel();
	~EchoKernel();
	
	void 	Input(int samp);
	int		GetFxOut();
	void	Reset();
	void	SetEchoVol( int val )			{ m_echo_vol = val; }
	void	SetFBLevel( int val )			{ m_fb_lev = val; }
	void	SetFIRTap( int index, int val )	{ m_fir_taps[index] = val; }
	void	SetDelayTime( int val )			{ m_delay_samples = val * DELAY_UNIT; }
	
private:
	static const int DELAY_UNIT = 512;
	static const int ECHO_BUFFER_SIZE = 7680;
	static const int FIR_LENGTH = 8;
	static const int FILTER_STLIDE = 1;
	
	int			*mEchoBuffer;
	int			*mFIRbuf;
	int			mEchoIndex;
	int			mFIRIndex;
	
	int			m_echo_vol;
	int			m_fb_lev;
	int			m_fir_taps[8];
	int			m_delay_samples;
	
	int			m_input;
};