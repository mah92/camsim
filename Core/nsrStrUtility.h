#ifndef __NSRSTRUTILITY__
#define __NSRSTRUTILITY__

#define MAX_PARAM_LENGTH 60
#define MAX_LINE_LENGTH 200

#define MAX_PATH_LENGTH 300
#define MAX_FILENAME_LENGTH 60

#define isNum(val) (('0'<=(val) && (val)<='9')?1:0)

#ifdef __cplusplus
extern "C" {
#endif

//0 if endsWith
int endsWith(const char *str, const char *suffix);
	
//returns len
int findIntInLine(char* linebuf, int searchFrom, int searchTo, int* foundFrom, int* foundTo);

//returns len
int findFloatInLine(char* linebuf, int searchFrom, int searchTo, int* foundFrom, int* foundTo);

#ifdef __cplusplus
}
#endif

#endif // __NSRSTRUTILITY__
