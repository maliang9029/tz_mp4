#ifndef _VORX_GLOBAL_FUNCTIONS_HEAD_FILE_DEFINED_
#define _VORX_GLOBAL_FUNCTIONS_HEAD_FILE_DEFINED_

#include "MacroDefine.h"

namespace vfc
{
	void VGAPI_SetErrcodeBase(long val);
	UINT64 VGAPI_GetMillisecond();
	void VGAPI_Address2String(in_addr in,char* ret);
	long VGAPI_GetMaxMaskLength(ULONG nip1,ULONG nip2);
	int VGAPI_GetLocalAddress(vector<in_addr>& addrArray);
	in_addr VGAPI_NearestAddress(vector<in_addr> addrArray,LPCTSTR sCmpIP);
	BOOL VGAPI_IsValidMask(LPCTSTR sMask);
	BOOL VGAPI_IsValidMulticast(LPCTSTR sIP);
	BOOL VGAPI_IsValidIPAddr(LPCTSTR sIP);
	BOOL VGAPI_IsValidHostIP(LPCTSTR sIP);
	BOOL VGAPI_IsValidMacAddr(LPCTSTR sMac);
	long VGAPI_GetFreeNetPort();
	char VGAPI_GetChar();
	//以下修改日期2014.6.27
	string VGAPI_GetGatIPAdress();
	in_addr VGAPI_NearestAddressEx(vector<in_addr> addrArray,LPCTSTR sCmpIP);
	//以上修改日期2014.6.27
}

#endif
