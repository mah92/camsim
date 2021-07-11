/**
 * @file
 * این فایل وظیفه انجام کار های ساده متنی از جمله کارهای زیر را بر عهده دارد: ا
 * <ul>

 * <li> بررسی انتهای متن(مثل وجود یا عدم وجود یک پسوند در اسم فایل) ا
 * <li> یافتن اعداد و کلمات در متن
 * <li> تبدیل متون به حروف کوچک
 * <li> جستجوی آرایه اعداد
 * </ul>
 *
 * @since 1393
 */

#include "nsrStrUtility.h"
//#include "./nsrMsgLog.h"

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

//0 if endsWith
int endsWith(const char *str, const char *suffix)
{
	if(!str || !suffix)
		return -1;
	int lenstr = strlen(str);
	int lensuffix = strlen(suffix);
	if(lensuffix >  lenstr)
		return -1;
	return strncmp(str + lenstr - lensuffix, suffix, lensuffix);
}
	
int findIntInLine(const char* linebuf, int searchFrom, int searchTo, int* foundFrom, int* foundTo)
{
	int i;
	int len = 0;

	for(i = searchFrom; i <= searchTo; i++) {
		if(isNum(linebuf[i]) || linebuf[i] == '-' || linebuf[i] == '+') { //is num letter
			if(len == 0) //is first num letter
				*foundFrom = i;
			*foundTo = i;
			len++;
		} else if(len > 0) //Num scope finished.
			break;
	}

	return len;
}

int findFloatInLine(const char* linebuf, int searchFrom, int searchTo, int* foundFrom, int* foundTo)
{
	int i;
	int dots = 0;
	int len = 0, len1 = 0, len2 = 0;
	int foundTo1, foundTo2;
	int foundFrom1, foundFrom2;
	int foundDot = 0;

	for(i = searchFrom; i <= searchTo; i++) {
		if(isNum(linebuf[i]) || linebuf[i] == '-' || linebuf[i] == '+' || (foundDot == 0 && linebuf[i] == '.')) { //a digit or first dot
			if(linebuf[i] == '.')
				foundDot++;

			if(len == 0) //is first letter
				*foundFrom = i;
			*foundTo = i;
			len++;
		} else if(foundDot == len && len > 0) { //just found a single dot, not a digit, continue search
			len = 0;
			continue;
		} else if(len > 0) //String scope finished.
			break;
	}

	//LOGI("Hosein", "Searching in (%s), found (%f)", &linebuf[searchFrom], atof(&linebuf[*foundFrom]));

	return len;
}

#ifdef __cplusplus
}
#endif
