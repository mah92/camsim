#ifndef __NSRPLOT_H__
#define __NSRPLOT_H__

#include <plstream.h>
#include <vector>

#ifdef __cplusplus
extern "C" {
#endif

enum Color {COL_BLACK = 0, COL_RED, COL_YELLOW, COL_GREEN, COL_AQUAMARINE,
			COL_PINK = 5, COL_WHEAT, COL_GREY, COL_BROWN, COL_BLUE, COL_BLUE_VIOLET,
			COL_CYAN = 11, COL_TURQUOISE, COL_MAGNETA, COL_SALMON, COL_WHITE
		   }; //numbered colors accepted by plplot

enum PenType {PEN_POINT, PEN_LINE};

enum WinState {SUBWIN_UNINITED, SUBWIN_READY2DRAW, SUBWIN_DRAWN};

enum DrawProp {AUTOSCALE_NONE = 0, AUTOSCALE_X = 1, AUTOSCALE_Y = 2, AUTOSCALE_Z = 4, AXIS_EQUAL = 8, AUTOSCALE_AND_AXIS_EQUAL = 15, NEXT = 16};

class SubWinProperties
{
public:
	int axis_num;
	int legend_num;
	int state;
	char* xlable, *ylable, *zlable, *title;
	double xmin, xmax, ymin, ymax, zmin, zmax;
	bool xReverse, yReverse, zReverse;

	bool active;

	SubWinProperties() : axis_num(0), legend_num(0), state(SUBWIN_UNINITED),
		xlable(NULL), ylable(NULL), zlable(NULL), title(NULL) {}
	~SubWinProperties()
	{
		if(xlable != NULL) free(xlable);
		if(ylable != NULL) free(ylable);
		if(zlable != NULL) free(zlable);
		if(title != NULL) free(title);
	}
};

class SignalProperties
{
public:
	char* name;
	int sub_win;
	int pen;
	char penStyle;
	int color;

	int win_size_inited;
	double xmin, xmax, ymin, ymax, zmin, zmax;
	int auto_scaler;

	int max_points;
	int points;
	double *x;
	double *y;
	double *z;

	SignalProperties() : name(NULL), max_points(0), points(0),
		x(NULL), y(NULL), z(NULL), win_size_inited(0) {}
	~SignalProperties()
	{
		max_points = 0;
		if(x != NULL) delete[] x;
		if(y != NULL) delete[] y;
		if(z != NULL) delete[] z;
		if(name != NULL) free(name);
	}
};

class NsrFig
{
	bool enabled;
public:
	NsrFig(int m = 1, int n = 1, bool _enabled = true);

	void addSubWin(int sub_win, int axis_num = 2,
				   const char* xlable = "", const char* ylable = "", const char* zlable = "", const char* title = "",
				   bool xReverse = false, bool yReverse = false, bool zReverse = false);

	void setAxis(int sub_win, double _xmin, double _xmax, double _ymin, double _ymax, double _zmin = 0, double _zmax = 0);

	//penStyle can be any character(for points) or line style starting by 1(full line)
	void addSignal(int sub_win = 0, const char* name = "", int pen = PEN_POINT, char penStyle = '.', int color = COL_BLUE,
				   int auto_scaler = AUTOSCALE_X | AUTOSCALE_Y | AUTOSCALE_Z, int max_points = 10000);

	void activate(int sub_win, bool enabled);

	void draw(int sig, double x, double y, double z = 0.);

	~NsrFig();

	plstream *pls;
	std::vector <SubWinProperties*> subWinProps;
	std::vector <SignalProperties*> sigsProps;

	int drawn_points, shown_points;
	void regenerate_subwindow(int sub_win);
};

void refreshPlots(); //should be called more than every 0.1s externally to refresh threads, just needed when CREATE_REFRESH_THREAD not defined
int refreshPlotsOnEnd(); //should be called once at program end and ctrl+c handle
int pauseTrap();

#ifdef __cplusplus
}
#endif

#endif
