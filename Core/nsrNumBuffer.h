#ifndef __NSRNumBuffer_H__
#define __NSRNumBuffer_H__

//#define NO_MALLOC
//#define cbBUFELEMS 16
//#define cbBUFDEPTH 5
//#define cbREADERS 2

#ifdef __cplusplus
extern "C" {
#endif

#define NUMBUFFER_ERR_PULL_NODATA -1
#define NUMBUFFER_ERR_PULL_DATALOST -2
#define NUMBUFFER_ERR_LAST_EMPTY -3
#define NUMBUFFER_ERR_INTERPOLATE_EMPTY -4
#define NUMBUFFER_ERR_INTERPOLATE_TIMEUNREACHED -5
#define NUMBUFFER_ERR_INTERPOLATE_TIMEPASSED -6

typedef struct NumBuffer_ {
	/** تعداد کانال های بافر */
	int no_Elems;
	/** عمق ذخیره سازی توسط بافر */
	int no_Depth;
	/** تعداد خواننده های موازی بافر، هر خواننده تا جایی که می خواهد می خواند. */
	int no_Readers;
	/** این گزینه میتواند بافر را غیر فعال کند به طوری که همیشه آخرین المان ورودی را بدهد. */
	int UseBuffer;
	/** نوع خطا */
	int ERR;
#ifdef NO_MALLOC
	/** تعریف داده؛
	 * تعریف بدون دستور malloc ولی با طول یکسان در تمام نمونه های بافر این کتابخانه
	 */
	double Data[cbBUFELEMS * cbBUFDEPTH];
	/** تعریف زمان ذخیره داده؛
	 * تعریف بدون دستور malloc ولی با طول یکسان در تمام نمونه های بافر این کتابخانه
	 */
	double Time[cbBUFELEMS * cbBUFDEPTH];
	/** تعداد خانه های حافظه نوشته شده در هر کانال؛
	 * تعریف بدون دستور malloc ولی با طول یکسان در تمام نمونه های بافر این کتابخانه.
	 */
	int Wrtn[cbBUFELEMS];
	/** تعداد خانه های حافظه خوانده شده در هر کانال و توسط خوانندگان مختلف از بافر؛
	 * تعریف بدون دستور malloc ولی با طول یکسان در تمام نمونه های بافر این کتابخانه
	 */
	int Read[cbBUFELEMS * cbREADERS];
#else
	/** تعداد خانه های حافظه خوانده شده در هر کانال و توسط خوانندگان مختلف از بافر؛
	 * داده با دستور malloc تعریف شده و میتواند هر طولی داشته باشد.
	 */
	int *Read;
	/** تعریف داده؛
	 * داده با دستور malloc تعریف شده و میتواند هر طولی داشته باشد.
	 */
	double *Data;
	/** تعریف زمان ذخیره داده؛
	 * داده با دستور malloc تعریف شده و میتواند هر طولی داشته باشد.
	 */
	double *Time;
	/** تعداد خانه های حافظه نوشته شده در هر کانال؛
	 * داده با دستور malloc تعریف شده و میتواند هر طولی داشته باشد.
	 */
	int *Wrtn;
#endif
	//Due to unknown error, every element defined here may overlap structure arrays in NO_MALLOC condition!!! (mingw compiler)
} NumBuffer;

#ifdef NO_MALLOC
/**
 * حافظه عمومی کلاس را بدون استفاده از دستور malloc مقدار دهی اولیه می کند.
 *
 * @param[in,Out] mNB حافظه عمومی کلاس
 */
void cbInitNumBuffer(NumBuffer* mNB);
#else
/**
 * حافظه عمومی کلاس را با استفاده از دستور malloc مقدار دهی اولیه می کند.
 *
 * @param[in,Out] mNB حافظه عمومی کلاس
 * @param[in] Elems_ تعداد کانال های بافر
 * @param[in] Depth_ عمق ذخیره سازی
 * @param[in] Readers_ تعداد خوانندگان بافر
 */
void cbInitNumBuffer(NumBuffer* mNB, int Elems_, int Depth_, int Readers_);
#endif

#ifndef NO_MALLOC
/**
 * حافظه عمومی کلاس را با استفاده از دستور malloc مقدار دهی اولیه می کند
 * ولی داده و تعداد دفعات نوشته شدن در بافر را به صورت اشاره گر میگیرد.
 *
 * @param[in,Out] mNB حافظه عمومی کلاس
 * @param[in] Elems_ تعداد کانال های بافر
 * @param[in] Depth_ عمق ذخیره سازی
 * @param[in] Readers_ تعداد خوانندگان بافر
 * @param[in] ptr_Data اشاره گر داده
 * @param[in] ptr_Time اشاره گر زمان ذخیره سازی داده
 * @param[in] ptr_Wrtn اشاره گر تعداد دفعات نوشتن شده در داده
 */
void cbInitNumBuffer2(NumBuffer* mNB, int Elems_, int Depth_, int Readers_, double ptr_Data[], double ptr_Time[], int ptr_Wrtn[]);
#endif

/**
 * حافظه بافر را آزاد می کند.
 *
 * @param[in,out] mNB حافظه عمومی کلاس
 */
void cbErase(NumBuffer* mNB);

/**
 * حافظه بافر را پاک می کند.
 *
 * @param[in,out] mNB حافظه عمومی کلاس
 */
void cbReset(NumBuffer* mNB);

/**
 * بافر اول را در بافر دوم کپی می کند.
 *
 * @param[in] mNB بافر اول
 * @param[out] Buff بافر دوم
 */
void cbUpdateto(NumBuffer* mNB, NumBuffer* Buff);

/**
 * داده را در کانال مشخص و در زمان مشخص در بافر می گذارد.
 *
 * @param[in,out] mNB حافظه عمومی کلاس
 * @param[in] elem کانال مورد نظر در بافر، شروع از یک
 * @param[in] PushData داده ورودی
 * @param[in] time زمان ورود داده
 */
void cbPush(NumBuffer* mNB, int elem, double PushData, double time);

/**
 * داده قبلی را پاک  کرده و دوباره داده دیگری را در کانال مشخص و در زمان مشخص در بافر می گذارد.
 *
 * @param[in,out] mNB حافظه عمومی کلاس
 * @param[in] elem کانال مورد نظر در بافر، شروع از یک
 * @param[in] PushData داده ورودی
 * @param[in] time زمان ورود داده
 */
void cbRePush(NumBuffer* mNB, int elem, double PushData, double time);

int cbPushIfChanged(NumBuffer* mNB, int elem, double PushData, double time);           //Push Data & Time if changed

/**
 * داده ای را از کانال مشخص می خواند.
 *
 * @param[in,out] mNB حافظه عمومی کلاس
 * @param[in] elem کانال مورد نظر در بافر، شروع از یک
 * @param[in] reader شماره خواننده، شروع از صفر
 * @return داده
 */
double cbPull(NumBuffer* mNB, int elem, int reader);

/**
 * داده ای را بدون جلوبردن بافر از کانال مشخص می خواند.
 *
 * @param[in,out] mNB حافظه عمومی کلاس
 * @param[in] elem کانال مورد نظر در بافر، شروع از یک
 * @param[in] reader شماره خواننده، شروع از صفر
 * @return داده
 */
double cbCheckPull(NumBuffer* mNB, int elem, int reader);

/**
 * زمان ذخیره آخرین داده خوانده شده را از کانال مشخص می خواند.
 *
 * @param[in,out] mNB حافظه عمومی کلاس
 * @param[in] elem کانال مورد نظر در بافر، شروع از یک
 * @param[in] reader شماره خواننده، شروع از صفر
 * @return زمان
 */
double cbtime(NumBuffer* mNB, int elem, int reader);

/**
 * زمان ذخیره آخرین داده پیش از خوانده شدن از کانال مشخص را می خواند.
 *
 * @param[in,out] mNB حافظه عمومی کلاس
 * @param[in] elem کانال مورد نظر در بافر، شروع از یک
 * @param[in] reader شماره خواننده، شروع از صفر
 * @return زمان
 */
double cbChecktime(NumBuffer* mNB, int elem, int reader);

/**
 * آخرین داده را از کانال مشخص می خواند.
 *
 * @param[in,out] mNB حافظه عمومی کلاس
 * @param[in] elem کانال مورد نظر در بافر، شروع از یک
 * @return داده
 */
double cbLast(NumBuffer* mNB, int elem);

/**
 * زمان آخرین داده را از کانال مشخص می خواند.
 *
 * @param[in,out] mNB حافظه عمومی کلاس
 * @param[in] elem کانال مورد نظر در بافر، شروع از یک
 * @return زمان
 */
double cbLastTime(NumBuffer* mNB, int elem);

/**
 * می گوید که آیا داده جدید در بافر موجود است یا نه
 *
 * @param[in,out] mNB حافظه عمومی کلاس
 * @param[in] elem کانال مورد نظر در بافر، شروع از یک
 * @param[in] time زمان کنونی
 * @param[in] thresh اختلاف مجاز از زمان کنونی
 * @return 1 برای وجود داده جدید، 0 برای عدم وجود داده، -1 برای وجود داده قدیمی
 */
int cbIsNewData(NumBuffer* mNB, int elem, double time, double thresh);

/**
 * داده ای را در کانال مورد نظر و در زمانی مشخص درونیابی خطی می کند.
 * برای درون یابی تنها از دو داده استفاده می شود و زمان درونیابی باید بین زمان دو داده باشد.
 *
 * @param[in,out] mNB حافظه عمومی کلاس
 * @param[in] elem کانال مورد نظر در بافر، شروع از یک
 * @param[in] time زمان درونیابی
 * @return نتیجه درونیابی
 */
double cbInterpolate(NumBuffer* mNB, int elem, double time);

#ifdef __cplusplus
}
#endif

#endif /*__NSRNumBuffer_H__*/
