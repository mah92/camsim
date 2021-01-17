//Standard functions for logging///////////////////////////
#ifndef __NSRLOG_H__
#define __NSRLOG_H__

#include <stdio.h> //"FILE" definition

#define RESET   "\033[0m"
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */

#define  LOGOPEN(...)  ;
#define  LOGV(mTAG, ...) ; //{printf("V\t%f\t%s\t", myTime(), mTAG); printf(__VA_ARGS__);}
#define  LOGD(mTAG, ...)  {printf("BLUE D\t%f\t%s\t", myTime(), mTAG); printf(__VA_ARGS__); printf(RESET);}
#define  LOGI(mTAG, ...)  {printf(GREEN "I\t%f\t%s\t", myTime(), mTAG); printf(__VA_ARGS__); printf(RESET);}
#define  LOGW(mTAG, ...)  {printf(YELLOW "W\t%f\t%s\t", myTime(), mTAG); printf(__VA_ARGS__); printf(RESET); fflush(stdout);}
#define  LOGE(mTAG, ...)  {printf(BOLDRED "E\t%f\t%s\t", myTime(), mTAG); printf(__VA_ARGS__); printf(RESET); fflush(stdout);}
#define  LOGDUMP();  ;
#define  LOGCLOSE();  ;

#endif
