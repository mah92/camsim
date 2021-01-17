//ALLAH
#ifndef __NSRSETTINGS_H__
#define __NSRSETTINGS_H__

#include "./nsrUtility.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SettingsStruct_ {

	/** آدرس پوشه ذخیره سازی  */
	char savepath[MAX_PATH_LENGTH];

	/** آدرس محل ذخیره سازی نقشه */
	char mappath[MAX_PATH_LENGTH];

	/** آدرس محل ذخیره سازی نقشه ارتفاعی */
	char dempath[MAX_PATH_LENGTH];

} SettingsStruct;

extern SettingsStruct settings;

/**
 * فرمان مقدار دهی اولیه تنظيمات را مي دهد.
 * @return شماره خطا
 */
int init_settings();

/**
 * فرمان خواندن تنظيمات ثابت از فايل را مي دهد.
 * @return شماره خطا
 */
int read_settings();

#ifdef __cplusplus
}
#endif

#endif /*__SETTINGS_H__*/
