/*
 * This source code is public domain.
 *
 * Authors: Olivier Lapicque <olivierl@jps.net>
*/

#include "stdafx.h"
#include "TrackDecoder.h"

#ifdef MSC_VER
#pragma warning(disable:4244)
#endif

// Tables defined in tables.cpp
extern BYTE ImpulseTrackerPortaVolCmd[16];
extern WORD S3MFineTuneTable[16];
extern WORD ProTrackerPeriodTable[6 * 12];
extern WORD ProTrackerTunedPeriods[15 * 12];
extern WORD FreqS3MTable[];
extern WORD XMPeriodTable[96 + 8];
extern UINT XMLinearTable[768];
extern DWORD FineLinearSlideUpTable[16];
extern DWORD FineLinearSlideDownTable[16];
extern DWORD LinearSlideUpTable[256];
extern DWORD LinearSlideDownTable[256];
extern signed char retrigTable1[16];
extern signed char retrigTable2[16];
extern short int ModRandomTable[64];


UINT CDecompressTrack::SeekInPattern(UINT Pattern, UINT Row, UINT MusicSpeed,
	UINT MusicTempo, UINT Position)
{
	UINT CurRow;
	//   UINT NewRow;
	UINT Samples = 0;
	UINT patloop[MAX_CHANNELS];
	UINT SpeedCount;

	int BreakFlag = 0;

	// ��������� Rows
	for (CurRow = Row; CurRow < PatternSize[Pattern]; CurRow++) {
		BreakFlag = 0;
		//  	NewRow  	= 0;

		// ���� ������ ������� � �������, �������� ����� ������ ������� ��������
		if (!CurRow) {
			for (UINT ipck = 0; ipck < m_nChannels; ipck++)
				patloop[ipck] = Samples;
		}

		// ���������� ��������� �� ������ ��������
		MODCHANNEL* pChn = Chn;
		MODCOMMAND* p = Patterns[Pattern] + CurRow* m_nChannels;
		for (UINT nChn = 0; nChn < m_nChannels; p++,pChn++, nChn++) {
			if (*((DWORD *) p) && p->command) {
				UINT param = p->param;
				UINT note = p->note;

				switch (p->command) {
					// Position Jump
				case CMD_POSITIONJUMP:
					/*
												   // �������� �� ������������
											   if( param <= Pattern && OrderInfo[param]._init != 0 )
												  BreakFlag   = 2;
											   else
												  BreakFlag   = 1;
												   break;*/

					// Pattern Break
				case CMD_PATTERNBREAK:
					BreakFlag = 1;
					break;

					// Set Speed
				case CMD_SPEED:
					if (!param)
						break;
					if ((param <= 0x20) || (m_nType != MOD_TYPE_MOD)) {
						if (param < 128)
							MusicSpeed = param;
					}
					break;
					// Set Tempo
				case CMD_TEMPO:
					if (param >= 0x20)
						MusicTempo = param;
					else {
						// Tempo Slide
						if ((param & 0xF0) == 0x10) {
							MusicTempo += param & 0x0F;
							if (MusicTempo > 255)
								MusicTempo = 255;
						} else {
							MusicTempo -= param & 0x0F;
							if (MusicTempo < 32)
								MusicTempo = 32;
						}
					}
					break;
					// Pattern Delay
				case CMD_S3MCMDEX:
					if ((param & 0xF0) == 0x60) {
						SpeedCount = param & 0x0F;
						break;
					} else {
						if ((param & 0xF0) == 0xB0) {
							param &= 0x0F; param |= 0x60;
						}
					}
				case CMD_MODCMDEX:
					if ((param & 0xF0) == 0xE0)
						SpeedCount = (param & 0x0F) * MusicSpeed;
					else {
						if ((param & 0xF0) == 0x60) {
							if (param & 0x0F)
								Samples += (Samples - patloop[nChn]) * (param & 0x0F);
							else
								patloop[nChn] = Samples;
						}
					}
					break;
				}
			}
		}

		int len = ((gdwMixingFreq * 5 * m_nTempoFactor) / (MusicTempo << 8)) * MusicSpeed;
		if (Position >= Samples && Position < (Samples + len))
			return CurRow;

		Samples += len;
		// ������ �����������������
		SpeedCount += MusicSpeed;
		//			  dwElapsedTime  += (2500 * SpeedCount) / MusicTempo;
		//  	Position	   -= len;
		//  	SamplesInFile  += SampleOf;

		// �������� �� ������� �� ��������� �������
		if (BreakFlag != 0)
			break;
	}
	return -1;
}

////////////////////////////////////////////////////////////
// �������� �������������� ������
// �� �����    :  *
// �� ������   :  *
////////////////////////////////////////////////////////////
void CDecompressTrack::CreateOrderInfo(void)
{
	enum { TRACK_BREAK_NONE = 0, TRACK_BREAK_END, TRACK_BREAK_NEXT };

	SamplesInFile = 0;
	MaxSamplesInPattern = 0;

	UINT CurOrder = 0;
	UINT CurPattern = 0;
	UINT CurRow = 0;
	UINT NewOrder = 0;
	UINT NewRow = 0;

	//	UINT  dwElapsedTime  = 0;
	DWORD SampleOf = 0;
	UINT Len = 0;

	UINT MusicSpeed = m_nDefaultSpeed;
	UINT MusicTempo = m_nDefaultTempo;
	UINT SpeedCount = 0;
	int BreakFlag = TRACK_BREAK_NONE;

	DWORD patloop[MAX_CHANNELS];

	memset(&OrderInfo, 0, sizeof(ORDERINFO) * MAX_ORDERS);

	memset(patloop, 0, sizeof(patloop));

	CurOrder = 0;
	CurPattern = 0;
	CurRow = 0;

	NewOrder = 0;
	NewRow = 0;

	// ��������� ������ ��������
	while (CurOrder < MAX_ORDERS && Order[CurOrder] != 0xFF) {
		SpeedCount = 0;

		// ��������� ������ �������� ������� �� ������ ���������������
		CurPattern = Order[CurOrder];
		NewOrder = CurOrder + 1;
		SampleOf = 0;

		// �������� ������ �������
		if (CurPattern < MAX_PATTERNS) {
			// �������� ������� �������
			if (!Patterns[CurPattern])
				break;

			// ��������� Rows
			for (CurRow = NewRow;
				CurRow < PatternSize[CurPattern];
				CurRow++) {
				BreakFlag = TRACK_BREAK_NONE;
				NewRow = 0;

				// ���� ������ ������� � �������, �������� ����� ������ ������� ��������
				if (!CurRow) {
					for (UINT ipck = 0; ipck < m_nChannels; ipck++)
						patloop[ipck] = SamplesInFile;
				}

				// ���������� ��������� �� ������ ��������
				MODCHANNEL* pChn = Chn;
				MODCOMMAND* p = Patterns[CurPattern] + CurRow* m_nChannels;
				for (UINT nChn = 0; nChn < m_nChannels; p++,pChn++, nChn++) {
					if (*((DWORD *) p) && p->command) {
						UINT param = p->param;
						UINT note = p->note;

						switch (p->command) {
							// Position Jump
						case CMD_POSITIONJUMP:
							NewOrder = param;
							NewRow = 0;

							// �������� �� ������������
							if (param <= CurOrder &&
								OrderInfo[param]._init != 0)
								BreakFlag = TRACK_BREAK_END;
							else
								BreakFlag = TRACK_BREAK_NEXT;
							break;

							// Pattern Break
						case CMD_PATTERNBREAK:
							NewOrder = CurOrder + 1;
							NewRow = param;
							BreakFlag = TRACK_BREAK_NEXT;
							break;

							// Set Speed
						case CMD_SPEED:
							if (!param)
								break;
							if ((param <= 0x20) || (m_nType != MOD_TYPE_MOD)) {
								if (param < 128)
									MusicSpeed = param;
							}
							break;
							// Set Tempo
						case CMD_TEMPO:
							if (param >= 0x20)
								MusicTempo = param;
							else {
								// Tempo Slide
								if ((param & 0xF0) == 0x10) {
									MusicTempo += param & 0x0F;
									if (MusicTempo > 255)
										MusicTempo = 255;
								} else {
									MusicTempo -= param & 0x0F;
									if (MusicTempo < 32)
										MusicTempo = 32;
								}
							}
							break;
							// Pattern Delay
						case CMD_S3MCMDEX:
							if ((param & 0xF0) == 0x60) {
								SpeedCount = param & 0x0F;
								break;
							} else {
								if ((param & 0xF0) == 0xB0) {
									param &= 0x0F; param |= 0x60;
								}
							}
						case CMD_MODCMDEX:
							if ((param & 0xF0) == 0xE0)
								SpeedCount = (param & 0x0F) * MusicSpeed;
							else {
								if ((param & 0xF0) == 0x60) {
									if (param & 0x0F)
										SamplesInFile += (SamplesInFile -
											patloop[nChn]) * (param & 0x0F);
									else
										patloop[nChn] = SamplesInFile;
								}
							}
							break;
						}
					}
				}
				// ������ �����������������
				SpeedCount += MusicSpeed;
				//			  dwElapsedTime  += (2500 * SpeedCount) / MusicTempo;
				Len = ((gdwMixingFreq * 5 * m_nTempoFactor) /
					(MusicTempo << 8)) * MusicSpeed;
				SampleOf += Len;
				SamplesInFile += Len;

				// �������� �� ������� �� ��������� �������
				if (BreakFlag != TRACK_BREAK_NONE)
					break;
			}

			// ������� ���������� � ��������
			if (OrderInfo[CurOrder]._init == 0) {
				OrderInfo[CurOrder]._init = 1;  				  // ���� �������������
				OrderInfo[CurOrder]._next = NewOrder - CurOrder;  // �������� �� ��������� ������� � ������
				OrderInfo[CurOrder]._row = NewRow;  			 // ����� ��� ��� �������� �� ��������� �������
				OrderInfo[CurOrder]._samples = SampleOf;			 // ���������� ������� � ��������
				OrderInfo[CurOrder]._speed = MusicSpeed;		   // ��������
				OrderInfo[CurOrder]._tempo = MusicTempo;		   // ����

				// ����� ������������� ������� ������� � �������
				if (MaxSamplesInPattern < SampleOf)
					MaxSamplesInPattern = SampleOf;
			}

			// �����
			if (BreakFlag == TRACK_BREAK_END)
				break;
		}
		// ������� �� ��������� �������
		CurOrder = NewOrder;
	};
}
/*
////////////////////////////////////////////////////////////
// Length

DWORD CDecompressTrack::GetSamples(BOOL bAdjust, BOOL bTotal)
//----------------------------------------------------
{
	UINT dwElapsedTime=0, nRow=0, CurrentPattern=0, nNextPattern=0, nPattern=Order[0];
	UINT nMusicSpeed=m_nDefaultSpeed, nMusicTempo=m_nDefaultTempo, nNextRow=0;
	UINT nMaxRow = 0, nMaxPattern = 0;
	LONG nGlbVol = m_nDefaultGlobalVolume, nOldGlbVolSlide = 0;
	DWORD patloop[MAX_CHANNELS];

   memset( &OrderInfo, 0, sizeof(ORDERINFO) * MAX_ORDERS );

   DWORD SampleOf = 0;

	memset(patloop, 0, sizeof(patloop));

	nMaxRow 		  = m_nNextRow;
	nMaxPattern 	  = m_nNextPattern;
	CurrentPattern    = 0;
   nNextPattern 	 = 0;
	nPattern		  = Order[0];
	nRow			  = 0;
   nNextRow 		 = 0;

   // ��������� ������ ��������
	for (;;)
	{
		UINT nSpeedCount  = 0;
		nRow			  = nNextRow;
		CurrentPattern    = nNextPattern;

		// ��������� ������ �������� ������� �� ������ ���������������
		nPattern = Order[CurrentPattern];

	  // ������� ���������� � �������
	  if( OrderInfo[CurrentPattern]._init == 0 )
	  {
		 OrderInfo[CurrentPattern]._init		= 1;		   // ���� �������������
		 OrderInfo[CurrentPattern]._next		= 1;		   // �������� �� ��������� ������� � ������
		 OrderInfo[CurrentPattern]._row 		= 0;		   // ����� ��� ��� �������� �� ��������� �������
		 OrderInfo[CurrentPattern]._samplePos   = SampleOf;    // ������� 
		 OrderInfo[CurrentPattern]._speed   	= nMusicSpeed; // ��������
		 OrderInfo[CurrentPattern]._tempo   	= nMusicTempo; // ����
	  }

	  // ���� ����� ���������������� ������� ������ �����������
		while (nPattern >= MAX_PATTERNS)
		{
			// ��� ����� ������?
			if ((nPattern == 0xFF) || (CurrentPattern >= MAX_ORDERS))
			{
			// ����� ������
				goto EndMod;
			} else
			{
			// ������� ������ � ������� ����� ������ �� ������
				CurrentPattern++;
				nPattern = (CurrentPattern < MAX_ORDERS) ? Order[CurrentPattern] : 0xFF;
			}
			nNextPattern = CurrentPattern;
		}

		// ���� ����� ���������������� ������� ������ ����������� ��� ������ ������� ���
	  // ����� �� �����
		if ((nPattern >= MAX_PATTERNS) || (!Patterns[nPattern]))
		 break;

		// ���� ������� ������� ������ ��� ���������� ��������� � �������, ������� ����� 0
		if (nRow >= PatternSize[nPattern])
		 nRow = 0;

		// ���������� ���������� ��������
		nNextRow = nRow + 1;
	  // ���� ��������� ������� ������ ��� ���������� ��������� � �������,
	  // ��������� ������� ����� 0, ����� �������� �� ��������� ������� ������
		if (nNextRow >= PatternSize[nPattern])
		{

			nNextPattern = CurrentPattern + 1;
			nNextRow = 0;
		}
	  
	  // ���� ������ ������� � �������, �������� ����� ������ ������� ��������
		if (!nRow)
		{
			for (UINT ipck=0; ipck<m_nChannels; ipck++)
			patloop[ipck] = dwElapsedTime;
		}

	  // ���������� ��������� �� ������ ��������
		MODCHANNEL *pChn = Chn;
		MODCOMMAND *p = Patterns[nPattern] + nRow * m_nChannels;
		for (UINT nChn=0; nChn<m_nChannels; p++,pChn++, nChn++) 
	  {
		 if (*((DWORD *)p) && p->command )
		   {
			   UINT param     = p->param;
			   UINT note	  = p->note;

			switch (p->command)
			   {
			   // Position Jump
			   case CMD_POSITIONJUMP:
				   if( param <= CurrentPattern && OrderInfo[param]._init != 0 )
				  goto EndMod;   // ����� ���������� ������������ 
				   nNextPattern = param;
				   nNextRow = 0;
				   break;
			   // Pattern Break
			   case CMD_PATTERNBREAK:
				   nNextRow = param;
				   nNextPattern = CurrentPattern + 1;
				   break;
			   // Set Speed
			   case CMD_SPEED:
				   if (!param) break;
				   if ((param <= 0x20) || (m_nType != MOD_TYPE_MOD))
				   {
				  //!!! MS
					   if (param < 128) nMusicSpeed = param;
				   }
				   break;
			   // Set Tempo
			   case CMD_TEMPO:
				   if (param >= 0x20)
				  nMusicTempo = param;
			   else
			   {
					  // Tempo Slide
					  if ((param & 0xF0) == 0x10)
					  {
						  nMusicTempo += param & 0x0F;
						  if (nMusicTempo > 255) nMusicTempo = 255;
					  } else
					  {
						  nMusicTempo -= param & 0x0F;
						  if (nMusicTempo < 32) nMusicTempo = 32;
					  }
			   }
				   break;
			   // Pattern Delay
			   case CMD_S3MCMDEX:
				   if ((param & 0xF0) == 0x60)
			   {
				  nSpeedCount = param & 0x0F;
				  break;
			   } else
			   {
					  if ((param & 0xF0) == 0xB0)
				  {
					 param &= 0x0F; param |= 0x60;
				  }
			   }
			   case CMD_MODCMDEX:
				   if ((param & 0xF0) == 0xE0)
				  nSpeedCount = (param & 0x0F) * nMusicSpeed;
			   else
			   {
					  if ((param & 0xF0) == 0x60)
					  {
						  if (param & 0x0F)
						dwElapsedTime += (dwElapsedTime - patloop[nChn]) * (param & 0x0F);
						  else
						patloop[nChn] = dwElapsedTime;
					  }
			   }
				   break;
			   }
		   }
	  }
		nSpeedCount += nMusicSpeed;
		dwElapsedTime += (2500 * nSpeedCount) / nMusicTempo;

	  SampleOf += ((gdwMixingFreq * 5 * m_nTempoFactor) / (nMusicTempo << 8)) * nMusicSpeed;
	}
EndMod:
   
	return SampleOf;
}
*/

/*
DWORD CDecompressTrack::Seek(unsigned int Pos)
//----------------------------------------------------
{
	UINT dwElapsedTime=0, nRow=0, CurrentPattern=0, nNextPattern=0, nPattern=Order[0];
	UINT nMusicSpeed=m_nDefaultSpeed, nMusicTempo=m_nDefaultTempo, nNextRow=0;
	UINT nMaxRow = 0, nMaxPattern = 0;
	LONG nGlbVol = m_nDefaultGlobalVolume, nOldGlbVolSlide = 0;
	BYTE samples[MAX_CHANNELS];
	BYTE instr[MAX_CHANNELS];
	BYTE notes[MAX_CHANNELS];
	BYTE vols[MAX_CHANNELS];
	BYTE oldparam[MAX_CHANNELS];
	BYTE chnvols[MAX_CHANNELS];
	DWORD patloop[MAX_CHANNELS];

   DWORD SampleOf = 0;

	memset(instr, 0, sizeof(instr));
	memset(notes, 0, sizeof(notes));
	memset(vols, 0xFF, sizeof(vols));
	memset(patloop, 0, sizeof(patloop));
	memset(oldparam, 0, sizeof(oldparam));
	memset(chnvols, 64, sizeof(chnvols));
	memset(samples, 0, sizeof(samples));
	
   for (UINT icv=0; icv<m_nChannels; icv++)
	  chnvols[icv] = ChnSettings[icv].nVolume;

	nMaxRow 		  = m_nNextRow;
	nMaxPattern 	  = m_nNextPattern;
	CurrentPattern   = 0;
   nNextPattern 	 = 0;
	nPattern		  = Order[0];
	nRow			  = 0;
   nNextRow 		 = 0;

   // ��������� ������ ��������
	for (;;)
	{
		UINT nSpeedCount  = 0;
		nRow			  = nNextRow;
		CurrentPattern   = nNextPattern;

		// ��������� ������ �������� ������� �� ������ ���������������
		nPattern = Order[CurrentPattern];

	  // ���� ����� ���������������� ������� ������ �����������
		while (nPattern >= MAX_PATTERNS)
		{
			// ��� ����� ������?
			if ((nPattern == 0xFF) || (CurrentPattern >= MAX_ORDERS))
			{
			// ����� ������
				goto EndMod;
			} else
			{
			// ������� ������ � ������� ����� ������ �� ������
				CurrentPattern++;
				nPattern = (CurrentPattern < MAX_ORDERS) ? Order[CurrentPattern] : 0xFF;
			}
			nNextPattern = CurrentPattern;
		}

		// ���� ����� ���������������� ������� ������ ����������� ��� ������ ������� ���
	  // ����� �� �����
		if ((nPattern >= MAX_PATTERNS) || (!Patterns[nPattern]))
		 break;

		// ���� ������� ������� ������ ��� ���������� ��������� � �������, ������� ����� 0
		if (nRow >= PatternSize[nPattern])
		 nRow = 0;

		// ���������� ���������� ��������
		nNextRow = nRow + 1;
	  // ���� ��������� ������� ������ ��� ���������� ��������� � �������,
	  // ��������� ������� ����� 0, ����� �������� �� ��������� ������� ������
		if (nNextRow >= PatternSize[nPattern])
		{

			nNextPattern = CurrentPattern + 1;
			nNextRow = 0;
		}
	  
	  // ���� ������ ������� � �������, �������� ����� ������ ������� ��������
		if (!nRow)
		{
			for (UINT ipck=0; ipck<m_nChannels; ipck++)
			patloop[ipck] = dwElapsedTime;
		}

	  // ���������� ��������� �� ������ ��������
		MODCHANNEL *pChn = Chn;
		MODCOMMAND *p = Patterns[nPattern] + nRow * m_nChannels;
		for (UINT nChn=0; nChn<m_nChannels; p++,pChn++, nChn++) 
	  {
		 if (*((DWORD *)p))
		   {
			   UINT command   = p->command;
			   UINT param     = p->param;
			   UINT note	  = p->note;
			   if (p->instr) { instr[nChn] = p->instr; notes[nChn] = 0; vols[nChn] = 0xFF; }
			   if ((note) && (note <= 120)) notes[nChn] = note;
			   if (p->volcmd == VOLCMD_VOLUME)	{ vols[nChn] = p->vol; }
			   if (command) switch (command)
			   {
			   // Position Jump
			   case CMD_POSITIONJUMP:
				   if (param <= CurrentPattern)
				  goto EndMod;   // ����� ���������� ������������ 
				   nNextPattern = param;
				   nNextRow = 0;
				   break;
			   // Pattern Break
			   case CMD_PATTERNBREAK:
				   nNextRow = param;
				   nNextPattern = CurrentPattern + 1;
				   break;
			   // Set Speed
			   case CMD_SPEED:
				   if (!param) break;
				   if ((param <= 0x20) || (m_nType != MOD_TYPE_MOD))
				   {
				  //!!! MS
					   if (param < 128) nMusicSpeed = param;
				   }
				   break;
			   // Set Tempo
			   case CMD_TEMPO:
				   if (param >= 0x20)
				  nMusicTempo = param;
			   else
			   {
					  // Tempo Slide
					  if ((param & 0xF0) == 0x10)
					  {
						  nMusicTempo += param & 0x0F;
						  if (nMusicTempo > 255) nMusicTempo = 255;
					  } else
					  {
						  nMusicTempo -= param & 0x0F;
						  if (nMusicTempo < 32) nMusicTempo = 32;
					  }
			   }
				   break;
			   // Pattern Delay
			   case CMD_S3MCMDEX:
				   if ((param & 0xF0) == 0x60)
			   {
				  nSpeedCount = param & 0x0F;
				  break;
			   } else
			   {
					  if ((param & 0xF0) == 0xB0)
				  {
					 param &= 0x0F; param |= 0x60;
				  }
			   }
			   case CMD_MODCMDEX:
				   if ((param & 0xF0) == 0xE0)
				  nSpeedCount = (param & 0x0F) * nMusicSpeed;
			   else
			   {
					  if ((param & 0xF0) == 0x60)
					  {
						  if (param & 0x0F)
						dwElapsedTime += (dwElapsedTime - patloop[nChn]) * (param & 0x0F);
						  else
						patloop[nChn] = dwElapsedTime;
					  }
			   }
				   break;
			   }
		   }
	  }
		nSpeedCount += nMusicSpeed;
		dwElapsedTime += (2500 * nSpeedCount) / nMusicTempo;

	  SampleOf += ((gdwMixingFreq * 5 * m_nTempoFactor) / (nMusicTempo << 8)) * nMusicSpeed;
	  
	  // �����?
	  if( SampleOf >= Pos )
	  {

	  }
	}
EndMod:
   
	return 0;
}
*/
//////////////////////////////////////////////////////////////////////////////////////////////////
// Effects

void CDecompressTrack::InstrumentChange(MODCHANNEL* pChn, UINT instr,
	BOOL bPorta, BOOL bUpdVol, BOOL bResetEnv)
	//--------------------------------------------------------------------------------------------------------
{
	BOOL bInstrumentChanged = FALSE;

	if (instr >= MAX_INSTRUMENTS)
		return;
	INSTRUMENTHEADER* penv = Headers[instr];
	MODINSTRUMENT* psmp =& Ins[instr];
	UINT note = pChn->nNewNote;
	if ((penv) && (note) && (note <= 128)) {
		if (penv->NoteMap[note - 1] >= 0xFE)
			return;
		UINT n = penv->Keyboard[note - 1];
		psmp = ((n) && (n < MAX_SAMPLES)) ? &Ins[n] : NULL;
	} else if (m_nInstruments) {
		if (note >= 0xFE)
			return;
		psmp = NULL;
	}
	// Update Volume
	if (bUpdVol)
		pChn->nVolume = (psmp) ? psmp->nVolume : 0;
	// bInstrumentChanged is used for IT carry-on env option
	if (penv != pChn->pHeader) {
		bInstrumentChanged = TRUE;
		pChn->pHeader = penv;
	} else
	// Special XM hack
	if ((bPorta) &&
		(m_nType & (MOD_TYPE_XM | MOD_TYPE_MT2)) &&
		(penv) &&
		(pChn->pInstrument) &&
		(psmp != pChn->pInstrument)) {
		// FT2 doesn't change the sample in this case,
		// but still uses the sample info from the old one (bug?)
		return;
	}
	// Instrument adjust
	pChn->nNewIns = 0;
	if (psmp) {
		if (penv) {
			pChn->nInsVol = (psmp->nGlobalVol * penv->nGlobalVol) >> 6;
			if (penv->dwFlags & ENV_SETPANNING)
				pChn->nPan = penv->nPan;
			pChn->nNNA = penv->nNNA;
		} else {
			pChn->nInsVol = psmp->nGlobalVol;
		}
		if (psmp->uFlags & CHN_PANNING)
			pChn->nPan = psmp->nPan;
	}
	// Reset envelopes
	if (bResetEnv) {
		if ((!bPorta) ||
			(!(m_nType & MOD_TYPE_IT)) ||
			(m_dwSongFlags & SONG_ITCOMPATMODE) ||
			(!pChn->nLength) ||
			((pChn->dwFlags & CHN_NOTEFADE) && (!pChn->nFadeOutVol))) {
			pChn->dwFlags |= CHN_FASTVOLRAMP;
			if ((m_nType & MOD_TYPE_IT) &&
				(!bInstrumentChanged) &&
				(penv) &&
				(!(pChn->dwFlags & (CHN_KEYOFF | CHN_NOTEFADE)))) {
				if (!(penv->dwFlags & ENV_VOLCARRY))
					pChn->nVolEnvPosition = 0;
				if (!(penv->dwFlags & ENV_PANCARRY))
					pChn->nPanEnvPosition = 0;
				if (!(penv->dwFlags & ENV_PITCHCARRY))
					pChn->nPitchEnvPosition = 0;
			} else {
				pChn->nVolEnvPosition = 0;
				pChn->nPanEnvPosition = 0;
				pChn->nPitchEnvPosition = 0;
			}
			pChn->nAutoVibDepth = 0;
			pChn->nAutoVibPos = 0;
		} else if ((penv) && (!(penv->dwFlags & ENV_VOLUME))) {
			pChn->nVolEnvPosition = 0;
			pChn->nAutoVibDepth = 0;
			pChn->nAutoVibPos = 0;
		}
	}
	// Invalid sample ?
	if (!psmp) {
		pChn->pInstrument = NULL;
		pChn->nInsVol = 0;
		return;
	}
	// Tone-Portamento doesn't reset the pingpong direction flag
	if ((bPorta) && (psmp == pChn->pInstrument)) {
		if (m_nType & (MOD_TYPE_S3M | MOD_TYPE_IT))
			return;
		pChn->dwFlags &= ~ (CHN_KEYOFF | CHN_NOTEFADE);
		pChn->dwFlags = (pChn->dwFlags & (0xFFFFFF00 | CHN_PINGPONGFLAG)) |
			(psmp->uFlags);
	} else {
		pChn->dwFlags &= ~ (CHN_KEYOFF |
			CHN_NOTEFADE |
			CHN_VOLENV |
			CHN_PANENV |
			CHN_PITCHENV);
		pChn->dwFlags = (pChn->dwFlags & 0xFFFFFF00) | (psmp->uFlags);
		if (penv) {
			if (penv->dwFlags & ENV_VOLUME)
				pChn->dwFlags |= CHN_VOLENV;
			if (penv->dwFlags & ENV_PANNING)
				pChn->dwFlags |= CHN_PANENV;
			if (penv->dwFlags & ENV_PITCH)
				pChn->dwFlags |= CHN_PITCHENV;
			if ((penv->dwFlags & ENV_PITCH) && (penv->dwFlags & ENV_FILTER)) {
				if (!pChn->nCutOff)
					pChn->nCutOff = 0x7F;
			}
			if (penv->nIFC & 0x80)
				pChn->nCutOff = penv->nIFC & 0x7F;
			if (penv->nIFR & 0x80)
				pChn->nResonance = penv->nIFR & 0x7F;
		}
		pChn->nVolSwing = pChn->nPanSwing = 0;
	}
	pChn->pInstrument = psmp;
	pChn->nLength = psmp->nLength;
	pChn->nLoopStart = psmp->nLoopStart;
	pChn->nLoopEnd = psmp->nLoopEnd;
	pChn->nC4Speed = psmp->nC4Speed;
	pChn->pSample = psmp->pSample;
	pChn->nTranspose = psmp->RelativeTone;
	pChn->nFineTune = psmp->nFineTune;
	if (pChn->dwFlags & CHN_SUSTAINLOOP) {
		pChn->nLoopStart = psmp->nSustainStart;
		pChn->nLoopEnd = psmp->nSustainEnd;
		pChn->dwFlags |= CHN_LOOP;
		if (pChn->dwFlags & CHN_PINGPONGSUSTAIN)
			pChn->dwFlags |= CHN_PINGPONGLOOP;
	}
	if ((pChn->dwFlags & CHN_LOOP) && (pChn->nLoopEnd < pChn->nLength))
		pChn->nLength = pChn->nLoopEnd;
}


void CDecompressTrack::NoteChange(UINT nChn, int note, BOOL bPorta,
	BOOL bResetEnv)
	//---------------------------------------------------------------------------
{
	if (note < 1)
		return;
	MODCHANNEL * const pChn =& Chn[nChn];
	MODINSTRUMENT* pins = pChn->pInstrument;
	INSTRUMENTHEADER* penv = pChn->pHeader;
	if ((penv) && (note <= 0x80)) {
		UINT n = penv->Keyboard[note - 1];
		if ((n) && (n < MAX_SAMPLES))
			pins = &Ins[n];
		note = penv->NoteMap[note - 1];
	}
	// Key Off
	if (note >= 0x80)	// 0xFE or invalid note => key off
	{
		// Key Off
		KeyOff(nChn);
		// Note Cut
		if (note == 0xFE) {
			pChn->dwFlags |= (CHN_NOTEFADE | CHN_FASTVOLRAMP);
			if ((!(m_nType & MOD_TYPE_IT)) || (m_nInstruments))
				pChn->nVolume = 0;
			pChn->nFadeOutVol = 0;
		}
		return;
	}
	if (!pins)
		return;
	if ((!bPorta) && (m_nType & (MOD_TYPE_XM | MOD_TYPE_MED | MOD_TYPE_MT2))) {
		pChn->nTranspose = pins->RelativeTone;
		pChn->nFineTune = pins->nFineTune;
	}
	if (m_nType & (MOD_TYPE_XM | MOD_TYPE_MT2 | MOD_TYPE_MED))
		note += pChn->nTranspose;
	if (note < 1)
		note = 1;
	if (note > 132)
		note = 132;
	pChn->nNote = note;
	if ((!bPorta) || (m_nType & (MOD_TYPE_S3M | MOD_TYPE_IT)))
		pChn->nNewIns = 0;
	UINT period = GetPeriodFromNote(note, pChn->nFineTune, pChn->nC4Speed);
	if (period) {
		if ((!bPorta) || (!pChn->nPeriod))
			pChn->nPeriod = period;
		pChn->nPortamentoDest = period;
		if ((!bPorta) || ((!pChn->nLength) && (!(m_nType & MOD_TYPE_S3M)))) {
			pChn->pInstrument = pins;
			pChn->pSample = pins->pSample;
			pChn->nLength = pins->nLength;
			pChn->nLoopEnd = pins->nLength;
			pChn->nLoopStart = 0;
			pChn->dwFlags = (pChn->dwFlags & 0xFFFFFF00) | (pins->uFlags);
			if (pChn->dwFlags & CHN_SUSTAINLOOP) {
				pChn->nLoopStart = pins->nSustainStart;
				pChn->nLoopEnd = pins->nSustainEnd;
				pChn->dwFlags &= ~CHN_PINGPONGLOOP;
				pChn->dwFlags |= CHN_LOOP;
				if (pChn->dwFlags & CHN_PINGPONGSUSTAIN)
					pChn->dwFlags |= CHN_PINGPONGLOOP;
				if (pChn->nLength > pChn->nLoopEnd)
					pChn->nLength = pChn->nLoopEnd;
			} else if (pChn->dwFlags & CHN_LOOP) {
				pChn->nLoopStart = pins->nLoopStart;
				pChn->nLoopEnd = pins->nLoopEnd;
				if (pChn->nLength > pChn->nLoopEnd)
					pChn->nLength = pChn->nLoopEnd;
			}
			pChn->nPos = 0;
			pChn->nPosLo = 0;
			if (pChn->nVibratoType < 4)
				pChn->nVibratoPos = ((m_nType & MOD_TYPE_IT) &&
					(!(m_dwSongFlags & SONG_ITOLDEFFECTS))) ?
					0x10 :
					0;
			if (pChn->nTremoloType < 4)
				pChn->nTremoloPos = 0;
		}
		if (pChn->nPos >= pChn->nLength)
			pChn->nPos = pChn->nLoopStart;
	} else
		bPorta = FALSE;
	if ((!bPorta) ||
		(!(m_nType & MOD_TYPE_IT)) ||
		((pChn->dwFlags & CHN_NOTEFADE) && (!pChn->nFadeOutVol)) ||
		((m_dwSongFlags & SONG_ITCOMPATMODE) && (pChn->nRowInstr))) {
		if ((m_nType & MOD_TYPE_IT) &&
			(pChn->dwFlags & CHN_NOTEFADE) &&
			(!pChn->nFadeOutVol)) {
			pChn->nVolEnvPosition = 0;
			pChn->nPanEnvPosition = 0;
			pChn->nPitchEnvPosition = 0;
			pChn->nAutoVibDepth = 0;
			pChn->nAutoVibPos = 0;
			pChn->dwFlags &= ~CHN_NOTEFADE;
			pChn->nFadeOutVol = 65536;
		}
		if ((!bPorta) ||
			(!(m_dwSongFlags & SONG_ITCOMPATMODE)) ||
			(pChn->nRowInstr)) {
			if ((!(m_nType & (MOD_TYPE_XM | MOD_TYPE_MT2))) ||
				(pChn->nRowInstr)) {
				pChn->dwFlags &= ~CHN_NOTEFADE;
				pChn->nFadeOutVol = 65536;
			}
		}
	}
	pChn->dwFlags &= ~ (CHN_EXTRALOUD | CHN_KEYOFF);
	// Enable Ramping
	if (!bPorta) {
		pChn->nVUMeter = 0x100;
		pChn->nLeftVU = pChn->nRightVU = 0xFF;
		pChn->dwFlags &= ~CHN_FILTER;
		pChn->dwFlags |= CHN_FASTVOLRAMP;
		pChn->nRetrigCount = 0;
		pChn->nTremorCount = 0;
		if (bResetEnv) {
			pChn->nVolSwing = pChn->nPanSwing = 0;
			if (penv) {
				if (!(penv->dwFlags & ENV_VOLCARRY))
					pChn->nVolEnvPosition = 0;
				if (!(penv->dwFlags & ENV_PANCARRY))
					pChn->nPanEnvPosition = 0;
				if (!(penv->dwFlags & ENV_PITCHCARRY))
					pChn->nPitchEnvPosition = 0;
				if (m_nType & MOD_TYPE_IT) {
					// Volume Swing
					if (penv->nVolSwing) {
						int d = ((LONG)
							penv->nVolSwing*(LONG)
							((rand() & 0xFF) - 0x7F)) /
							128;
						pChn->nVolSwing = (signed short)
							((d * pChn->nVolume + 1) / 128);
					}
					// Pan Swing
					if (penv->nPanSwing) {
						int d = ((LONG)
							penv->nPanSwing*(LONG)
							((rand() & 0xFF) - 0x7F)) /
							128;
						pChn->nPanSwing = (signed short) d;
					}
				}
			}
			pChn->nAutoVibDepth = 0;
			pChn->nAutoVibPos = 0;
		}
		pChn->nLeftVol = pChn->nRightVol = 0;
		BOOL bFlt = (m_dwSongFlags& SONG_MPTFILTERMODE) ? FALSE : TRUE;
		// Setup Initial Filter for this note
		if (penv) {
			if (penv->nIFR & 0x80) {
				pChn->nResonance = penv->nIFR & 0x7F; bFlt = TRUE;
			}
			if (penv->nIFC & 0x80) {
				pChn->nCutOff = penv->nIFC & 0x7F; bFlt = TRUE;
			}
		} else {
			pChn->nVolSwing = pChn->nPanSwing = 0;
		}
	}
}


UINT CDecompressTrack::GetNNAChannel(UINT nChn) const
	//---------------------------------------------
{
	const MODCHANNEL* pChn =& Chn[nChn];
	// Check for empty channel
	const MODCHANNEL* pi =& Chn[m_nChannels];
	for (UINT i = m_nChannels; i < MAX_CHANNELS; i++, pi++)
		if (!pi->nLength)
			return i;
	if (!pChn->nFadeOutVol)
		return 0;
	// All channels are used: check for lowest volume
	UINT result = 0;
	DWORD vol = 64 * 65536;	// 25%
	DWORD envpos = 0xFFFFFF;
	const MODCHANNEL* pj =& Chn[m_nChannels];
	for (UINT j = m_nChannels; j < MAX_CHANNELS; j++, pj++) {
		if (!pj->nFadeOutVol)
			return j;
		DWORD v = pj->nVolume;
		if (pj->dwFlags & CHN_NOTEFADE)
			v = v * pj->nFadeOutVol;
		else
			v <<= 16;
		if (pj->dwFlags & CHN_LOOP)
			v >>= 1;
		if ((v < vol) || ((v == vol) && (pj->nVolEnvPosition > envpos))) {
			envpos = pj->nVolEnvPosition;
			vol = v;
			result = j;
		}
	}
	return result;
}


void CDecompressTrack::CheckNNA(UINT nChn, UINT instr, int note,
	BOOL bForceCut)
	//------------------------------------------------------------------------
{
	MODCHANNEL* pChn =& Chn[nChn];
	INSTRUMENTHEADER* penv = pChn->pHeader,* pHeader;
	signed char * pSample;
	if (note > 0x80)
		note = 0;
	if (note < 1)
		return;
	// Always NNA cut - using
	if ((!(m_nType & (MOD_TYPE_IT | MOD_TYPE_MT2))) ||
		(!m_nInstruments) ||
		(bForceCut)) {
		if ((m_dwSongFlags & SONG_CPUVERYHIGH) ||
			(!pChn->nLength) ||
			(pChn->dwFlags & CHN_MUTE) ||
			((!pChn->nLeftVol) && (!pChn->nRightVol)))
			return;
		UINT n = GetNNAChannel(nChn);
		if (!n)
			return;
		MODCHANNEL* p =& Chn[n];
		// Copy Channel
		*p = *pChn;
		p->dwFlags &= ~ (CHN_VIBRATO |
			CHN_TREMOLO |
			CHN_PANBRELLO |
			CHN_MUTE |
			CHN_PORTAMENTO);
		p->nMasterChn = nChn + 1;
		p->nCommand = 0;
		// Cut the note
		p->nFadeOutVol = 0;
		p->dwFlags |= (CHN_NOTEFADE | CHN_FASTVOLRAMP);
		// Stop this channel
		pChn->nLength = pChn->nPos = pChn->nPosLo = 0;
		pChn->nROfs = pChn->nLOfs = 0;
		pChn->nLeftVol = pChn->nRightVol = 0;
		return;
	}
	if (instr >= MAX_INSTRUMENTS)
		instr = 0;
	pSample = pChn->pSample;
	pHeader = pChn->pHeader;
	if ((instr) && (note)) {
		pHeader = Headers[instr];
		if (pHeader) {
			UINT n = 0;
			if (note <= 0x80) {
				n = pHeader->Keyboard[note - 1];
				note = pHeader->NoteMap[note - 1];
				if ((n) && (n < MAX_SAMPLES))
					pSample = Ins[n].pSample;
			}
		} else
			pSample = NULL;
	}
	if (!penv)
		return;
	MODCHANNEL* p = pChn;
	for (UINT i = nChn; i < MAX_CHANNELS; p++, i++)
		if ((i >= m_nChannels) || (p == pChn)) {
			if (((p->nMasterChn == nChn + 1) || (p == pChn)) && (p->pHeader)) {
				BOOL bOk = FALSE;
				// Duplicate Check Type
				switch (p->pHeader->nDCT) {
					// Note
				case DCT_NOTE:
					if ((note) &&
						(p->nNote == note) &&
						(pHeader == p->pHeader))
						bOk = TRUE;
					break;
					// Sample
				case DCT_SAMPLE:
					if ((pSample) && (pSample == p->pSample))
						bOk = TRUE;
					break;
					// Instrument
				case DCT_INSTRUMENT:
					if (pHeader == p->pHeader)
						bOk = TRUE;
					break;
				}
				// Duplicate Note Action
				if (bOk) {
					switch (p->pHeader->nDNA) {
						// Cut
					case DNA_NOTECUT:
						KeyOff(i);
						p->nVolume = 0;
						break;
						// Note Off
					case DNA_NOTEOFF:
						KeyOff(i);
						break;
						// Note Fade
					case DNA_NOTEFADE:
						p->dwFlags |= CHN_NOTEFADE;
						break;
					}
					if (!p->nVolume) {
						p->nFadeOutVol = 0;
						p->dwFlags |= (CHN_NOTEFADE | CHN_FASTVOLRAMP);
					}
				}
			}
		}
	if (pChn->dwFlags & CHN_MUTE)
		return;
	// New Note Action
	if ((pChn->nVolume) && (pChn->nLength)) {
		UINT n = GetNNAChannel(nChn);
		if (n) {
			MODCHANNEL* p =& Chn[n];
			// Copy Channel
			*p = *pChn;
			p->dwFlags &= ~ (CHN_VIBRATO |
				CHN_TREMOLO |
				CHN_PANBRELLO |
				CHN_MUTE |
				CHN_PORTAMENTO);
			p->nMasterChn = nChn + 1;
			p->nCommand = 0;
			// Key Off the note
			switch (pChn->nNNA) {
			case NNA_NOTEOFF:
				KeyOff(n); break;
			case NNA_NOTECUT:
				p->nFadeOutVol = 0;
			case NNA_NOTEFADE:
				p->dwFlags |= CHN_NOTEFADE; break;
			}
			if (!p->nVolume) {
				p->nFadeOutVol = 0;
				p->dwFlags |= (CHN_NOTEFADE | CHN_FASTVOLRAMP);
			}
			// Stop this channel
			pChn->nLength = pChn->nPos = pChn->nPosLo = 0;
			pChn->nROfs = pChn->nLOfs = 0;
		}
	}
}


BOOL CDecompressTrack::ProcessEffects()
	//-------------------------------
{
	int nBreakRow = - 1, nPosJump = -1, nPatLoopRow = -1;
	MODCHANNEL* pChn = Chn;
	for (UINT nChn = 0; nChn < m_nChannels; nChn++, pChn++) {
		UINT instr = pChn->nRowInstr;
		UINT volcmd = pChn->nRowVolCmd;
		UINT vol = pChn->nRowVolume;
		UINT cmd = pChn->nRowCommand;
		UINT param = pChn->nRowParam;
		BOOL bPorta = ((cmd != CMD_TONEPORTAMENTO) &&
			(cmd != CMD_TONEPORTAVOL) &&
			(volcmd != VOLCMD_TONEPORTAMENTO)) ?
			FALSE :
			TRUE;
		UINT nStartTick = 0;

		pChn->dwFlags &= ~CHN_FASTVOLRAMP;
		// Process special effects (note delay, pattern delay, pattern loop)
		if ((cmd == CMD_MODCMDEX) || (cmd == CMD_S3MCMDEX)) {
			if ((!param) && (m_nType & (MOD_TYPE_S3M | MOD_TYPE_IT)))
				param = pChn->nOldCmdEx;
			else
				pChn->nOldCmdEx = param;
			// Note Delay ?
			if ((param & 0xF0) == 0xD0) {
				nStartTick = param & 0x0F;
			} else if (!m_nTickCount) {
				// Pattern Loop ?
				if ((((param & 0xF0) == 0x60) && (cmd == CMD_MODCMDEX)) ||
					(((param & 0xF0) == 0xB0) && (cmd == CMD_S3MCMDEX))) {
					int nloop = PatternLoop(pChn, param & 0x0F);
					if (nloop >= 0)
						nPatLoopRow = nloop;
				} else
				// Pattern Delay
				if ((param & 0xF0) == 0xE0) {
					m_nPatternDelay = param & 0x0F;
				}
			}
		}

		// Handles note/instrument/volume changes
		if (m_nTickCount == nStartTick) // can be delayed by a note delay effect
		{
			UINT note = pChn->nRowNote;
			if (instr)
				pChn->nNewIns = instr;
			// XM: Key-Off + Sample == Note Cut
			if (m_nType & (MOD_TYPE_MOD | MOD_TYPE_XM | MOD_TYPE_MT2)) {
				if ((note == 0xFF) &&
					((!pChn->pHeader) ||
					(!(pChn->pHeader->dwFlags & ENV_VOLUME)))) {
					pChn->dwFlags |= CHN_FASTVOLRAMP;
					pChn->nVolume = 0;
					note = instr = 0;
				}
			}
			if ((!note) && (instr)) {
				if (m_nInstruments) {
					if (pChn->pInstrument)
						pChn->nVolume = pChn->pInstrument->nVolume;
					if (m_nType & (MOD_TYPE_XM | MOD_TYPE_MT2)) {
						pChn->dwFlags |= CHN_FASTVOLRAMP;
						pChn->nVolEnvPosition = 0;
						pChn->nPanEnvPosition = 0;
						pChn->nPitchEnvPosition = 0;
						pChn->nAutoVibDepth = 0;
						pChn->nAutoVibPos = 0;
						pChn->dwFlags &= ~CHN_NOTEFADE;
						pChn->nFadeOutVol = 65536;
					}
				} else {
					if (instr < MAX_SAMPLES)
						pChn->nVolume = Ins[instr].nVolume;
				}
				if (!(m_nType & MOD_TYPE_IT))
					instr = 0;
			}
			// Invalid Instrument ?
			if (instr >= MAX_INSTRUMENTS)
				instr = 0;
			// Note Cut/Off => ignore instrument
			if (note >= 0xFE)
				instr = 0;
			if ((note) && (note <= 128))
				pChn->nNewNote = note;
			// New Note Action ?
			if ((note) && (note <= 128) && (!bPorta)) {
				CheckNNA(nChn, instr, note, FALSE);
			}
			// Instrument Change ?
			if (instr) {
				MODINSTRUMENT* psmp = pChn->pInstrument;
				InstrumentChange(pChn, instr, bPorta, TRUE);
				pChn->nNewIns = 0;
				// Special IT case: portamento+note causes sample change -> ignore portamento
				if ((m_nType & (MOD_TYPE_S3M | MOD_TYPE_IT)) &&
					(psmp != pChn->pInstrument) &&
					(note) &&
					(note < 0x80)) {
					bPorta = FALSE;
				}
			}
			// New Note ?
			if (note) {
				if ((!instr) && (pChn->nNewIns) && (note < 0x80)) {
					InstrumentChange(pChn,
						pChn->nNewIns,
						bPorta,
						FALSE,
						(m_nType & (MOD_TYPE_XM | MOD_TYPE_MT2)) ?
						FALSE :
						TRUE);
					pChn->nNewIns = 0;
				}
				NoteChange(nChn,
					note,
					bPorta,
					(m_nType & (MOD_TYPE_XM | MOD_TYPE_MT2)) ? FALSE : TRUE);
				if ((bPorta) &&
					(m_nType & (MOD_TYPE_XM | MOD_TYPE_MT2)) &&
					(instr)) {
					pChn->dwFlags |= CHN_FASTVOLRAMP;
					pChn->nVolEnvPosition = 0;
					pChn->nPanEnvPosition = 0;
					pChn->nPitchEnvPosition = 0;
					pChn->nAutoVibDepth = 0;
					pChn->nAutoVibPos = 0;
				}
			}
			// Tick-0 only volume commands
			if (volcmd == VOLCMD_VOLUME) {
				if (vol > 64)
					vol = 64;
				pChn->nVolume = vol << 2;
				pChn->dwFlags |= CHN_FASTVOLRAMP;
			} else if (volcmd == VOLCMD_PANNING) {
				if (vol > 64)
					vol = 64;
				pChn->nPan = vol << 2;
				pChn->dwFlags |= CHN_FASTVOLRAMP;
			}
		}

		// Volume Column Effect (except volume & panning)
		if ((volcmd > VOLCMD_PANNING) && (m_nTickCount >= nStartTick)) {
			if (volcmd == VOLCMD_TONEPORTAMENTO) {
				if (m_nType & MOD_TYPE_IT)
					TonePortamento(pChn, ImpulseTrackerPortaVolCmd[vol & 0x0F]);
				else
					TonePortamento(pChn, vol * 16);
			} else {
				if (vol)
					pChn->nOldVolParam = vol;
				else
					vol = pChn->nOldVolParam;
				switch (volcmd) {
				case VOLCMD_VOLSLIDEUP:
					VolumeSlide(pChn, vol << 4);
					break;

				case VOLCMD_VOLSLIDEDOWN:
					VolumeSlide(pChn, vol);
					break;

				case VOLCMD_FINEVOLUP:
					if (m_nType & MOD_TYPE_IT) {
						if (m_nTickCount == nStartTick)
							VolumeSlide(pChn, (vol << 4) | 0x0F);
					} else
						FineVolumeUp(pChn, vol);
					break;

				case VOLCMD_FINEVOLDOWN:
					if (m_nType & MOD_TYPE_IT) {
						if (m_nTickCount == nStartTick)
							VolumeSlide(pChn, 0xF0 | vol);
					} else
						FineVolumeDown(pChn, vol);
					break;

				case VOLCMD_VIBRATOSPEED:
					Vibrato(pChn, vol << 4);
					break;

				case VOLCMD_VIBRATO:
					Vibrato(pChn, vol);
					break;

				case VOLCMD_PANSLIDELEFT:
					PanningSlide(pChn, vol);
					break;

				case VOLCMD_PANSLIDERIGHT:
					PanningSlide(pChn, vol << 4);
					break;

				case VOLCMD_PORTAUP:
					PortamentoUp(pChn, vol << 2);
					break;

				case VOLCMD_PORTADOWN:
					PortamentoDown(pChn, vol << 2);
					break;
				}
			}
		}

		// Effects
		if (cmd)
			switch (cmd) {
				// Set Volume
			case CMD_VOLUME:
				if (!m_nTickCount) {
					pChn->nVolume = (param < 64) ? param * 4 : 256;
					pChn->dwFlags |= CHN_FASTVOLRAMP;
				}
				break;

				// Portamento Up
			case CMD_PORTAMENTOUP:
				if ((!param) && (m_nType & MOD_TYPE_MOD))
					break;
				PortamentoUp(pChn, param);
				break;

				// Portamento Down
			case CMD_PORTAMENTODOWN:
				if ((!param) && (m_nType & MOD_TYPE_MOD))
					break;
				PortamentoDown(pChn, param);
				break;

				// Volume Slide
			case CMD_VOLUMESLIDE:
				if ((param) || (m_nType != MOD_TYPE_MOD))
					VolumeSlide(pChn, param);
				break;

				// Tone-Portamento
			case CMD_TONEPORTAMENTO:
				TonePortamento(pChn, param);
				break;

				// Tone-Portamento + Volume Slide
			case CMD_TONEPORTAVOL:
				if ((param) || (m_nType != MOD_TYPE_MOD))
					VolumeSlide(pChn, param);
				TonePortamento(pChn, 0);
				break;

				// Vibrato
			case CMD_VIBRATO:
				Vibrato(pChn, param);
				break;

				// Vibrato + Volume Slide
			case CMD_VIBRATOVOL:
				if ((param) || (m_nType != MOD_TYPE_MOD))
					VolumeSlide(pChn, param);
				Vibrato(pChn, 0);
				break;

				// Set Speed
			case CMD_SPEED:
				if (!m_nTickCount)
					SetSpeed(param);
				break;

				// Set Tempo
			case CMD_TEMPO:
				if (!m_nTickCount) {
					if (m_nType & (MOD_TYPE_S3M | MOD_TYPE_IT)) {
						if (param)
							pChn->nOldTempo = param;
						else
							param = pChn->nOldTempo;
					}
					SetTempo(param);
				}
				break;

				// Set Offset
			case CMD_OFFSET:
				if (m_nTickCount)
					break;
				if (param)
					pChn->nOldOffset = param;
				else
					param = pChn->nOldOffset;
				param <<= 8;
				param |= (UINT) (pChn->nOldHiOffset) << 16;
				if ((pChn->nRowNote) && (pChn->nRowNote < 0x80)) {
					if (bPorta)
						pChn->nPos = param;
					else
						pChn->nPos += param;
					if (pChn->nPos >= pChn->nLength) {
						if (!(m_nType & (MOD_TYPE_XM | MOD_TYPE_MT2))) {
							pChn->nPos = pChn->nLoopStart;
							if ((m_dwSongFlags & SONG_ITOLDEFFECTS) &&
								(pChn->nLength > 4)) {
								pChn->nPos = pChn->nLength - 2;
							}
						}
					}
				} else if ((param < pChn->nLength) &&
					(m_nType & (MOD_TYPE_MTM | MOD_TYPE_DMF))) {
					pChn->nPos = param;
				}
				break;

				// Arpeggio
			case CMD_ARPEGGIO:
				if ((m_nTickCount) || (!pChn->nPeriod) || (!pChn->nNote))
					break;
				if ((!param) && (!(m_nType & (MOD_TYPE_S3M | MOD_TYPE_IT))))
					break;
				pChn->nCommand = CMD_ARPEGGIO;
				if (param)
					pChn->nArpeggio = param;
				break;

				// Retrig
			case CMD_RETRIG:
				if (m_nType & (MOD_TYPE_XM | MOD_TYPE_MT2)) {
					if (!(param & 0xF0))
						param |= pChn->nRetrigParam & 0xF0;
					if (!(param & 0x0F))
						param |= pChn->nRetrigParam & 0x0F;
					param |= 0x100; // increment retrig count on first row
				}
				if (param)
					pChn->nRetrigParam = (BYTE) (param & 0xFF);
				else
					param = pChn->nRetrigParam;
				RetrigNote(nChn, param);
				break;

				// Tremor
			case CMD_TREMOR:
				if (m_nTickCount)
					break;
				pChn->nCommand = CMD_TREMOR;
				if (param)
					pChn->nTremorParam = param;
				break;

				// Set Global Volume
			case CMD_GLOBALVOLUME:
				if (m_nTickCount)
					break;
				if (m_nType != MOD_TYPE_IT)
					param <<= 1;
				if (param > 128)
					param = 128;
				m_nGlobalVolume = param << 1;
				break;

				// Global Volume Slide
			case CMD_GLOBALVOLSLIDE:
				GlobalVolSlide(param);
				break;

				// Set 8-bit Panning
			case CMD_PANNING8:
				if (m_nTickCount)
					break;
				if (!(m_dwSongFlags & SONG_SURROUNDPAN))
					pChn->dwFlags &= ~CHN_SURROUND;
				if (m_nType & (MOD_TYPE_IT | MOD_TYPE_XM | MOD_TYPE_MT2)) {
					pChn->nPan = param;
				} else if (param <= 0x80) {
					pChn->nPan = param << 1;
				} else if (param == 0xA4) {
					pChn->dwFlags |= CHN_SURROUND;
					pChn->nPan = 0x80;
				}
				pChn->dwFlags |= CHN_FASTVOLRAMP;
				break;

				// Panning Slide
			case CMD_PANNINGSLIDE:
				PanningSlide(pChn, param);
				break;

				// Tremolo
			case CMD_TREMOLO:
				Tremolo(pChn, param);
				break;

				// Fine Vibrato
			case CMD_FINEVIBRATO:
				FineVibrato(pChn, param);
				break;

				// MOD/XM Exx Extended Commands
			case CMD_MODCMDEX:
				ExtendedMODCommands(nChn, param);
				break;

				// S3M/IT Sxx Extended Commands
			case CMD_S3MCMDEX:
				ExtendedS3MCommands(nChn, param);
				break;

				// Key Off
			case CMD_KEYOFF:
				if (!m_nTickCount)
					KeyOff(nChn);
				break;

				// Extra-fine porta up/down
			case CMD_XFINEPORTAUPDOWN:
				switch (param & 0xF0) {
				case 0x10:
					ExtraFinePortamentoUp(pChn, param & 0x0F); break;
				case 0x20:
					ExtraFinePortamentoDown(pChn, param & 0x0F); break;
					// Modplug XM Extensions
				case 0x50:
				case 0x60:
				case 0x70:
				case 0x90:
				case 0xA0:
					ExtendedS3MCommands(nChn, param); break;
				}
				break;

				// Set Channel Global Volume
			case CMD_CHANNELVOLUME:
				if (m_nTickCount)
					break;
				if (param <= 64) {
					pChn->nGlobalVol = param;
					pChn->dwFlags |= CHN_FASTVOLRAMP;
				}
				break;

				// Channel volume slide
			case CMD_CHANNELVOLSLIDE:
				ChannelVolSlide(pChn, param);
				break;

				// Panbrello (IT)
			case CMD_PANBRELLO:
				Panbrello(pChn, param);
				break;

				// Set Envelope Position
			case CMD_SETENVPOSITION:
				if (!m_nTickCount) {
					pChn->nVolEnvPosition = param;
					pChn->nPanEnvPosition = param;
					pChn->nPitchEnvPosition = param;
					if (pChn->pHeader) {
						INSTRUMENTHEADER* penv = pChn->pHeader;
						if ((pChn->dwFlags & CHN_PANENV) &&
							(penv->nPanEnv) &&
							(param > penv->PanPoints[penv->nPanEnv - 1])) {
							pChn->dwFlags &= ~CHN_PANENV;
						}
					}
				}
				break;

				// Position Jump
			case CMD_POSITIONJUMP:
				nPosJump = param;
				break;

				// Pattern Break
			case CMD_PATTERNBREAK:
				nBreakRow = param;
				break;

				// Midi Controller
			case CMD_MIDI:
				if (m_nTickCount)
					break;
				if (param < 0x80) {
					ProcessMidiMacro(nChn,
						& m_MidiCfg.szMidiSFXExt[pChn->nActiveMacro << 5],
						param);
				} else {
					ProcessMidiMacro(nChn,
						& m_MidiCfg.szMidiZXXExt[(param & 0x7F) << 5],
						0);
				}
				break;
			}
	}

	// Navigation Effects
	if (!m_nTickCount) {
		// Pattern Loop
		if (nPatLoopRow >= 0) {
			m_nNextPattern = m_nCurrentPattern;
			m_nNextRow = nPatLoopRow;
			if (m_nPatternDelay)
				m_nNextRow++;
		} else
		// Pattern Break / Position Jump only if no loop running
		if ((nBreakRow >= 0) || (nPosJump >= 0)) {
			BOOL bNoLoop = FALSE;
			if (nPosJump < 0)
				nPosJump = m_nCurrentPattern + 1;
			if (nBreakRow < 0)
				nBreakRow = 0;
			// Modplug Tracker & ModPlugin allow backward jumps
			//    #ifndef MODPLUG_FASTSOUNDLIB
			if ((nPosJump < (int) m_nCurrentPattern) ||
				((nPosJump == (int) m_nCurrentPattern) &&
				(nBreakRow <= (int) m_nRow))) {
				if (!IsValidBackwardJump(m_nCurrentPattern,
					 	m_nRow,
					 	nPosJump,
					 	nBreakRow)) {
					if (m_nRepeatCount) {
						if (m_nRepeatCount > 0)
							m_nRepeatCount--;
					} else {
#ifdef MODPLUG_TRACKER
						if (gdwSoundSetup & SNDMIX_NOBACKWARDJUMPS)
							#endif
							// Backward jump disabled
							bNoLoop = TRUE;
						//reset repeat count incase there are multiple loops.
						//(i.e. Unreal tracks)
						m_nRepeatCount = m_nInitialRepeatCount;
					}
				}
			}
			//		#endif	// MODPLUG_FASTSOUNDLIB
			if (((!bNoLoop) && (nPosJump < MAX_ORDERS)) &&
				((nPosJump != (int) m_nCurrentPattern) ||
				(nBreakRow != (int) m_nRow))) {
				if (nPosJump != (int) m_nCurrentPattern) {
					for (UINT i = 0; i < m_nChannels; i++)
						Chn[i].nPatternLoopCount = 0;
				}
				m_nNextPattern = nPosJump;
				m_nNextRow = (UINT) nBreakRow;
			}
		}
	}
	return TRUE;
}


////////////////////////////////////////////////////////////
// Channels effects

void CDecompressTrack::PortamentoUp(MODCHANNEL* pChn, UINT param)
	//---------------------------------------------------------
{
	if (param)
		pChn->nOldPortaUpDown = param;
	else
		param = pChn->nOldPortaUpDown;
	if ((m_nType & (MOD_TYPE_S3M | MOD_TYPE_IT | MOD_TYPE_STM)) &&
		((param & 0xF0) >= 0xE0)) {
		if (param & 0x0F) {
			if ((param & 0xF0) == 0xF0) {
				FinePortamentoUp(pChn, param & 0x0F);
			} else if ((param & 0xF0) == 0xE0) {
				ExtraFinePortamentoUp(pChn, param & 0x0F);
			}
		}
		return;
	}
	// Regular Slide
	if (!(m_dwSongFlags & SONG_FIRSTTICK)) {
		DoFreqSlide(pChn, -(int) (param * 4));
	}
}


void CDecompressTrack::PortamentoDown(MODCHANNEL* pChn, UINT param)
	//-----------------------------------------------------------
{
	if (param)
		pChn->nOldPortaUpDown = param;
	else
		param = pChn->nOldPortaUpDown;
	if ((m_nType & (MOD_TYPE_S3M | MOD_TYPE_IT | MOD_TYPE_STM)) &&
		((param & 0xF0) >= 0xE0)) {
		if (param & 0x0F) {
			if ((param & 0xF0) == 0xF0) {
				FinePortamentoDown(pChn, param & 0x0F);
			} else if ((param & 0xF0) == 0xE0) {
				ExtraFinePortamentoDown(pChn, param & 0x0F);
			}
		}
		return;
	}
	if (!(m_dwSongFlags & SONG_FIRSTTICK))
		DoFreqSlide(pChn, (int) (param << 2));
}


void CDecompressTrack::FinePortamentoUp(MODCHANNEL* pChn, UINT param)
	//-------------------------------------------------------------
{
	if (m_nType & (MOD_TYPE_XM | MOD_TYPE_MT2)) {
		if (param)
			pChn->nOldFinePortaUpDown = param;
		else
			param = pChn->nOldFinePortaUpDown;
	}
	if (m_dwSongFlags & SONG_FIRSTTICK) {
		if ((pChn->nPeriod) && (param)) {
			if ((m_dwSongFlags & SONG_LINEARSLIDES) &&
				(!(m_nType & (MOD_TYPE_XM | MOD_TYPE_MT2)))) {
				pChn->nPeriod = _muldivr(pChn->nPeriod,
									LinearSlideDownTable[param & 0x0F],
									65536);
			} else {
				pChn->nPeriod -= (int) (param * 4);
			}
			if (pChn->nPeriod < 1)
				pChn->nPeriod = 1;
		}
	}
}


void CDecompressTrack::FinePortamentoDown(MODCHANNEL* pChn, UINT param)
	//---------------------------------------------------------------
{
	if (m_nType & (MOD_TYPE_XM | MOD_TYPE_MT2)) {
		if (param)
			pChn->nOldFinePortaUpDown = param;
		else
			param = pChn->nOldFinePortaUpDown;
	}
	if (m_dwSongFlags & SONG_FIRSTTICK) {
		if ((pChn->nPeriod) && (param)) {
			if ((m_dwSongFlags & SONG_LINEARSLIDES) &&
				(!(m_nType & (MOD_TYPE_XM | MOD_TYPE_MT2)))) {
				pChn->nPeriod = _muldivr(pChn->nPeriod,
									LinearSlideUpTable[param & 0x0F],
									65536);
			} else {
				pChn->nPeriod += (int) (param * 4);
			}
			if (pChn->nPeriod > 0xFFFF)
				pChn->nPeriod = 0xFFFF;
		}
	}
}


void CDecompressTrack::ExtraFinePortamentoUp(MODCHANNEL* pChn, UINT param)
	//------------------------------------------------------------------
{
	if (m_nType & (MOD_TYPE_XM | MOD_TYPE_MT2)) {
		if (param)
			pChn->nOldFinePortaUpDown = param;
		else
			param = pChn->nOldFinePortaUpDown;
	}
	if (m_dwSongFlags & SONG_FIRSTTICK) {
		if ((pChn->nPeriod) && (param)) {
			if ((m_dwSongFlags & SONG_LINEARSLIDES) &&
				(!(m_nType & (MOD_TYPE_XM | MOD_TYPE_MT2)))) {
				pChn->nPeriod = _muldivr(pChn->nPeriod,
									FineLinearSlideDownTable[param & 0x0F],
									65536);
			} else {
				pChn->nPeriod -= (int) (param);
			}
			if (pChn->nPeriod < 1)
				pChn->nPeriod = 1;
		}
	}
}


void CDecompressTrack::ExtraFinePortamentoDown(MODCHANNEL* pChn, UINT param)
	//--------------------------------------------------------------------
{
	if (m_nType & (MOD_TYPE_XM | MOD_TYPE_MT2)) {
		if (param)
			pChn->nOldFinePortaUpDown = param;
		else
			param = pChn->nOldFinePortaUpDown;
	}
	if (m_dwSongFlags & SONG_FIRSTTICK) {
		if ((pChn->nPeriod) && (param)) {
			if ((m_dwSongFlags & SONG_LINEARSLIDES) &&
				(!(m_nType & (MOD_TYPE_XM | MOD_TYPE_MT2)))) {
				pChn->nPeriod = _muldivr(pChn->nPeriod,
									FineLinearSlideUpTable[param & 0x0F],
									65536);
			} else {
				pChn->nPeriod += (int) (param);
			}
			if (pChn->nPeriod > 0xFFFF)
				pChn->nPeriod = 0xFFFF;
		}
	}
}


// Portamento Slide
void CDecompressTrack::TonePortamento(MODCHANNEL* pChn, UINT param)
	//-----------------------------------------------------------
{
	if (param)
		pChn->nPortamentoSlide = param * 4;
	pChn->dwFlags |= CHN_PORTAMENTO;
	if ((pChn->nPeriod) &&
		(pChn->nPortamentoDest) &&
		(!(m_dwSongFlags & SONG_FIRSTTICK))) {
		if (pChn->nPeriod < pChn->nPortamentoDest) {
			LONG delta = (int) pChn->nPortamentoSlide;
			if ((m_dwSongFlags & SONG_LINEARSLIDES) &&
				(!(m_nType & (MOD_TYPE_XM | MOD_TYPE_MT2)))) {
				UINT n = pChn->nPortamentoSlide >> 2;
				if (n > 255)
					n = 255;
				delta = _muldivr(pChn->nPeriod, LinearSlideUpTable[n], 65536) -
					pChn->nPeriod;
				if (delta < 1)
					delta = 1;
			}
			pChn->nPeriod += delta;
			if (pChn->nPeriod > pChn->nPortamentoDest)
				pChn->nPeriod = pChn->nPortamentoDest;
		} else if (pChn->nPeriod > pChn->nPortamentoDest) {
			LONG delta = - (int) pChn->nPortamentoSlide;
			if ((m_dwSongFlags & SONG_LINEARSLIDES) &&
				(!(m_nType & (MOD_TYPE_XM | MOD_TYPE_MT2)))) {
				UINT n = pChn->nPortamentoSlide >> 2;
				if (n > 255)
					n = 255;
				delta = _muldivr(pChn->nPeriod, LinearSlideDownTable[n], 65536) -
					pChn->nPeriod;
				if (delta > -1)
					delta = -1;
			}
			pChn->nPeriod += delta;
			if (pChn->nPeriod < pChn->nPortamentoDest)
				pChn->nPeriod = pChn->nPortamentoDest;
		}
	}
}


void CDecompressTrack::Vibrato(MODCHANNEL* p, UINT param)
	//-------------------------------------------------
{
	if (param & 0x0F)
		p->nVibratoDepth = (param & 0x0F) * 4;
	if (param & 0xF0)
		p->nVibratoSpeed = (param >> 4) & 0x0F;
	p->dwFlags |= CHN_VIBRATO;
}


void CDecompressTrack::FineVibrato(MODCHANNEL* p, UINT param)
	//-----------------------------------------------------
{
	if (param & 0x0F)
		p->nVibratoDepth = param & 0x0F;
	if (param & 0xF0)
		p->nVibratoSpeed = (param >> 4) & 0x0F;
	p->dwFlags |= CHN_VIBRATO;
}


void CDecompressTrack::Panbrello(MODCHANNEL* p, UINT param)
	//---------------------------------------------------
{
	if (param & 0x0F)
		p->nPanbrelloDepth = param & 0x0F;
	if (param & 0xF0)
		p->nPanbrelloSpeed = (param >> 4) & 0x0F;
	p->dwFlags |= CHN_PANBRELLO;
}


void CDecompressTrack::VolumeSlide(MODCHANNEL* pChn, UINT param)
	//--------------------------------------------------------
{
	if (param)
		pChn->nOldVolumeSlide = param;
	else
		param = pChn->nOldVolumeSlide;
	LONG newvolume = pChn->nVolume;
	if (m_nType & (MOD_TYPE_S3M | MOD_TYPE_IT | MOD_TYPE_STM | MOD_TYPE_AMF)) {
		if ((param & 0x0F) == 0x0F) {
			if (param & 0xF0) {
				FineVolumeUp(pChn, (param >> 4));
				return;
			} else {
				if ((m_dwSongFlags & SONG_FIRSTTICK) &&
					(!(m_dwSongFlags & SONG_FASTVOLSLIDES))) {
					newvolume -= 0x0F * 4;
				}
			}
		} else if ((param & 0xF0) == 0xF0) {
			if (param & 0x0F) {
				FineVolumeDown(pChn, (param & 0x0F));
				return;
			} else {
				if ((m_dwSongFlags & SONG_FIRSTTICK) &&
					(!(m_dwSongFlags & SONG_FASTVOLSLIDES))) {
					newvolume += 0x0F * 4;
				}
			}
		}
	}
	if ((!(m_dwSongFlags & SONG_FIRSTTICK)) ||
		(m_dwSongFlags & SONG_FASTVOLSLIDES)) {
		if (param & 0x0F)
			newvolume -= (int) ((param & 0x0F) * 4);
		else
			newvolume += (int) ((param & 0xF0) >> 2);
		if (m_nType & MOD_TYPE_MOD)
			pChn->dwFlags |= CHN_FASTVOLRAMP;
	}
	if (newvolume < 0)
		newvolume = 0;
	if (newvolume > 256)
		newvolume = 256;
	pChn->nVolume = newvolume;
}


void CDecompressTrack::PanningSlide(MODCHANNEL* pChn, UINT param)
	//---------------------------------------------------------
{
	LONG nPanSlide = 0;
	if (param)
		pChn->nOldPanSlide = param;
	else
		param = pChn->nOldPanSlide;
	if (m_nType & (MOD_TYPE_S3M | MOD_TYPE_IT | MOD_TYPE_STM)) {
		if (((param & 0x0F) == 0x0F) && (param & 0xF0)) {
			if (m_dwSongFlags & SONG_FIRSTTICK) {
				param = (param & 0xF0) >> 2;
				nPanSlide = - (int) param;
			}
		} else if (((param & 0xF0) == 0xF0) && (param & 0x0F)) {
			if (m_dwSongFlags & SONG_FIRSTTICK) {
				nPanSlide = (param & 0x0F) << 2;
			}
		} else {
			if (!(m_dwSongFlags & SONG_FIRSTTICK)) {
				if (param & 0x0F)
					nPanSlide = (int) ((param & 0x0F) << 2);
				else
					nPanSlide = -(int) ((param & 0xF0) >> 2);
			}
		}
	} else {
		if (!(m_dwSongFlags & SONG_FIRSTTICK)) {
			if (param & 0x0F)
				nPanSlide = -(int) ((param & 0x0F) << 2);
			else
				nPanSlide = (int) ((param & 0xF0) >> 2);
		}
	}
	if (nPanSlide) {
		nPanSlide += pChn->nPan;
		if (nPanSlide < 0)
			nPanSlide = 0;
		if (nPanSlide > 256)
			nPanSlide = 256;
		pChn->nPan = nPanSlide;
	}
}


void CDecompressTrack::FineVolumeUp(MODCHANNEL* pChn, UINT param)
	//---------------------------------------------------------
{
	if (param)
		pChn->nOldFineVolUpDown = param;
	else
		param = pChn->nOldFineVolUpDown;
	if (m_dwSongFlags & SONG_FIRSTTICK) {
		pChn->nVolume += param * 4;
		if (pChn->nVolume > 256)
			pChn->nVolume = 256;
		if (m_nType & MOD_TYPE_MOD)
			pChn->dwFlags |= CHN_FASTVOLRAMP;
	}
}


void CDecompressTrack::FineVolumeDown(MODCHANNEL* pChn, UINT param)
	//-----------------------------------------------------------
{
	if (param)
		pChn->nOldFineVolUpDown = param;
	else
		param = pChn->nOldFineVolUpDown;
	if (m_dwSongFlags & SONG_FIRSTTICK) {
		pChn->nVolume -= param * 4;
		if (pChn->nVolume < 0)
			pChn->nVolume = 0;
		if (m_nType & MOD_TYPE_MOD)
			pChn->dwFlags |= CHN_FASTVOLRAMP;
	}
}


void CDecompressTrack::Tremolo(MODCHANNEL* p, UINT param)
	//-------------------------------------------------
{
	if (param & 0x0F)
		p->nTremoloDepth = (param & 0x0F) << 2;
	if (param & 0xF0)
		p->nTremoloSpeed = (param >> 4) & 0x0F;
	p->dwFlags |= CHN_TREMOLO;
}


void CDecompressTrack::ChannelVolSlide(MODCHANNEL* pChn, UINT param)
	//------------------------------------------------------------
{
	LONG nChnSlide = 0;
	if (param)
		pChn->nOldChnVolSlide = param;
	else
		param = pChn->nOldChnVolSlide;
	if (((param & 0x0F) == 0x0F) && (param & 0xF0)) {
		if (m_dwSongFlags & SONG_FIRSTTICK)
			nChnSlide = param >> 4;
	} else if (((param & 0xF0) == 0xF0) && (param & 0x0F)) {
		if (m_dwSongFlags & SONG_FIRSTTICK)
			nChnSlide = - (int) (param & 0x0F);
	} else {
		if (!(m_dwSongFlags & SONG_FIRSTTICK)) {
			if (param & 0x0F)
				nChnSlide = -(int) (param & 0x0F);
			else
				nChnSlide = (int) ((param & 0xF0) >> 4);
		}
	}
	if (nChnSlide) {
		nChnSlide += pChn->nGlobalVol;
		if (nChnSlide < 0)
			nChnSlide = 0;
		if (nChnSlide > 64)
			nChnSlide = 64;
		pChn->nGlobalVol = nChnSlide;
	}
}


void CDecompressTrack::ExtendedMODCommands(UINT nChn, UINT param)
	//---------------------------------------------------------
{
	MODCHANNEL* pChn =& Chn[nChn];
	UINT command = param & 0xF0;
	param &= 0x0F;
	switch (command) {
		// E0x: Set Filter
		// E1x: Fine Portamento Up
	case 0x10:
		if ((param) || (m_nType & (MOD_TYPE_XM | MOD_TYPE_MT2)))
			FinePortamentoUp(pChn, param); break;
		// E2x: Fine Portamento Down
	case 0x20:
		if ((param) || (m_nType & (MOD_TYPE_XM | MOD_TYPE_MT2)))
			FinePortamentoDown(pChn, param); break;
		// E3x: Set Glissando Control
	case 0x30:
		pChn->dwFlags &= ~CHN_GLISSANDO; if (param)
										 	pChn->dwFlags |= CHN_GLISSANDO; break;
		// E4x: Set Vibrato WaveForm
	case 0x40:
		pChn->nVibratoType = param & 0x07; break;
		// E5x: Set FineTune
	case 0x50:
		if (m_nTickCount)
			break;
		pChn->nC4Speed = S3MFineTuneTable[param];
		if (m_nType & (MOD_TYPE_XM | MOD_TYPE_MT2))
			pChn->nFineTune = param * 2;
		else
			pChn->nFineTune = MOD2XMFineTune(param);
		if (pChn->nPeriod)
			pChn->nPeriod = GetPeriodFromNote(pChn->nNote,
								pChn->nFineTune,
								pChn->nC4Speed);
		break;
		// E6x: Pattern Loop
		// E7x: Set Tremolo WaveForm
	case 0x70:
		pChn->nTremoloType = param & 0x07; break;
		// E8x: Set 4-bit Panning
	case 0x80:
		if (!m_nTickCount) {
			pChn->nPan = (param << 4) + 8; pChn->dwFlags |= CHN_FASTVOLRAMP;
		} break;
		// E9x: Retrig
	case 0x90:
		RetrigNote(nChn, param); break;
		// EAx: Fine Volume Up
	case 0xA0:
		if ((param) || (m_nType & (MOD_TYPE_XM | MOD_TYPE_MT2)))
			FineVolumeUp(pChn, param); break;
		// EBx: Fine Volume Down
	case 0xB0:
		if ((param) || (m_nType & (MOD_TYPE_XM | MOD_TYPE_MT2)))
			FineVolumeDown(pChn, param); break;
		// ECx: Note Cut
	case 0xC0:
		NoteCut(nChn, param); break;
		// EDx: Note Delay
		// EEx: Pattern Delay
		// EFx: MOD: Invert Loop, XM: Set Active Midi Macro
	case 0xF0:
		pChn->nActiveMacro = param;	break;
	}
}


void CDecompressTrack::ExtendedS3MCommands(UINT nChn, UINT param)
	//---------------------------------------------------------
{
	MODCHANNEL* pChn =& Chn[nChn];
	UINT command = param & 0xF0;
	param &= 0x0F;
	switch (command) {
		// S0x: Set Filter
		// S1x: Set Glissando Control
	case 0x10:
		pChn->dwFlags &= ~CHN_GLISSANDO; if (param)
										 	pChn->dwFlags |= CHN_GLISSANDO; break;
		// S2x: Set FineTune
	case 0x20:
		if (m_nTickCount)
			break;
		pChn->nC4Speed = S3MFineTuneTable[param & 0x0F];
		pChn->nFineTune = MOD2XMFineTune(param);
		if (pChn->nPeriod)
			pChn->nPeriod = GetPeriodFromNote(pChn->nNote,
								pChn->nFineTune,
								pChn->nC4Speed);
		break;
		// S3x: Set Vibrato WaveForm
	case 0x30:
		pChn->nVibratoType = param & 0x07; break;
		// S4x: Set Tremolo WaveForm
	case 0x40:
		pChn->nTremoloType = param & 0x07; break;
		// S5x: Set Panbrello WaveForm
	case 0x50:
		pChn->nPanbrelloType = param & 0x07; break;
		// S6x: Pattern Delay for x frames
	case 0x60:
		m_nFrameDelay = param; break;
		// S7x: Envelope Control
	case 0x70:
		if (m_nTickCount)
			break;
		switch (param) {
		case 0:
		case 1:
		case 2:
			 {
				MODCHANNEL* bkp =& Chn[m_nChannels];
				for (UINT i = m_nChannels; i < MAX_CHANNELS; i++, bkp++) {
					if (bkp->nMasterChn == nChn + 1) {
						if (param == 1)
							KeyOff(i);
						else if (param == 2)
							bkp->dwFlags |= CHN_NOTEFADE;
						else {
							bkp->dwFlags |= CHN_NOTEFADE; bkp->nFadeOutVol = 0;
						}
					}
				}
			}
			break;
		case 3:
			pChn->nNNA = NNA_NOTECUT; break;
		case 4:
			pChn->nNNA = NNA_CONTINUE; break;
		case 5:
			pChn->nNNA = NNA_NOTEOFF; break;
		case 6:
			pChn->nNNA = NNA_NOTEFADE; break;
		case 7:
			pChn->dwFlags &= ~CHN_VOLENV; break;
		case 8:
			pChn->dwFlags |= CHN_VOLENV; break;
		case 9:
			pChn->dwFlags &= ~CHN_PANENV; break;
		case 10:
			pChn->dwFlags |= CHN_PANENV; break;
		case 11:
			pChn->dwFlags &= ~CHN_PITCHENV; break;
		case 12:
			pChn->dwFlags |= CHN_PITCHENV; break;
		}
		break;
		// S8x: Set 4-bit Panning
	case 0x80:
		if (!m_nTickCount) {
			pChn->nPan = (param << 4) + 8; pChn->dwFlags |= CHN_FASTVOLRAMP;
		} break;
		// S9x: Set Surround
	case 0x90:
		ExtendedChannelEffect(pChn, param & 0x0F); break;
		// SAx: Set 64k Offset
	case 0xA0:
		if (!m_nTickCount) {
			pChn->nOldHiOffset = param;
			if ((pChn->nRowNote) && (pChn->nRowNote < 0x80)) {
				DWORD pos = param << 16;
				if (pos < pChn->nLength)
					pChn->nPos = pos;
			}
		}
		break;
		// SBx: Pattern Loop
		// SCx: Note Cut
	case 0xC0:
		NoteCut(nChn, param); break;
		// SDx: Note Delay
		// case 0xD0:	break;
		// SEx: Pattern Delay for x rows
		// SFx: S3M: Funk Repeat, IT: Set Active Midi Macro
	case 0xF0:
		pChn->nActiveMacro = param; break;
	}
}


void CDecompressTrack::ExtendedChannelEffect(MODCHANNEL* pChn, UINT param)
	//------------------------------------------------------------------
{
	// S9x and X9x commands (S3M/XM/IT only)
	if (m_nTickCount)
		return;
	switch (param & 0x0F) {
		// S90: Surround Off
	case 0x00:
		pChn->dwFlags &= ~CHN_SURROUND;	break;
		// S91: Surround On
	case 0x01:
		pChn->dwFlags |= CHN_SURROUND; pChn->nPan = 128; break;
		////////////////////////////////////////////////////////////
		// Modplug Extensions
		// S98: Reverb Off
	case 0x08:
		pChn->dwFlags &= ~CHN_REVERB;
		pChn->dwFlags |= CHN_NOREVERB;
		break;
		// S99: Reverb On
	case 0x09:
		pChn->dwFlags &= ~CHN_NOREVERB;
		pChn->dwFlags |= CHN_REVERB;
		break;
		// S9A: 2-Channels surround mode
	case 0x0A:
		m_dwSongFlags &= ~SONG_SURROUNDPAN;
		break;
		// S9B: 4-Channels surround mode
	case 0x0B:
		m_dwSongFlags |= SONG_SURROUNDPAN;
		break;
		// S9C: IT Filter Mode
	case 0x0C:
		m_dwSongFlags &= ~SONG_MPTFILTERMODE;
		break;
		// S9D: MPT Filter Mode
	case 0x0D:
		m_dwSongFlags |= SONG_MPTFILTERMODE;
		break;
		// S9E: Go forward
	case 0x0E:
		pChn->dwFlags &= ~ (CHN_PINGPONGFLAG);
		break;
		// S9F: Go backward (set position at the end for non-looping samples)
	case 0x0F:
		if ((!(pChn->dwFlags & CHN_LOOP)) && (!pChn->nPos) && (pChn->nLength)) {
			pChn->nPos = pChn->nLength - 1;
			pChn->nPosLo = 0xFFFF;
		}
		pChn->dwFlags |= CHN_PINGPONGFLAG;
		break;
	}
}


void CDecompressTrack::ProcessMidiMacro(UINT nChn, LPCSTR pszMidiMacro,
	UINT param)
	//---------------------------------------------------------------------------
{
	MODCHANNEL* pChn =& Chn[nChn];
	DWORD dwMacro = (*((LPDWORD) pszMidiMacro)) & 0x7F5F7F5F;
	// Not Internal Device ?
	if (dwMacro != 0x30463046) {
		UINT pos = 0, nNib = 0, nBytes = 0;
		DWORD dwMidiCode = 0, dwByteCode = 0;
		while (pos + 6 <= 32) {
			CHAR cData = pszMidiMacro[pos++];
			if (!cData)
				break;
			if ((cData >= '0') && (cData <= '9')) {
				dwByteCode = (dwByteCode << 4) | (cData - '0'); nNib++;
			} else if ((cData >= 'A') && (cData <= 'F')) {
				dwByteCode = (dwByteCode << 4) | (cData - 'A' + 10); nNib++;
			} else if ((cData >= 'a') && (cData <= 'f')) {
				dwByteCode = (dwByteCode << 4) | (cData - 'a' + 10); nNib++;
			} else if ((cData == 'z') || (cData == 'Z')) {
				dwByteCode = param & 0x7f; nNib = 2;
			} else if ((cData == 'x') || (cData == 'X')) {
				dwByteCode = param & 0x70; nNib = 2;
			} else if ((cData == 'y') || (cData == 'Y')) {
				dwByteCode = (param & 0x0f) << 3; nNib = 2;
			} else if (nNib >= 2) {
				   	nNib = 0;
				   	dwMidiCode |= dwByteCode << (nBytes * 8);
				   	dwByteCode = 0;
				   	nBytes++;
				if (nBytes >= 3) {
					UINT nMasterCh = (nChn < m_nChannels) ?
						nChn +
						1 :
						pChn->nMasterChn;
					if ((nMasterCh) && (nMasterCh <= m_nChannels)) {
						UINT nPlug = ChnSettings[nMasterCh - 1].nMixPlugin;
						if ((nPlug) && (nPlug <= MAX_MIXPLUGINS)) {
							IMixPlugin* pPlugin = m_MixPlugins[nPlug - 1].pMixPlugin;
							if ((pPlugin) &&
								(m_MixPlugins[nPlug - 1].pMixState)) {
								pPlugin->MidiSend(dwMidiCode);
							}
						}
					}
					nBytes = 0;
					dwMidiCode = 0;
				}
			}
		}
		return;
	}
	// Internal device
	pszMidiMacro += 4;
	// Filter ?
	if (pszMidiMacro[0] == '0') {
		CHAR cData1 = pszMidiMacro[2];
		DWORD dwParam = 0;
		if ((cData1 == 'z') || (cData1 == 'Z')) {
			dwParam = param;
		} else {
			CHAR cData2 = pszMidiMacro[3];
			if ((cData1 >= '0') && (cData1 <= '9'))
				dwParam += (cData1 - '0') << 4;
			else if ((cData1 >= 'A') && (cData1 <= 'F'))
				dwParam += (cData1 - 'A' + 0x0A) << 4;
			if ((cData2 >= '0') && (cData2 <= '9'))
				dwParam += (cData2 - '0');
			else if ((cData2 >= 'A') && (cData2 <= 'F'))
				dwParam += (cData2 - 'A' + 0x0A);
		}
		switch (pszMidiMacro[1]) {
			// F0.F0.00.xx: Set CutOff
		case '0':
			 {
				int oldcutoff = pChn->nCutOff;
				if (dwParam < 0x80)
					pChn->nCutOff = dwParam;
			}
			break;

			// F0.F0.01.xx: Set Resonance
		case '1':
			if (dwParam < 0x80)
				pChn->nResonance = dwParam;
			break;
		}
	}
}


void CDecompressTrack::RetrigNote(UINT nChn, UINT param)
	//------------------------------------------------
{
	// Retrig: bit 8 is set if it's the new XM retrig
	MODCHANNEL* pChn =& Chn[nChn];
	UINT nRetrigSpeed = param & 0x0F;
	UINT nRetrigCount = pChn->nRetrigCount;
	BOOL bDoRetrig = FALSE;

	if (m_nType & (MOD_TYPE_S3M | MOD_TYPE_IT)) {
		if (!nRetrigSpeed)
			nRetrigSpeed = 1;
		if ((nRetrigCount) && (!(nRetrigCount % nRetrigSpeed)))
			bDoRetrig = TRUE;
		nRetrigCount++;
	} else {
		UINT realspeed = nRetrigSpeed;
		if ((param & 0x100) &&
			(pChn->nRowVolCmd == VOLCMD_VOLUME) &&
			(pChn->nRowParam & 0xF0))
			realspeed++;
		if ((m_nTickCount) || (param & 0x100)) {
			if (!realspeed)
				realspeed = 1;
			if ((!(param & 0x100)) &&
				(m_nMusicSpeed) &&
				(!(m_nTickCount % realspeed)))
				bDoRetrig = TRUE;
			nRetrigCount++;
		} else if (m_nType & (MOD_TYPE_XM | MOD_TYPE_MT2))
			nRetrigCount = 0;
		if (nRetrigCount >= realspeed) {
			if ((m_nTickCount) || ((param & 0x100) && (!pChn->nRowNote)))
				bDoRetrig = TRUE;
		}
	}
	if (bDoRetrig) {
		UINT dv = (param >> 4) & 0x0F;
		if (dv) {
			int vol = pChn->nVolume;
			if (retrigTable1[dv])
				vol = (vol * retrigTable1[dv]) >> 4;
			else
				vol += ((int) retrigTable2[dv]) << 2;
			if (vol < 0)
				vol = 0;
			if (vol > 256)
				vol = 256;
			pChn->nVolume = vol;
			pChn->dwFlags |= CHN_FASTVOLRAMP;
		}
		UINT nNote = pChn->nNewNote;
		LONG nOldPeriod = pChn->nPeriod;
		if ((nNote) && (nNote <= 120) && (pChn->nLength))
			CheckNNA(nChn, 0, nNote, TRUE);
		BOOL bResetEnv = FALSE;
		if (m_nType & (MOD_TYPE_XM | MOD_TYPE_MT2)) {
			if ((pChn->nRowInstr) && (param < 0x100)) {
				InstrumentChange(pChn, pChn->nRowInstr, FALSE, FALSE); bResetEnv = TRUE;
			}
			if (param < 0x100)
				bResetEnv = TRUE;
		}
		NoteChange(nChn, nNote, FALSE, bResetEnv);
		if ((m_nType & MOD_TYPE_IT) && (!pChn->nRowNote) && (nOldPeriod))
			pChn->nPeriod = nOldPeriod;
		if (!(m_nType & (MOD_TYPE_S3M | MOD_TYPE_IT)))
			nRetrigCount = 0;
	}
	pChn->nRetrigCount = (BYTE) nRetrigCount;
}


void CDecompressTrack::DoFreqSlide(MODCHANNEL* pChn, LONG nFreqSlide)
	//-------------------------------------------------------------
{
	// IT Linear slides
	if (!pChn->nPeriod)
		return;
	if ((m_dwSongFlags & SONG_LINEARSLIDES) &&
		(!(m_nType & (MOD_TYPE_XM | MOD_TYPE_MT2)))) {
		if (nFreqSlide < 0) {
			UINT n = (- nFreqSlide) >> 2;
			if (n > 255)
				n = 255;
			pChn->nPeriod = _muldivr(pChn->nPeriod,
								LinearSlideDownTable[n],
								65536);
		} else {
			UINT n = (nFreqSlide) >> 2;

			if (n > 255)
				n = 255;
			pChn->nPeriod = _muldivr(pChn->nPeriod,
								LinearSlideUpTable[n],
								65536);
		}
	} else {
		pChn->nPeriod += nFreqSlide;
	}
	if (pChn->nPeriod < 1) {
		pChn->nPeriod = 1;
		if (m_nType & MOD_TYPE_IT) {
			pChn->dwFlags |= CHN_NOTEFADE;
			pChn->nFadeOutVol = 0;
		}
	}
}


void CDecompressTrack::NoteCut(UINT nChn, UINT nTick)
	//---------------------------------------------
{
	if (m_nTickCount == nTick) {
		MODCHANNEL* pChn =& Chn[nChn];
		// if (m_nInstruments) KeyOff(pChn); ?
		pChn->nVolume = 0;
		pChn->dwFlags |= CHN_FASTVOLRAMP;
	}
}


void CDecompressTrack::KeyOff(UINT nChn)
	//--------------------------------
{
	MODCHANNEL* pChn =& Chn[nChn];
	BOOL bKeyOn = (pChn->dwFlags& CHN_KEYOFF) ? FALSE : TRUE;
	pChn->dwFlags |= CHN_KEYOFF;
	//if ((!pChn->pHeader) || (!(pChn->dwFlags & CHN_VOLENV)))
	if ((pChn->pHeader) && (!(pChn->dwFlags & CHN_VOLENV))) {
		pChn->dwFlags |= CHN_NOTEFADE;
	}
	if (!pChn->nLength)
		return;
	if ((pChn->dwFlags & CHN_SUSTAINLOOP) && (pChn->pInstrument) && (bKeyOn)) {
		MODINSTRUMENT* psmp = pChn->pInstrument;
		if (psmp->uFlags & CHN_LOOP) {
			if (psmp->uFlags & CHN_PINGPONGLOOP)
				pChn->dwFlags |= CHN_PINGPONGLOOP;
			else
				pChn->dwFlags &= ~ (CHN_PINGPONGLOOP | CHN_PINGPONGFLAG);
			pChn->dwFlags |= CHN_LOOP;
			pChn->nLength = psmp->nLength;
			pChn->nLoopStart = psmp->nLoopStart;
			pChn->nLoopEnd = psmp->nLoopEnd;
			if (pChn->nLength > pChn->nLoopEnd)
				pChn->nLength = pChn->nLoopEnd;
		} else {
			pChn->dwFlags &= ~ (CHN_LOOP | CHN_PINGPONGLOOP | CHN_PINGPONGFLAG);
			pChn->nLength = psmp->nLength;
		}
	}
	if (pChn->pHeader) {
		INSTRUMENTHEADER* penv = pChn->pHeader;
		if (((penv->dwFlags & ENV_VOLLOOP) ||
			(m_nType & (MOD_TYPE_XM | MOD_TYPE_MT2))) &&
			(penv->nFadeOut))
			pChn->dwFlags |= CHN_NOTEFADE;
	}
}


//////////////////////////////////////////////////////////
// CDecompressTrack: Global Effects


void CDecompressTrack::SetSpeed(UINT param)
	//-----------------------------------
{
	UINT max = (m_nType == MOD_TYPE_IT) ? 256 : 128;
	// Modplug Tracker and Mod-Plugin don't do this check
#ifndef MODPLUG_TRACKER
#endif // MODPLUG_TRACKER
	if ((m_nType & MOD_TYPE_S3M) && (param > 0x80))
		param -= 0x80;
	if ((param) && (param <= max))
		m_nMusicSpeed = param;
}


void CDecompressTrack::SetTempo(UINT param)
	//-----------------------------------
{
	if (param < 0x20) {
		// Tempo Slide
		if ((param & 0xF0) == 0x10) {
			m_nMusicTempo += (param & 0x0F) * 2;
			if (m_nMusicTempo > 255)
				m_nMusicTempo = 255;
		} else {
			m_nMusicTempo -= (param & 0x0F) * 2;
			if ((LONG) m_nMusicTempo < 32)
				m_nMusicTempo = 32;
		}
	} else {
		m_nMusicTempo = param;
	}
}


int CDecompressTrack::PatternLoop(MODCHANNEL* pChn, UINT param)
	//-------------------------------------------------------
{
	if (param) {
		if (pChn->nPatternLoopCount) {
			pChn->nPatternLoopCount--;
			if (!pChn->nPatternLoopCount)
				return -1;
		} else {
			MODCHANNEL* p = Chn;
			for (UINT i = 0; i < m_nChannels; i++, p++)
				if (p != pChn) {
					// Loop already done
					if (p->nPatternLoopCount)
						return -1;
				}
			pChn->nPatternLoopCount = param;
		}
		return pChn->nPatternLoop;
	} else {
		pChn->nPatternLoop = m_nRow;
	}
	return -1;
}


void CDecompressTrack::GlobalVolSlide(UINT param)
	//-----------------------------------------
{
	LONG nGlbSlide = 0;
	if (param)
		m_nOldGlbVolSlide = param;
	else
		param = m_nOldGlbVolSlide;
	if (((param & 0x0F) == 0x0F) && (param & 0xF0)) {
		if (m_dwSongFlags & SONG_FIRSTTICK)
			nGlbSlide = (param >> 4) * 2;
	} else if (((param & 0xF0) == 0xF0) && (param & 0x0F)) {
		if (m_dwSongFlags & SONG_FIRSTTICK)
			nGlbSlide = - (int) ((param & 0x0F) * 2);
	} else {
		if (!(m_dwSongFlags & SONG_FIRSTTICK)) {
			if (param & 0xF0)
				nGlbSlide = (int) ((param & 0xF0) >> 4) * 2;
			else
				nGlbSlide = -(int) ((param & 0x0F) * 2);
		}
	}
	if (nGlbSlide) {
		if (m_nType != MOD_TYPE_IT)
			nGlbSlide *= 2;
		nGlbSlide += m_nGlobalVolume;
		if (nGlbSlide < 0)
			nGlbSlide = 0;
		if (nGlbSlide > 256)
			nGlbSlide = 256;
		m_nGlobalVolume = nGlbSlide;
	}
}


DWORD CDecompressTrack::IsSongFinished(UINT nStartOrder, UINT nStartRow) const
	//----------------------------------------------------------------------
{
	UINT nOrd;

	for (nOrd = nStartOrder; nOrd < MAX_ORDERS; nOrd++) {
		UINT nPat = Order[nOrd];
		if (nPat != 0xFE) {
			MODCOMMAND* p;

			if (nPat >= MAX_PATTERNS)
				break;
			p = Patterns[nPat];
			if (p) {
				UINT len = PatternSize[nPat] * m_nChannels;
				UINT pos = (nOrd == nStartOrder) ? nStartRow : 0;
				pos *= m_nChannels;
				while (pos < len) {
					UINT cmd;
					if ((p[pos].note) || (p[pos].volcmd))
						return 0;
					cmd = p[pos].command;
					if (cmd == CMD_MODCMDEX) {
						UINT cmdex = p[pos].param & 0xF0;
						if ((!cmdex) ||
							(cmdex == 0x60) ||
							(cmdex == 0xE0) ||
							(cmdex == 0xF0))
							cmd = 0;
					}
					if ((cmd) && (cmd != CMD_SPEED) && (cmd != CMD_TEMPO))
						return 0;
					pos++;
				}
			}
		}
	}
	return (nOrd < MAX_ORDERS) ? nOrd : MAX_ORDERS - 1;
}


BOOL CDecompressTrack::IsValidBackwardJump(UINT nStartOrder, UINT nStartRow,
	UINT nJumpOrder, UINT nJumpRow) const
	//----------------------------------------------------------------------------------------------------------
{
	while ((nJumpOrder < MAX_PATTERNS) && (Order[nJumpOrder] == 0xFE))
		nJumpOrder++;
	if ((nStartOrder >= MAX_PATTERNS) || (nJumpOrder >= MAX_PATTERNS))
		return FALSE;
	// Treat only case with jumps in the same pattern
	if (nJumpOrder > nStartOrder)
		return TRUE;
	if ((nJumpOrder < nStartOrder) ||
		(nJumpRow >= PatternSize[nStartOrder]) ||
		(!Patterns[nStartOrder]) ||
		(nStartRow >= 256) ||
		(nJumpRow >= 256))
		return FALSE;
	// See if the pattern is being played backward
	BYTE row_hist[256];
	memset(row_hist, 0, sizeof(row_hist));
	UINT nRows = PatternSize[nStartOrder], row = nJumpRow;
	if (nRows > 256)
		nRows = 256;
	row_hist[nStartRow] = TRUE;
	while ((row < 256) && (!row_hist[row])) {
		if (row >= nRows)
			return TRUE;
		row_hist[row] = TRUE;
		MODCOMMAND* p = Patterns[nStartOrder] + row* m_nChannels;
		row++;
		int breakrow = - 1, posjump = 0;
		for (UINT i = 0; i < m_nChannels; i++, p++) {
			if (p->command == CMD_POSITIONJUMP) {
				if (p->param < nStartOrder)
					return FALSE;
				if (p->param > nStartOrder)
					return TRUE;
				posjump = TRUE;
			} else if (p->command == CMD_PATTERNBREAK) {
				breakrow = p->param;
			}
		}
		if (breakrow >= 0) {
			if (!posjump)
				return TRUE;
			row = breakrow;
		}
		if (row >= nRows)
			return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////
// Note/Period/Frequency functions

UINT CDecompressTrack::GetNoteFromPeriod(UINT period) const
	//---------------------------------------------------
{
	if (!period)
		return 0;
	if (m_nType & (MOD_TYPE_MED |
		MOD_TYPE_MOD |
		MOD_TYPE_MTM |
		MOD_TYPE_669 |
		MOD_TYPE_OKT |
		MOD_TYPE_AMF0)) {
		period >>= 2;
		for (UINT i = 0; i < 6 * 12; i++) {
			if (period >= ProTrackerPeriodTable[i]) {
				if ((period != ProTrackerPeriodTable[i]) && (i)) {
					UINT p1 = ProTrackerPeriodTable[i - 1];
					UINT p2 = ProTrackerPeriodTable[i];
					if (p1 - period < (period - p2))
						return i + 36;
				}
				return i + 1 + 36;
			}
		}
		return 6 * 12 + 36;
	} else {
		for (UINT i = 1; i < 120; i++) {
			LONG n = GetPeriodFromNote(i, 0, 0);
			if ((n > 0) && (n <= (LONG) period))
				return i;
		}
		return 120;
	}
}



UINT CDecompressTrack::GetPeriodFromNote(UINT note, int nFineTune,
	UINT nC4Speed) const
	//-------------------------------------------------------------------------------
{
	if ((!note) || (note > 0xF0))
		return 0;
	if (m_nType & (MOD_TYPE_IT |
		MOD_TYPE_S3M |
		MOD_TYPE_STM |
		MOD_TYPE_MDL |
		MOD_TYPE_ULT |
		MOD_TYPE_WAV |
		MOD_TYPE_FAR |
		MOD_TYPE_DMF |
		MOD_TYPE_PTM |
		MOD_TYPE_AMS |
		MOD_TYPE_DBM |
		MOD_TYPE_AMF |
		MOD_TYPE_PSM)) {
		note--;
		if (m_dwSongFlags & SONG_LINEARSLIDES) {
			return (FreqS3MTable[note % 12] << 5) >> (note / 12);
		} else {
			if (!nC4Speed)
				nC4Speed = 8363;
			return _muldiv(8363,
				   	(FreqS3MTable[note % 12] << 5),
				   	nC4Speed << (note / 12));
		}
	} else if (m_nType & (MOD_TYPE_XM | MOD_TYPE_MT2)) {
		if (note < 13)
			note = 13;
		note -= 13;
		if (m_dwSongFlags & SONG_LINEARSLIDES) {
			LONG l = ((120 - note) << 6) - (nFineTune / 2);
			if (l < 1)
				l = 1;
			return (UINT) l;
		} else {
			int finetune = nFineTune;
			UINT rnote = (note % 12) << 3;
			UINT roct = note / 12;
			int rfine = finetune / 16;
			int i = rnote + rfine + 8;
			if (i < 0)
				i = 0;
			if (i >= 104)
				i = 103;
			UINT per1 = XMPeriodTable[i];
			if (finetune < 0) {
				rfine--;
				finetune = -finetune;
			} else
				rfine++;
			i = rnote + rfine + 8;
			if (i < 0)
				i = 0;
			if (i >= 104)
				i = 103;
			UINT per2 = XMPeriodTable[i];
			rfine = finetune & 0x0F;
			per1 *= 16 - rfine;
			per2 *= rfine;
			return ((per1 + per2) << 1) >> roct;
		}
	} else {
		note--;
		nFineTune = XM2MODFineTune(nFineTune);
		if ((nFineTune) || (note < 36) || (note >= 36 + 6 * 12))
			return (ProTrackerTunedPeriods[nFineTune * 12 + note % 12] << 5) >>
				(note / 12);
		else
			return (ProTrackerPeriodTable[note - 36] << 2);
	}
}


UINT CDecompressTrack::GetFreqFromPeriod(UINT period, UINT nC4Speed,
	int nPeriodFrac) const
	//-----------------------------------------------------------------------------------
{
	if (!period)
		return 0;
	if (m_nType & (MOD_TYPE_MED |
		MOD_TYPE_MOD |
		MOD_TYPE_MTM |
		MOD_TYPE_669 |
		MOD_TYPE_OKT |
		MOD_TYPE_AMF0)) {
		return (3546895L * 4) / period;
	} else if (m_nType & (MOD_TYPE_XM | MOD_TYPE_MT2)) {
		if (m_dwSongFlags & SONG_LINEARSLIDES)
			return XMLinearTable[period % 768] >> (period / 768);
		else
			return 8363 * 1712L / period;
	} else {
		if (m_dwSongFlags & SONG_LINEARSLIDES) {
			if (!nC4Speed)
				nC4Speed = 8363;
			return _muldiv(nC4Speed, 1712L << 8, (period << 8) + nPeriodFrac);
		} else {
			return _muldiv(8363, 1712L << 8, (period << 8) + nPeriodFrac);
		}
	}
}


