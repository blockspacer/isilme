//-----------------------------------------------------------------------------
// ������ � EAX �����������
// ��������� ��������� ��������� "�����"
// �������     : AntiTank
// ����������� : ������������ ����� (�����)
//-----------------------------------------------------------------------------

// ���������
#include "Eax.h"
#include "Eax1_api.h"
#include "Eax2_api.h"
#include "Eax3_api.h"

// ������� � ��������� ��������� ��� ��������� ���������� EAX ���������
int EAX_StructSize[2][EAX_NUM + 1] = {
	// ���������
	{
	0,  								   // ������ ��������
	sizeof(EAX1_LISTENER_PROPERTIES),      // EAX 1.0
	sizeof(EAX2_LISTENER_PROPERTIES),      // EAX 2.0
	sizeof(EAX3_LISTENER_PROPERTIES),      // EAX 3.0
	},
	// �����
	{
	0,  								   // ������ ��������
	sizeof(EAX1_BUFFER_PROPERTIES), 	   // EAX 1.0
	sizeof(EAX2_BUFFER_PROPERTIES), 	   // EAX 2.0
	sizeof(EAX3_BUFFER_PROPERTIES), 	   // EAX 3.0
	}
};

// ������� � ������� ��� ��������� ���� ���������� EAX ���������
int EAX_Param[2][EAX_NUM + 1] = {
	// ���������
	{ - 1,  								  // ������ ��������
	DSPROPERTY_EAX1_LISTENER_ALLPARAMETERS,// EAX 1.0
	DSPROPERTY_EAX2_LISTENER_ALLPARAMETERS,// EAX 2.0
	DSPROPERTY_EAX3_LISTENER_ALLPARAMETERS,// EAX 3.0
	},
	// �����
	{ - 1,  								  // ������ ��������
	DSPROPERTY_EAX1_BUFFER_ALLPARAMETERS,  // EAX 1.0
	DSPROPERTY_EAX2_BUFFER_ALLPARAMETERS,  // EAX 2.0
	DSPROPERTY_EAX3_BUFFER_ALLPARAMETERS,  // EAX 3.0
	}
};

// ������� ��������������� EAX ����������� ���������
GUID EAX_Guid[2][EAX_NUM + 1] = {
	// ���������
	{
	// ������ ��������
	{ 0x00000000, 0x0000, 0x0000, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
	// EAX 1.0
	{ 0x4a4e6fc1, 0xc341, 0x11d1, { 0xb7, 0x3a, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 } },
	// EAX 2.0
	{ 0x0306a6a8, 0xb224, 0x11d2, { 0x99, 0xe5, 0x00, 0x00, 0xe8, 0xd8, 0xc7, 0x22 } },
	// EAX 3.0
	{ 0xA8FA6882, 0xB476, 0x11D3, { 0xBD, 0xB9, 0x00, 0xC0, 0xF0, 0x2D, 0xDF, 0x87 } },
	},
	// �����
	{
	// ������ ��������
	{ 0x00000000, 0x0000, 0x0000, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
	// EAX 1.0
	{ 0x4a4e6fc0, 0xc341, 0x11d1, { 0xb7, 0x3a, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 } },
	// EAX 2.0
	{ 0x0306a6a7, 0xb224, 0x11d2, { 0x99, 0xe5, 0x00, 0x00, 0xe8, 0xd8, 0xc7, 0x22 } },
	// EAX 3.0
	{ 0xA8FA6881, 0xB476, 0x11D3, { 0xBD, 0xB9, 0x00, 0xC0, 0xF0, 0x2D, 0xDF, 0x87 } },
	}
};

// ����������������� �������� ��� EAX 1.0
EAX1_LISTENER_PROPERTIES EAX1_Preset[27] = {
	{  0, -1.0f, -1.0f, -1.0f     }, {  0,     0.5f,	1.493f,  0.5f	 },
	{  1,     0.25f,   0.1f,	0.0f	 },
	{  2,     0.417f,  0.4f,	0.666f   },
	{  3,     0.653f,  1.499f,  0.166f   },
	{  4,     0.208f,  0.478f,  0.0f	 },
	{  5,     0.5f,    2.309f,  0.888f   },
	{  6,     0.403f,  4.279f,  0.5f	 },
	{  7,     0.5f,    3.961f,  0.5f	 },
	{  8,     0.5f,    2.886f,  1.304f   },
	{  9,     0.361f,  7.284f,  0.332f   },
	{  10,    0.5f,   10.0f,	0.3f	 },
	{  11,	 0.153f,  0.259f,  2.0f 	},
	{  12,	 0.361f,  1.493f,  0.0f 	},
	{  13,	 0.444f,  2.697f,  0.638f   },
	{  14,	 0.25f,   1.752f,  0.776f   },
	{  15,	 0.111f,  3.145f,  0.472f   },
	{  16,	 0.111f,  2.767f,  0.224f   },
	{  17,	 0.194f,  7.841f,  0.472f   },
	{  18,	 1.0f,    1.499f,  0.5f 	},
	{  19,	 0.097f,  2.767f,  0.224f   },
	{  20,	 0.208f,  1.652f,  1.5f 	},
	{  21,	 0.652f,  2.886f,  0.25f	},
	{  22,	 1.0f,    1.499f,  0.0f 	},
	{  23,	 0.875f,  8.392f,  1.388f   },
	{  24,	 0.139f, 17.234f,  0.666f   },
	{  25,	 0.486f,  7.563f,  0.806f   }, 
};

// ����������������� �������� EAX 2.0
EAX2_LISTENER_PROPERTIES EAX2_Preset[27] = {
	//1		2   	3      4	   5	   7	   8	   10     11		13    14	   20    23 	26
	{ - 10000, -10000, 0.0f,  1.00f,  1.00f, -2602, 0.007f,    200, 0.011f,  0,   7.5f, 1.00f, -5.0f, 0x3f },
	{ - 1000, -100, 0.0f,  1.49f,  0.83f, -2602, 0.007f,	200, 0.011f,  0,   7.5f, 1.00f, -5.0f, 0x3f },
	{ - 1000, -6000, 0.0f,  0.17f,  0.10f, -1204, 0.001f,	207, 0.002f,  1,   1.4f, 1.00f, -5.0f, 0x3f },
	{ - 1000, -454, 0.0f,  0.40f,  0.83f, -1646, 0.002f,	 53, 0.003f,  2,   1.9f, 1.00f, -5.0f, 0x3f },
	{ - 1000, -1200, 0.0f,  1.49f,  0.54f, -370, 0.007f,   1030, 0.011f,  3,   1.4f, 1.00f, -5.0f, 0x3f },
	{ - 1000, -6000, 0.0f,  0.50f,  0.10f, -1376, 0.003f, -1104, 0.004f,  4,   2.5f, 1.00f, -5.0f, 0x3f },
	{ - 1000, -300, 0.0f,  2.31f,  0.64f, -711, 0.012f, 	83, 0.017f,  5,  11.6f, 1.00f, -5.0f, 0x3f },
	{ - 1000, -476, 0.0f,  4.32f,  0.59f, -789, 0.020f, -289, 0.030f,  6,  21.6f, 1.00f, -5.0f, 0x3f },
	{ - 1000, -500, 0.0f,  3.92f,  0.70f, -1230, 0.020f, -2, 0.029f,  7,  19.6f, 1.00f, -5.0f, 0x3f },
	{ - 1000,   	0, 0.0f,  2.91f,  1.30f, -602, 0.015f, -302, 0.022f,  8,  14.6f, 1.00f, -5.0f, 0x1f },
	{ - 1000, -698, 0.0f,  7.24f,  0.33f, -1166, 0.020f,	 16, 0.030f,  9,  36.2f, 1.00f, -5.0f, 0x3f },
	{ - 1000, -1000, 0.0f, 10.05f,  0.23f, -602, 0.020f,	198, 0.030f, 10,  50.3f, 1.00f, -5.0f, 0x3f },
	{ - 1000, -4000, 0.0f,  0.30f,  0.10f, -1831, 0.002f, -1630, 0.030f, 11,   1.9f, 1.00f, -5.0f, 0x3f },
	{ - 1000, -300, 0.0f,  1.49f,  0.59f, -1219, 0.007f,	441, 0.011f, 12,   1.8f, 1.00f, -5.0f, 0x3f },
	{ - 1000, -237, 0.0f,  2.70f,  0.79f, -1214, 0.013f,	395, 0.020f, 13,  13.5f, 1.00f, -5.0f, 0x3f },
	{ - 1000, -270, 0.0f,  1.49f,  0.86f, -1204, 0.007f, -4, 0.011f, 14,   7.5f, 0.30f, -5.0f, 0x3f },
	{ - 1000, -3300, 0.0f,  1.49f,  0.54f, -2560, 0.162f, -229, 0.088f, 15,  38.0f, 0.30f, -5.0f, 0x3f },
	{ - 1000, -800, 0.0f,  1.49f,  0.67f, -2273, 0.007f, -1691, 0.011f, 16,   7.5f, 0.50f, -5.0f, 0x3f },
	{ - 1000, -2500, 0.0f,  1.49f,  0.21f, -2780, 0.300f, -1434, 0.100f, 17, 100.0f, 0.27f, -5.0f, 0x1f },
	{ - 1000, -1000, 0.0f,  1.49f,  0.83f, -10000, 0.061f,    500, 0.025f, 18,  17.5f, 1.00f, -5.0f, 0x3f },
	{ - 1000, -2000, 0.0f,  1.49f,  0.50f, -2466, 0.179f, -1926, 0.100f, 19,  42.5f, 0.21f, -5.0f, 0x3f },
	{ - 1000,   	0, 0.0f,  1.65f,  1.50f, -1363, 0.008f, -1153, 0.012f, 20,   8.3f, 1.00f, -5.0f, 0x1f },
	{ - 1000, -1000, 0.0f,  2.81f,  0.14f,    429, 0.014f,   1023, 0.021f, 21,   1.7f, 0.80f, -5.0f, 0x3f },
	{ - 1000, -4000, 0.0f,  1.49f,  0.10f, -449, 0.007f,   1700, 0.011f, 22,   1.8f, 1.00f, -5.0f, 0x3f },
	{ - 1000,   	0, 0.0f,  8.39f,  1.39f, -115, 0.002f,    985, 0.030f, 23,   1.9f, 0.50f, -5.0f, 0x1f },
	{ - 1000, -400, 0.0f, 17.23f,  0.56f, -1713, 0.020f, -613, 0.030f, 24,   1.8f, 0.60f, -5.0f, 0x1f },
	{ - 1000, -151, 0.0f,  7.56f,  0.91f, -626, 0.020f,    774, 0.030f, 25,   1.0f, 0.50f, -5.0f, 0x1f }, 
};

// ���������������� �������� ��� EAX 3.0
EAX3_LISTENER_PROPERTIES EAX3_Preset[27] = {
	//	 Env	Size Diffus    Room  RoomHF	RoomLF   DecTm	  DcHF  DcLF	Refl RefDel	Ref Pan				Revb	RevDel		Rev Pan				EchTm	EchDp	ModTm	ModDp	AirAbs	HFRef	LFRef	RRlOff	FLAGS
	{ 0,   7.5f, 1.00f, -10000, -10000, 	0,  1.00f, 1.00f, 1.0f, -2602, 0.007f, { 0.0f,0.0f,0.0f },   200, 0.011f, { 0.0f,0.0f,0.0f },	0.250f, 0.00f,	0.25f,	0.000f, -5.0f, 5000.0f, 250.0f, 0.0f, 0x33f },
	{ 0,   7.5f, 1.00f, -1000, -100,	 0,  1.49f, 0.83f, 1.0f, -2602, 0.007f, { 0.0f,0.0f,0.0f },   200, 0.011f, { 0.0f,0.0f,0.0f },	0.250f, 0.00f,	0.25f,	0.000f, -5.0f, 5000.0f, 250.0f, 0.0f, 0x3f },
	{ 1,   1.4f, 1.00f, -1000, -6000, 	0,  0.17f, 0.10f, 1.0f, -1204, 0.001f, { 0.0f,0.0f,0.0f },   207, 0.002f, { 0.0f,0.0f,0.0f },	0.250f, 0.00f,	0.25f,	0.000f, -5.0f, 5000.0f, 250.0f, 0.0f, 0x3f },
	{ 2,   1.9f, 1.00f, -1000, -454,	 0,  0.40f, 0.83f, 1.0f, -1646, 0.002f, { 0.0f,0.0f,0.0f },    53, 0.003f, { 0.0f,0.0f,0.0f },	0.250f, 0.00f,	0.25f,	0.000f, -5.0f, 5000.0f, 250.0f, 0.0f, 0x3f },
	{ 3,   1.4f, 1.00f, -1000, -1200, 	0,  1.49f, 0.54f, 1.0f, -370, 0.007f, { 0.0f,0.0f,0.0f },  1030, 0.011f, { 0.0f,0.0f,0.0f },	0.250f, 0.00f,	0.25f,	0.000f, -5.0f, 5000.0f, 250.0f, 0.0f, 0x3f },
	{ 4,   2.5f, 1.00f, -1000, -6000, 	0,  0.50f, 0.10f, 1.0f, -1376, 0.003f, { 0.0f,0.0f,0.0f }, -1104, 0.004f, { 0.0f,0.0f,0.0f },	0.250f, 0.00f,	0.25f,	0.000f, -5.0f, 5000.0f, 250.0f, 0.0f, 0x3f },
	{ 5,  11.6f, 1.00f, -1000, -300,	 0,  2.31f, 0.64f, 1.0f, -711, 0.012f, { 0.0f,0.0f,0.0f },    83, 0.017f, { 0.0f,0.0f,0.0f },	0.250f, 0.00f,	0.25f,	0.000f, -5.0f, 5000.0f, 250.0f, 0.0f, 0x3f },
	{ 6,  21.6f, 1.00f, -1000, -476,	 0,  4.32f, 0.59f, 1.0f, -789, 0.020f, { 0.0f,0.0f,0.0f }, -289, 0.030f, { 0.0f,0.0f,0.0f },	0.250f, 0.00f,	0.25f,	0.000f, -5.0f, 5000.0f, 250.0f, 0.0f, 0x3f },
	{ 7,  19.6f, 1.00f, -1000, -500,	 0,  3.92f, 0.70f, 1.0f, -1230, 0.020f, { 0.0f,0.0f,0.0f }, -02, 0.029f, { 0.0f,0.0f,0.0f },	0.250f, 0.00f,	0.25f,	0.000f, -5.0f, 5000.0f, 250.0f, 0.0f, 0x3f },
	{ 8,  14.6f, 1.00f, -1000,  	0,     0,  2.91f, 1.30f, 1.0f, -602, 0.015f, { 0.0f,0.0f,0.0f }, -302, 0.022f, { 0.0f,0.0f,0.0f },	0.250f, 0.00f,	0.25f,	0.000f, -5.0f, 5000.0f, 250.0f, 0.0f, 0x1f },
	{ 9,  36.2f, 1.00f, -1000, -698,	 0,  7.24f, 0.33f, 1.0f, -1166, 0.020f, { 0.0f,0.0f,0.0f },    16, 0.030f, { 0.0f,0.0f,0.0f },	0.250f, 0.00f,	0.25f,	0.000f, -5.0f, 5000.0f, 250.0f, 0.0f, 0x3f },
	{10,  50.3f, 1.00f, -1000, -1000, 	0, 10.05f, 0.23f, 1.0f, -602, 0.020f, { 0.0f,0.0f,0.0f },   198, 0.030f, { 0.0f,0.0f,0.0f },	0.250f, 0.00f,	0.25f,	0.000f, -5.0f, 5000.0f, 250.0f, 0.0f, 0x3f },
	{11,   1.9f, 1.00f, -1000, -4000, 	0,  0.30f, 0.10f, 1.0f, -1831, 0.002f, { 0.0f,0.0f,0.0f }, -1630, 0.030f, { 0.0f,0.0f,0.0f },	0.250f, 0.00f,	0.25f,	0.000f, -5.0f, 5000.0f, 250.0f, 0.0f, 0x3f },
	{12,   1.8f, 1.00f, -1000, -300,	 0,  1.49f, 0.59f, 1.0f, -1219, 0.007f, { 0.0f,0.0f,0.0f },   441, 0.011f, { 0.0f,0.0f,0.0f },	0.250f, 0.00f,	0.25f,	0.000f, -5.0f, 5000.0f, 250.0f, 0.0f, 0x3f },
	{13,  13.5f, 1.00f, -1000, -237,	 0,  2.70f, 0.79f, 1.0f, -1214, 0.013f, { 0.0f,0.0f,0.0f },   395, 0.020f, { 0.0f,0.0f,0.0f },	0.250f, 0.00f,	0.25f,	0.000f, -5.0f, 5000.0f, 250.0f, 0.0f, 0x3f },
	{14,   7.5f, 0.30f, -1000, -270,	 0,  1.49f, 0.86f, 1.0f, -1204, 0.007f, { 0.0f,0.0f,0.0f }, -4, 0.011f, { 0.0f,0.0f,0.0f },	0.125f, 0.95f,	0.25f,	0.000f, -5.0f, 5000.0f, 250.0f, 0.0f, 0x3f },
	{15,  38.0f, 0.30f, -1000, -3300, 	0,  1.49f, 0.54f, 1.0f, -2560, 0.162f, { 0.0f,0.0f,0.0f }, -229, 0.088f, { 0.0f,0.0f,0.0f },	0.125f, 1.00f,	0.25f,	0.000f, -5.0f, 5000.0f, 250.0f, 0.0f, 0x3f },
	{16,   7.5f, 0.50f, -1000, -800,	 0,  1.49f, 0.67f, 1.0f, -2273, 0.007f, { 0.0f,0.0f,0.0f }, -1691, 0.011f, { 0.0f,0.0f,0.0f },	0.250f, 0.00f,	0.25f,	0.000f, -5.0f, 5000.0f, 250.0f, 0.0f, 0x3f },
	{17, 100.0f, 0.27f, -1000, -2500, 	0,  1.49f, 0.21f, 1.0f, -2780, 0.300f, { 0.0f,0.0f,0.0f }, -1434, 0.100f, { 0.0f,0.0f,0.0f },	0.250f, 1.00f,	0.25f,	0.000f, -5.0f, 5000.0f, 250.0f, 0.0f, 0x1f },
	{18,  17.5f, 1.00f, -1000, -1000, 	0,  1.49f, 0.83f, 1.0f, -10000, 0.061f, { 0.0f,0.0f,0.0f },   500, 0.025f, { 0.0f,0.0f,0.0f },	0.125f, 0.70f,	0.25f,	0.000f, -5.0f, 5000.0f, 250.0f, 0.0f, 0x3f },
	{19,  42.5f, 0.21f, -1000, -2000, 	0,  1.49f, 0.50f, 1.0f, -2466, 0.179f, { 0.0f,0.0f,0.0f }, -1926, 0.100f, { 0.0f,0.0f,0.0f },	0.250f, 1.00f,	0.25f,	0.000f, -5.0f, 5000.0f, 250.0f, 0.0f, 0x3f },
	{20,   8.3f, 1.00f, -1000,  	0,     0,  1.65f, 1.50f, 1.0f, -1363, 0.008f, { 0.0f,0.0f,0.0f }, -1153, 0.012f, { 0.0f,0.0f,0.0f },	0.250f, 0.00f,	0.25f,	0.000f, -5.0f, 5000.0f, 250.0f, 0.0f, 0x1f },
	{21,   1.7f, 0.80f, -1000, -1000, 	0,  2.81f, 0.14f, 1.0f,    429, 0.014f, { 0.0f,0.0f,0.0f },  1023, 0.021f, { 0.0f,0.0f,0.0f },	0.250f, 0.00f,	0.25f,	0.000f, -5.0f, 5000.0f, 250.0f, 0.0f, 0x3f },
	{22,   1.8f, 1.00f, -1000, -4000, 	0,  1.49f, 0.10f, 1.0f, -449, 0.007f, { 0.0f,0.0f,0.0f },  1700, 0.011f, { 0.0f,0.0f,0.0f },	0.250f, 0.00f,	1.18f,	0.348f, -5.0f, 5000.0f, 250.0f, 0.0f, 0x3f },
	{23,   1.9f, 0.50f, -1000,  	0,     0,  8.39f, 1.39f, 1.0f, -115, 0.002f, { 0.0f,0.0f,0.0f },   985, 0.030f, { 0.0f,0.0f,0.0f },	0.250f, 0.00f,	0.25f,	1.000f, -5.0f, 5000.0f, 250.0f, 0.0f, 0x1f },
	{24,   1.8f, 0.60f, -1000, -400,	 0, 17.23f, 0.56f, 1.0f, -1713, 0.020f, { 0.0f,0.0f,0.0f }, -613, 0.030f, { 0.0f,0.0f,0.0f },	0.250f, 1.00f,	0.81f,	0.310f, -5.0f, 5000.0f, 250.0f, 0.0f, 0x1f },
	{25,   1.0f, 0.50f, -1000, -151,	 0,  7.56f, 0.91f, 1.0f, -626, 0.020f, { 0.0f,0.0f,0.0f },   774, 0.030f, { 0.0f,0.0f,0.0f },	0.250f, 0.00f,	4.00f,	1.000f, -5.0f, 5000.0f, 250.0f, 0.0f, 0x1f }
};

//-----------------------------------------------------------------------------
// ��������� ���������� ��� ��������� ������
// �� �����    :  buffer - ��������� �� �������� ����� ��� �������� �����
//  					   �������� eax ���������
// �� ������   :  ��������� �� ���������� ���������, � ������ ���� ��������
//  			  ����� 0 ������ ��������� �� ����������.
//-----------------------------------------------------------------------------
LPKSPROPERTYSET eax_GetBufferInterface(LPDIRECTSOUNDBUFFER buffer)
{
	LPKSPROPERTYSET prop;

	// �������� ������� ������
	if (!buffer)
		return 0;

	// ��������� ����������
	if (buffer->QueryInterface(IID_IKsPropertySet, (void * *) &prop) == DS_OK)
		return prop;

	// ������
	return 0;
}

//-----------------------------------------------------------------------------
// ��������� ���������� ��� ����������� ������
// �� �����    :  buffer3d - ��������� �� �������� ����� ��� �������� �����
//  						 �������� eax ���������
// �� ������   :  ��������� �� ���������� ���������, � ������ ���� ��������
//  			  ����� 0 ������ ��������� �� ����������.
//-----------------------------------------------------------------------------
LPKSPROPERTYSET eax_GetBuffer3DInterface(LPDIRECTSOUND3DBUFFER buffer3d)
{
	LPKSPROPERTYSET prop;

	// �������� ������� ������
	if (!buffer3d)
		return 0;

	// ��������� ����������
	if (buffer3d->QueryInterface(IID_IKsPropertySet, (void * *) &prop) ==
		DS_OK)
		return prop;

	// ������
	return 0;
}

//-----------------------------------------------------------------------------
// �������� ����������� ������ � ������
// �� �����    :  prop     - ��������� �� eax ���������
//  			  type     - ��� ���������� (��������� ��� �����)
//  			  version  - ������ ����������
// �� ������   :  ���������� ����������� ������ � ������
//-----------------------------------------------------------------------------
int eax_TestQuerySupport(LPKSPROPERTYSET prop, int type, int version)
{
	unsigned long support;

	// �������� ����������
	if (!prop)
		return false;

	// �������� ��������� ���������
	if (prop->QuerySupport(EAX_Guid[type][version],
			  	EAX_Param[type][version],
			  	& support) == DS_OK) {
		// �������� ����������� ������ � ������
		if ((support & KSPROPERTY_SUPPORT_GET) &&
			(support & KSPROPERTY_SUPPORT_SET))
			return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
// ��������� eax ����������
// �� �����    :  prop     - ��������� �� eax ���������
//  			  type     - ��� ���������� (��������� ��� �����)
//  			  version  - ������ ����������
//  			  data     - ��������� �� ������ ��� ���������
// �� ������   :  ���������� ��������� ����������
//-----------------------------------------------------------------------------
int eax_Set(LPKSPROPERTYSET prop, int type, int version, void* data)
{
	// �������� ����������
	if ((!prop) || (version == 0) || (type > EAX_BUFFER))
		return false;

	// ��������� ����������
	if (prop->Set(EAX_Guid[type][version],
			  	EAX_Param[type][version],
			  	NULL,
			  	0,
			  	data,
			  	EAX_StructSize[type][version]) != DS_OK)
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// �������� eax ����������
// �� �����    :  prop     - ��������� �� eax ���������
//  			  type     - ��� ���������� (��������� ��� �����)
//  			  version  - ������ ����������
//  			  data     - ��������� �� ����� � ������� ����� ���������
//  						 ��������� ������
// �� ������   :  ���������� ��������� ����������
//-----------------------------------------------------------------------------
int eax_Get(LPKSPROPERTYSET prop, int type, int version, void* data)
{
	unsigned long read;
	// �������� ����������
	if ((!prop) || (version == 0) || (type > EAX_BUFFER))
		return false;

	// ��������� ����������
	if (prop->Get(EAX_Guid[type][version],
			  	EAX_Param[type][version],
			  	NULL,
			  	0,
			  	data,
			  	EAX_StructSize[type][version],
			  	& read) != DS_OK)
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// ��������� ������������������ eax ���������
// �� �����    :  prop     - ��������� �� eax ���������
//  			  version  - ������ ����������
//  			  preset   - ����� ����������������� ��������
// �� ������   :  ���������� ����������� ������ � ������
//-----------------------------------------------------------------------------
int eax_Preset(LPKSPROPERTYSET prop, int version, int preset)
{
	switch (version) {
		// EAX 1.0
	case 1:
		return eax_Set(prop, EAX_LISTENER, 1, (void *) &EAX1_Preset[preset]);
		// EAX 2.0
	case 2:
		return eax_Set(prop, EAX_LISTENER, 2, (void *) &EAX2_Preset[preset]);
		// EAX 3.0
	case 3:
		return eax_Set(prop, EAX_LISTENER, 3, (void *) &EAX3_Preset[preset]);
	}
	return false;
}

//-----------------------------------------------------------------------------
// ��������� ������������������ eax ���������
// �� �����    :  direct   - ��������� �� ������ ���������������
//  			  version  - ������ ����������
// �� ������   :  ���������� �������� ���������
//-----------------------------------------------------------------------------
int eax_TestSupport(LPDIRECTSOUND direct, int version)
{
	// ���������� ����������
	WAVEFORMATEX wfx;   		   // ������ �����
	DSBUFFERDESC dsbd;  		   // ����������
	int ok; 			  // ���� ������������� EAX ����������
	LPDIRECTSOUNDBUFFER Buffer;
	LPDIRECTSOUND3DBUFFER Buffer3D;
	LPKSPROPERTYSET ListenerPropSet;
	LPKSPROPERTYSET BufferPropSet;

	// �������� ��������� � ��������
	wfx.wFormatTag = 1;
	wfx.nChannels = 2;
	wfx.nSamplesPerSec = 44100;
	wfx.wBitsPerSample = 16;
	wfx.nBlockAlign = (wfx.nChannels * wfx.wBitsPerSample) >> 3;
	wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;
	wfx.cbSize = 0;

	// ������� ���������
	ok = false;
	Buffer = 0;
	Buffer3D = 0;
	ListenerPropSet = 0;
	BufferPropSet = 0;

	// �������� ������� ������� ���������������
	if (direct) {
		// ��������� ������ ���������� ������
		ZeroMemory(&dsbd, sizeof(DSBUFFERDESC));
		dsbd.dwSize = sizeof(DSBUFFERDESC);
		dsbd.dwFlags = DSBCAPS_STATIC | DSBCAPS_CTRL3D;
		dsbd.dwBufferBytes = 128;
		dsbd.lpwfxFormat = &wfx;

		// �������� ���������� ������
		ok = (direct->CreateSoundBuffer(&dsbd, & Buffer, NULL) == DS_OK) ?
			true :
			false;

		// ������� ��������� ��� ������ � EAX ����������
		if (ok)
			ok = (Buffer->QueryInterface(IID_IKsPropertySet,
						  	(void * *) &ListenerPropSet) == DS_OK) ?
				true :
				false;

		// �������� ���������� ��� ����������� �����
		if (ok)
			ok = (Buffer->QueryInterface(IID_IDirectSound3DBuffer,
						  	(VOID * *) &Buffer3D) == DS_OK) ?
				true :
				false;

		// ��������� �����������
		ListenerPropSet = eax_GetBufferInterface(Buffer);
		BufferPropSet = eax_GetBuffer3DInterface(Buffer3D);

		// �������� ����������� ������ � ������ ���������
		if (ok)
			ok = eax_TestQuerySupport(ListenerPropSet, EAX_LISTENER, version);

		if (ok)
			ok = eax_TestQuerySupport(ListenerPropSet, EAX_BUFFER, version);

		// ��������� EAX ��������
		if (BufferPropSet) {
			BufferPropSet->Release();
			BufferPropSet = 0;
		}

		// ��������� 3D �����
		if (Buffer3D) {
			Buffer3D->Release();
			Buffer3D = 0;
		}

		// ��������� EAX ���������
		if (ListenerPropSet) {
			ListenerPropSet->Release();
			ListenerPropSet = 0;
		}

		// ��������� �����
		if (Buffer) {
			Buffer->Release();
			Buffer = 0;
		}
	}
	return ok;
}
