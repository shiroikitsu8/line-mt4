#include "bana.h"
#include <string.h>
#include "shared/line.h"
#include "shared/input.h"
#include "shared/config.h"

#pragma push(pack, 1)
enum BngRwIdType
{
	IDTYPE_UNKNOWN = 0x00,
	IDTYPE_BNG = 0x100,
	IDTYPE_BNG_AUTH = 0x101,
	IDTYPE_BNG_OTHER = 0x102,
	IDTYPE_SEGA = 0x200,
	IDTYPE_TITO = 0x300,
	IDTYPE_MOBILE0 = 0x800
};

enum BngRwCardType
{
	CARDTYPE_UNKNOWN = 0x00,
	CARDTYPE_MIFAREUNKNOWN = 0x100,
	CARDTYPE_MIFARE1K = 0x101,
	CARDTYPE_MIFAREMINI = 0x102,
	CARDTYPE_FELICAUNKNOWN = 0x200,
	CARDTYPE_FELICALITE = 0x201,
	CARDTYPE_FELICAPLUG = 0x203,
	CARDTYPE_FELICASTD = 0x202,
	CARDTYPE_MFELICA = 0x210
};

enum BngRwMCarrier
{
	MCARRIER_UNKNOWN = 0x00,
	MCARRIER_DOCOMO = 0x100,
	MCARRIER_AU = 0x200,
	MCARRIER_SOFTBANK = 0x300,
	MCARRIER_WILLCOM = 0x400
};

struct BngRwCardHandle_t
{
	BngRwCardType cardType; // 0
	int idLen;				// 4
	int felicaOS;			// 8
	uint8_t chipId[16];		// 12
};

struct BngRwResWaitTouch_t
{
	BngRwCardHandle_t cardHandle; // 0
	uint8_t uc_chipId[16];		  // 28
	char chipId[36];			  // 44
	char accessCode[24];		  // 80
	BngRwIdType idType;			  // 104
	BngRwCardType cardType;		  // 108
	BngRwMCarrier carrierType;	  // 112
	int32_t felicaOS;			  // 116
	char bngProductId[8];		  // 120
	uint32_t bngId;				  // 128
	uint16_t bngTypeCode;		  // 132
	uint8_t bngRegionCode;		  // 134
	uint8_t ucBlock1[16];		  // 135
	uint8_t ucBlock2[16];		  // 151
	char pad2[1];				  // 167
};

enum BngRwStat
{
	BNGRW_S_OK = 0x00,
	BNGRW_S_CANCEL = 0x03
};

typedef void(__cdecl *BngCommandCallback)(int deviceId, BngRwStat status, void *userData);
typedef void(__cdecl *BngWaitTouchCallback)(int deviceId, BngRwStat status, BngRwResWaitTouch_t *res, void *userData);

#pragma pop(pack)

#define BANA_MAX_DEVICES 2

struct BanaState
{
	bool waitingTouch;
	BngWaitTouchCallback waitingTouchCallback;
	void *waitingTouchUserData;
};

BanaState BANA_STATE[2];

struct Sys_Device_IcCard_impl
{
	char pad[36];
	int ack;		  // 36
	BngRwStat status; // 40
	BngRwResWaitTouch_t waitTouch;
};

int jmp_BngRwAttach(int deviceId, void *name, int a3, int flags, BngCommandCallback callback, void *userData)
{
	if (deviceId >= BANA_MAX_DEVICES)
		return -100;
	callback(deviceId, BNGRW_S_OK, userData);
	return 1;
}

int jmp_BngRwReqLed(int deviceId, int ledType, BngCommandCallback callback, void *userData)
{
	if (deviceId >= BANA_MAX_DEVICES)
		return -100;
	callback(deviceId, BNGRW_S_OK, userData);
	return 1;
}

int jmp_BngRwReqBeep(int deviceId, int beepType, BngCommandCallback callback, void *userData)
{
	if (deviceId >= BANA_MAX_DEVICES)
		return -100;
	callback(deviceId, BNGRW_S_OK, userData);
	return 1;
}

int jmp_BngRwDevReset(int deviceId, BngCommandCallback callback, void *userData)
{
	if (deviceId >= BANA_MAX_DEVICES)
		return -100;

	BANA_STATE[deviceId].waitingTouch = false;

	callback(deviceId, BNGRW_S_OK, userData);
	return 1;
}

int jmp_BngRwReqWaitTouch(int deviceId, int timeout, int options, BngWaitTouchCallback callback, void *userData)
{
	if (deviceId >= BANA_MAX_DEVICES)
		return -100;

	BANA_STATE[deviceId].waitingTouchCallback = callback;
	BANA_STATE[deviceId].waitingTouchUserData = userData;
	BANA_STATE[deviceId].waitingTouch = true;
	return 1;
}

int jmp_BngRwIsCmdExec(int deviceId)
{
	if (deviceId >= BANA_MAX_DEVICES)
		return -100;
	return BANA_STATE[deviceId].waitingTouch;
}

int jmp_BngRwReqCancel(int deviceId)
{
	if (deviceId >= BANA_MAX_DEVICES)
		return -100;
	if (BANA_STATE[deviceId].waitingTouch)
	{
		BANA_STATE[deviceId].waitingTouch = false;

		BngRwResWaitTouch_t res;
		memset(&res, 0, sizeof(res));

		BANA_STATE[deviceId].waitingTouchCallback(deviceId, BNGRW_S_CANCEL, &res, BANA_STATE[deviceId].waitingTouchUserData);
	}
	return 1;
}

void(__cdecl *old_Sys_Device_IcCard_Update)();
void jmp_Sys_Device_IcCard_Update()
{
	if (BANA_STATE[0].waitingTouch)
	{
		if (get_card())
		{
			BANA_STATE[0].waitingTouch = false;

			BngRwResWaitTouch_t res;
			memset(&res, 0, sizeof(res));

			res.idType = IDTYPE_BNG;
			strcpy(res.accessCode, config.accessCode);
			strcpy(res.chipId, config.chipId);

			BANA_STATE[0].waitingTouchCallback(0, BNGRW_S_OK, &res, BANA_STATE[0].waitingTouchUserData);
		}
	}

	old_Sys_Device_IcCard_Update();
}

void bana_init()
{
	memset(BANA_STATE, 0, sizeof(BANA_STATE));

	Line::Hook((void *)0x8ac3ecc, (void *)jmp_BngRwAttach);
	Line::Hook((void *)0x8ac3774, (void *)jmp_BngRwReqLed);
	Line::Hook((void *)0x8ac3654, (void *)jmp_BngRwReqBeep);
	Line::Hook((void *)0x8ac387e, (void *)jmp_BngRwReqWaitTouch);
	Line::Hook((void *)0x8ac39b0, (void *)jmp_BngRwDevReset);
	Line::Hook((void *)0x8ac3e40, (void *)jmp_BngRwIsCmdExec);
	Line::Hook((void *)0x8ac3182, (void *)jmp_BngRwReqCancel);

	Line::Hook((void *)0x836e1a0, (void *)jmp_Sys_Device_IcCard_Update, (void **)&old_Sys_Device_IcCard_Update);
}