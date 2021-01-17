//In the name of ALLAH

/**
 * <h1> بافر اعداد <h1>
 * این کلاس یک پیاده سازی سی برای بافر حلقوی برای اعداد است
 * بافر یک مکانیزم پیشرفته ارتباطی بین اجزای مختلف برنامه است.
 * این در واقع بسیار شبیه نخاع در انسان است
 * ارتباط بین دو کلاس را در نظر بگیرید
 * این دو کلاس برای تبادل داده با یکدیگر باید به نحوی یکدیگر را صدا کنند.
 * این کار موجب وابستگی دو کلاس به یکدیگر می شود
 * حتی اگر وابستگی بسیار کم باشد دیگر امکان کامپایل جداگانه آنها وجود ندارد
 * استفاده از بافر این دو کلاس را از هم دیکوپل می کند.
 * <p>
 * امکانات بافر: ا
 * <p>
 * بافر چند کاناله است به این معنی که امکان ذخیره داده های
 * مختلف(با معانی مختلف) را دارد.
 * <p>
 * بافر حافظه(عمق) دارد یعنی می تواند چندین داده از آخر را ذخیره کند. این امکان خصوصا در برنامه هایی
 * که فرکانس اجرای قسمت های مختلف فرق دارد حائز اهمیت است
 * <p>
 * این مکانیزم به مکانهای مختلف برنامه این امکان را می دهد که
 * بدون فراخوانی یکدیگر از آخرین داده های یکدیگر استفاده کنند.
 * این توانایی خصوصا در برنامه نویسی چند نرخی که در آن
 * هر تابعی ممکن است با فرکانس متفاوتی اجرا شود مهم است.
 * <p>
 * از آن جا که در کد جاوا نیز مکانیزم مشابهی تعبیه شده و از طریق
 * حافظه مشترک به این مکانیزم متصل شده است، امکان ارتباط غیر مستقیم
 * با کد سی نیز وجود دارد. این ویژگی باعث کاهش صدا زدن های کد سی
 * از درون جاوا شده که با کاهش سربار آن کارایی برنامه را بهبود می دهد.
 * <p>
 * بافر از نوع حلقوی است یعنی هر چقدر هم داده در آن ریخته شود،
 * همیشه به اندازه حجم خود آخرین داده های ریخته شده
 * ا(و نه داده های قدیمی) را در خود دارد
 * <p>
 * اگر چند مشتری برای داده وجود داشته باشد، امکان برداشتن داده ها توسط
 * چند خواننده مستقل وجود دارد. برای مثال بخشی از کد ممکن است داده ها را
 * ذخیره کند، بخشی دیگر همان داده ها را برای وجود بسته های داده از نوع
 * زبان ماولینک جستجو کند و بخشی دیگر در جستجوی داده هایی از
 * نوع پروتکلی دیگر باشد. تمام این بخش ها می توانند به صورت مستقل داده ی
 * خود را از بافر دریافت کنند
 * <p>
 * نکته: همواره رعایت این نکته مهم است که در برنامه های چند نرخی، هنگام استفاده
 * از منابع مشترک مثل این بافر از قفل استفاده شود.
 * <p>
 * نکته: مکان داده های مختلف در بافر توسط
 * @link nsrIndex.c @endlink
 * تعیین و بازیابی می شود.
 *
 * @see com.nasir.sensors.NsrNumBuffer
 * @see nsrSerialBuffer2.c
 * @since 1391
 * @version 1
 */

/*  Help:
 *  Access to matrix like matlab 2D matrix converted to C:
 *  Z(m,n):=Z[(m-1)+(n-1)*M], where:
 *  m is rows num>=1, n is cols num(depth)>=1
 *  M is total rows num(no_Elems), N is total cols num(no_Depth)
*/

#include "./nsrNumBuffer.h"
#include <string.h> //for malloc and memcpy
#include <stdlib.h>//for malloc in some architectures

#define equals(variable, value, thresh)	\
	((((variable) - (value)) <= thresh && ((value) - (variable)) <= thresh)?1:0)

#ifdef __cplusplus
extern "C" {
#endif

#ifdef NO_MALLOC
void cbInitNumBuffer(NumBuffer* mNB)
{
	mNB->no_Elems = cbBUFELEMS;
	mNB->no_Depth = cbBUFDEPTH;
	mNB->no_Readers = cbREADERS;
#else
void cbInitNumBuffer(NumBuffer* mNB, int Elems_, int Depth_, int Readers_)
{
	mNB->no_Elems = Elems_;
	mNB->no_Depth = Depth_;
	mNB->no_Readers = Readers_;

	mNB->Data = (double*)malloc(mNB->no_Elems * mNB->no_Depth * sizeof(double)); //=ptr_Data;
	//obj.Data = zeros(Elems_,Depth_);
	mNB->Time = (double*)malloc(mNB->no_Elems * mNB->no_Depth * sizeof(double)); //=ptr_Time;
	//obj.Time = zeros(Elems_,Depth_);
	mNB->Wrtn = (int*)malloc(mNB->no_Elems * sizeof(int)); //=ptr_Wrtn;
	//obj.Wrtn = zeros(Elems_,1);
	mNB->Read = (int*)malloc(mNB->no_Elems * mNB->no_Readers * sizeof(int)); //=ptr_Read
	//obj.Read = zeros(Elems_,Readers_);
#endif

	mNB->UseBuffer = 1;
	mNB->ERR = 0;

	cbReset(mNB);
}

#ifndef NO_MALLOC
void cbInitNumBuffer2(NumBuffer* mNB, int Elems_, int Depth_, int Readers_, double ptr_Data[], double ptr_Time[], int ptr_Wrtn[])
{
	int i;
	mNB->UseBuffer = 1;
	mNB->ERR = 0;

	mNB->no_Elems = Elems_;
	mNB->no_Depth = Depth_;
	mNB->no_Readers = Readers_;

	mNB->Data = ptr_Data; //=new double[mNB->no_Elems*mNB->no_Depth];
	mNB->Time = ptr_Time; //=new double[mNB->no_Elems*mNB->no_Depth];
	mNB->Wrtn = ptr_Wrtn; //=new int[mNB->no_Elems +1];

	mNB->Read = (int*)malloc(mNB->no_Elems * mNB->no_Readers * sizeof(int)); //=ptr_Read

	for(i = 0; i < mNB->no_Elems * mNB->no_Readers; i++)
		mNB->Read[i] = 0;
}
#endif

void cbErase(NumBuffer* mNB)
{
#ifdef NO_MALLOC

#else
	free(mNB->Data);
	free(mNB->Time);
	free(mNB->Wrtn);
	free(mNB->Read);
#endif
}

void cbReset(NumBuffer* mNB)                                           //Reset Data & Time
{
	int i;

	for(i = 0; i < mNB->no_Elems * mNB->no_Depth; i++) {
		mNB->Data[i] = 0;
		mNB->Time[i] = 0;
	}

	for(i = 0; i < mNB->no_Elems * mNB->no_Readers; i++)
		mNB->Read[i] = 0;

	for(i = 0; i < mNB->no_Elems; i++)
		mNB->Wrtn[i] = 0;
}

void cbUpdateto(NumBuffer* mNB, NumBuffer* Buff)
{
	int i;

	for(i = 0; i < mNB->no_Elems * mNB->no_Depth; i++)
		Buff->Data[i] = mNB->Data[i];

	for(i = 0; i < mNB->no_Elems * mNB->no_Depth; i++)
		Buff->Time[i] = mNB->Time[i];

	for(i = 0; i < mNB->no_Elems; i++)
		Buff->Wrtn[i] = mNB->Wrtn[i];

	//Read Should Not be updated
	/*for(i=0;i<mNB->no_Elems*mNB->no_Readers;i++)
		Buff->Read[i]=mNB->Read[i];*/
}

void cbPush(NumBuffer* mNB, int elem, double PushData, double time)           //Push Data & Time
{
	int cursor;
	mNB->Wrtn[elem - 1]++;
	cursor = mNB->Wrtn[elem - 1] % mNB->no_Depth; //cursor:[0,mNB->no_Depth-1]
	//obj.Data(elem,cursor+1)=PushData;
	mNB->Data[(elem - 1) + (cursor + 1 - 1)*mNB->no_Elems] = PushData;
	//obj.Time(elem,cursor+1)=time;
	mNB->Time[(elem - 1) + (cursor + 1 - 1)*mNB->no_Elems] = time;
}

void cbRePush(NumBuffer* mNB, int elem, double PushData, double time)         //RePush Data & Time of
{
	mNB->Wrtn[elem - 1]--;
	cbPush(mNB, elem, PushData, time);
}

int cbPushIfChanged(NumBuffer* mNB, int elem, double PushData, double time)           //Push Data & Time if changed
{
	mNB->ERR = 0;
	double val = cbLast(mNB, elem);

	if(mNB->ERR != 0 || equals(PushData, val, 1e-6) == 0) { //buffer empty or value not equal, so push
		cbPush(mNB, elem, PushData, time);
		return 1;
	}
	return 0;
}

double cbPull(NumBuffer* mNB, int elem, int reader)                         //Pull data
{
	int unread, cursor;
	unread = mNB->Wrtn[elem - 1] - mNB->Read[(elem - 1) + (reader + 1 - 1) * mNB->no_Elems];

	if(unread <= 0) { //if Writing has been equal/less than Reading
		mNB->ERR = NUMBUFFER_ERR_PULL_NODATA;
		return 0;
	}

	if(unread > mNB->no_Depth) { //if we have lost some data due to buffer size limit
		//mNB->ERR=NUMBUFFER_ERR_PULL_DATALOST;
		mNB->Read[(elem - 1) + (reader + 1 - 1)*mNB->no_Elems] = mNB->Wrtn[elem - 1] - mNB->no_Depth;
	}

	mNB->Read[(elem - 1) + (reader + 1 - 1)*mNB->no_Elems]++;
	cursor = mNB->Read[(elem - 1) + (reader + 1 - 1) * mNB->no_Elems] % mNB->no_Depth; //cursor:[0,mNB->no_Depth-1]
	//PullData=obj.Data(elem,cursor+1);
	return mNB->Data[(elem - 1) + (cursor + 1 - 1) * mNB->no_Elems];
}

double cbCheckPull(NumBuffer* mNB, int elem, int reader)                         //Simulate pull data
{
	int unread, cursor;
	unread = mNB->Wrtn[elem - 1] - mNB->Read[(elem - 1) + (reader + 1 - 1) * mNB->no_Elems];

	if(unread <= 0) { //if Writing has been equal/less than Reading
		mNB->ERR = NUMBUFFER_ERR_PULL_NODATA;
		return 0;
	}

	if(unread > mNB->no_Depth) { //if we have lost some data due to buffer size limit
		//mNB->ERR=NUMBUFFER_ERR_PULL_DATALOST;
		mNB->Read[(elem - 1) + (reader + 1 - 1)*mNB->no_Elems] = mNB->Wrtn[elem - 1] - mNB->no_Depth;
	}

	//mNB->Read[(elem-1)+(reader+1-1)*mNB->no_Elems]++;
	cursor = (mNB->Read[(elem - 1) + (reader + 1 - 1) * mNB->no_Elems] + 1) % mNB->no_Depth; //cursor:[0,mNB->no_Depth-1]
	//PullData=obj.Data(elem,cursor+1);
	return mNB->Data[(elem - 1) + (cursor + 1 - 1) * mNB->no_Elems];
}

double cbtime(NumBuffer* mNB, int elem, int reader)                         //Time of last pulled Data
{
	int cursor;
	cursor = mNB->Read[(elem - 1) + (reader + 1 - 1) * mNB->no_Elems] % mNB->no_Depth; //cursor:[0,mNB->no_Depth-1]
	//PullData=obj.Time(elem,cursor+1);
	return mNB->Time[(elem - 1) + (cursor + 1 - 1) * mNB->no_Elems];
}

double cbChecktime(NumBuffer* mNB, int elem, int reader)	//Time of data if pulled
{
	int unread, cursor;
	unread = mNB->Wrtn[elem - 1] - mNB->Read[(elem - 1) + (reader + 1 - 1) * mNB->no_Elems];

	if(unread <= 0) { //if Writing has been equal/less than Reading
		mNB->ERR = NUMBUFFER_ERR_PULL_NODATA;
		return 0;
	}

	if(unread > mNB->no_Depth) { //if we have lost some data due to buffer size limit
		//mNB->ERR=NUMBUFFER_ERR_PULL_DATALOST;
		mNB->Read[(elem - 1) + (reader + 1 - 1)*mNB->no_Elems] = mNB->Wrtn[elem - 1] - mNB->no_Depth;
	}

	//mNB->Read[(elem-1)+(reader+1-1)*mNB->no_Elems]++;
	cursor = (mNB->Read[(elem - 1) + (reader + 1 - 1) * mNB->no_Elems] + 1) % mNB->no_Depth; //cursor:[0,mNB->no_Depth-1]
	//PullData=obj.Data(elem,cursor+1);
	return mNB->Time[(elem - 1) + (cursor + 1 - 1) * mNB->no_Elems];
}

double cbLast(NumBuffer* mNB, int elem)                                     //Gives last data
{
	int cursor;
	if(mNB->Wrtn[elem - 1] == 0) {
		mNB->ERR = NUMBUFFER_ERR_LAST_EMPTY;
		return 0;
	}

	cursor = mNB->Wrtn[elem - 1] % mNB->no_Depth; //cursor:[0,mNB->no_Depth-1]
	//LastData=obj.Data(elem,cursor+1);
	return mNB->Data[(elem - 1) + (cursor + 1 - 1) * mNB->no_Elems];
}

double cbLastTime(NumBuffer* mNB, int elem)                                 //Gives Last data time
{
	int cursor;
	if(mNB->Wrtn[elem - 1] == 0) {
		mNB->ERR = NUMBUFFER_ERR_LAST_EMPTY;
		return 0;
	}

	cursor = mNB->Wrtn[elem - 1] % mNB->no_Depth; //cursor:[0,mNB->no_Depth-1]
	//LastTime=obj.Time(elem,cursor+1);
	return mNB->Time[(elem - 1) + (cursor + 1 - 1) * mNB->no_Elems];
}

int cbIsNewData(NumBuffer* mNB, int elem, double time, double thresh)            //Tells if has new data in last interval
{
	if(mNB->Wrtn[elem - 1] == 0) { //no data at all
		mNB->ERR = NUMBUFFER_ERR_LAST_EMPTY;
		return 0;
	}

	if(time - cbLastTime(mNB, elem) <= thresh) //data new
		return 1;

	return -1; //data so old
}

double cbInterpolate(NumBuffer* mNB, int elem, double time)                  //Interpolated data
{
	int n, cursor;
	double t1, t2;
	double X1, X2;

	if(mNB->Wrtn[elem - 1] == 0) {
		mNB->ERR = NUMBUFFER_ERR_INTERPOLATE_EMPTY;
		return 0;
	}

	if(mNB->no_Depth == 1 || mNB->UseBuffer == 0 || mNB->Wrtn[elem - 1] == 1) { //interpolation disabled
		cursor = mNB->Wrtn[elem - 1] % mNB->no_Depth; //cursor:[0,mNB->no_Depth-1]
		return mNB->Data[(elem - 1) + (cursor + 1 - 1) * mNB->no_Elems];
	}

	n = mNB->Wrtn[elem - 1] - 1;

	if(time > cbLastTime(mNB, elem)) {
		mNB->ERR = NUMBUFFER_ERR_INTERPOLATE_TIMEUNREACHED;
	} else { //search through buffer
		cursor = n % mNB->no_Depth; //cursor:[0,mNB->no_Depth-1]
		while((n > 1) &&
				(mNB->Wrtn[elem - 1] - n + 1 < mNB->no_Depth) &&
				time < mNB->Time[(elem - 1) + (cursor + 1 - 1)*mNB->no_Elems]) { //if still not passed to before
			n--;
			cursor = n % mNB->no_Depth; //cursor:[0,mNB->no_Depth-1]
		}
	}

	cursor = n % mNB->no_Depth; //cursor:[0,mNB->no_Depth-1]
	//t1 = obj.Time(m,cursor+1);
	t1 = mNB->Time[(elem - 1) + (cursor + 1 - 1) * mNB->no_Elems];
	//X1 = obj.Data(m,cursor+1);
	X1 = mNB->Data[(elem - 1) + (cursor + 1 - 1) * mNB->no_Elems];

	n++;
	cursor = n % mNB->no_Depth; //cursor:[0,mNB->no_Depth-1]
	//t2 = obj.Time(m,cursor+1);
	t2 = mNB->Time[(elem - 1) + (cursor + 1 - 1) * mNB->no_Elems];
	//X2 = obj.Data(m,cursor+1);
	X2 = mNB->Data[(elem - 1) + (cursor + 1 - 1) * mNB->no_Elems];

	if(time < t1)
		mNB->ERR = NUMBUFFER_ERR_INTERPOLATE_TIMEPASSED;

	if((t2 - t1) < 1e-10) {
		return (X1 + X2) / 2.;
	}

	return (X1 * (t2 - time) + X2 * (time - t1)) / (t2 - t1); //(t1,X1)--(t,X)--------(t2,X2)
}

#ifdef __cplusplus
}
#endif
