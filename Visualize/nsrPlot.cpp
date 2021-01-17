//ALLAH
/*
sdev(): set device
 < 1> ps         PostScript File (monochrome)
 < 2> psc        PostScript File (color)
 < 3> xfig       Fig file
 < 4> null       Null device
 < 5> mem        User-supplied memory device
 < 6> wxwidgets  wxWidgets Driver
 < 7> svg        Scalable Vector Graphics (SVG 1.1)
 < 8> bmpqt      Qt Windows bitmap driver
 < 9> jpgqt      Qt jpg driver
 <10> pngqt      Qt png driver
 <11> ppmqt      Qt ppm driver
 <12> tiffqt     Qt tiff driver
 <13> svgqt      Qt SVG driver
 <14> qtwidget   Qt Widget
 <15> epsqt      Qt EPS driver
 <16> pdfqt      Qt PDF driver
 <17> extqt      External Qt driver
 <18> memqt      Memory Qt driver
*/
///for examples go to /usr/share/doc/plplot12-driver-wxwidgets/examples/c++

//////////////////////////////////////////////////////////////////////////

#include "./nsrPlot.h"
#include "./nsrLinuxKeyBoard2.h"

#include <unistd.h> //for usleep
#include <pthread.h>
#include <float.h> //for DBL_MAX
#include <plplotP.h>
#include <nsrUtility.h>

using namespace std;

//#define CREATE_REFRESH_THREAD

#ifdef __cplusplus
extern "C" {
#endif

char * nsrstrdup(const char* src)
{
	if(src == NULL)
		return NULL;
	char *dest = (char *) malloc((strlen(src) + 1) * sizeof(char));
	if(dest != NULL)
		strcpy(dest, src);

	return dest;
}

#ifdef CREATE_REFRESH_THREAD
static pthread_t refresh_thread;
static bool refreshThreadRun = false;
void *refresh_thread_function(void *argument);
#endif

static pthread_mutex_t *refresh_lock = NULL, _refresh_lock;

static int active_figures = 0;
static vector<NsrFig*> all_figures;
static nsrKeyBoard * kb;
static int active_figure = 0;
static int active_subwin = 0;
static double last_winchange_time_s = -100;

NsrFig::NsrFig(int m, int n, bool _enabled)
{
	enabled = _enabled;
	if(!enabled) return;

	int i;
	if(refresh_lock == NULL) {
		pthread_mutex_init(&_refresh_lock, NULL);
		refresh_lock = &_refresh_lock;
		kb = new nsrKeyBoard();
	}

	pthread_mutex_lock(refresh_lock);

	drawn_points = 0;
	shown_points = 0;

	//new window
	pls = new plstream();

	//pls->sdev("wxwidgets"); //lower cpu need
	pls->sdev("qtwidget");

	pls->scolbg(255, 255, 255); //back ground color, should be run before init()

	pls->ssub(n, m); //Determine win size, should be run before init()
	pls->init(); // Initialize plplot and ask sdev if not set

	pls->scol0(COL_BROWN, 0, 0, 0); //COL_BLACK not works, we changed and used brown

	////////////

	active_figures++;
	all_figures.push_back(this);

	for(i = 0; i < m * n; i++) {
		subWinProps.push_back(new SubWinProperties());
		memset((void*)subWinProps[i], 0, sizeof(SubWinProperties));
	}

#ifdef CREATE_REFRESH_THREAD
	if(refreshThreadRun == false) {
		refreshThreadRun = true;
		if(pthread_create(&refresh_thread, NULL, refresh_thread_function, (void*) NULL) != 0)
			printf(" Could not initialize refresh thread!\n");
	}
#endif

	pthread_mutex_unlock(refresh_lock);
}

void NsrFig::addSubWin(int sub_win, int axis_num,
					   const char* xlable, const char* ylable, const char* zlable, const char* title,
					   bool xReverse, bool yReverse, bool zReverse)
{
	if(!enabled) return;

	pthread_mutex_lock(refresh_lock);
	SubWinProperties * sWinP = subWinProps[sub_win];

	sWinP->state = SUBWIN_UNINITED;
	sWinP->axis_num = axis_num;
	sWinP->legend_num = 0;
	sWinP->xlable = nsrstrdup(xlable);
	sWinP->ylable = nsrstrdup(ylable);
	sWinP->zlable = nsrstrdup(zlable);
	sWinP->title = nsrstrdup(title);
	sWinP->xReverse = xReverse;
	sWinP->yReverse = yReverse;
	sWinP->zReverse = zReverse;
	sWinP->active = false;
	regenerate_subwindow(sub_win);

	pthread_mutex_unlock(refresh_lock);

}

void NsrFig::setAxis(int sub_win,
					 double _xmin, double _xmax, double _ymin, double _ymax, double _zmin, double _zmax)
{
	if(!enabled) return;

	pthread_mutex_lock(refresh_lock);
	SubWinProperties * sWinP = subWinProps[sub_win];

	sWinP->state = SUBWIN_READY2DRAW;
	sWinP->xmin = _xmin;
	sWinP->xmax = _xmax;
	sWinP->ymin = _ymin;
	sWinP->ymax = _ymax;
	sWinP->zmin = _zmin;
	sWinP->zmax = _zmax;

	pthread_mutex_unlock(refresh_lock);
}

//penStyle can be any character(for points) or line style starting by 1(full line)
void NsrFig::addSignal(int sub_win, const char* name, int pen, char penStyle, int color, int auto_scaler, int max_points)
{
	if(!enabled) return;

	pthread_mutex_lock(refresh_lock);

	if(strlen(name) > 0)
		subWinProps[sub_win]->legend_num++;

	//////////////////////////////
	sigsProps.push_back(new SignalProperties());
	int index = sigsProps.size() - 1;
	SignalProperties *sigP = sigsProps[index];

	sigP->name = nsrstrdup(name);
	sigP->sub_win = sub_win;
	sigP->pen = pen;
	sigP->penStyle = penStyle;
	sigP->color = color;

	sigP->auto_scaler = auto_scaler;

	sigP->max_points = max_points;
	sigP->x = new(nothrow) double[max_points];
	sigP->y = new(nothrow) double[max_points];
	if(subWinProps[sub_win]->axis_num == 3)
		sigP->z = new(nothrow) double[max_points];

	sigP->win_size_inited = 0;

	sigP->xmin = 0;
	sigP->xmax = 0;
	sigP->ymin = 0;
	sigP->ymax = 0;
	sigP->zmin = 0;
	sigP->zmax = 0;

	sigP->points = 0;

	pthread_mutex_unlock(refresh_lock);
}

void NsrFig::activate(int sub_win, bool _enabled)
{
	if(!enabled) return;

	SubWinProperties * sWinP = subWinProps[sub_win];

	sWinP->active = _enabled;
	sWinP->state = SUBWIN_READY2DRAW;
}

void NsrFig::draw(int sig, double x, double y, double z)
{
	if(!enabled) return;
	pthread_mutex_lock(refresh_lock);

	SignalProperties * sigP = sigsProps[sig];
	SubWinProperties * sWinP = subWinProps[sigP->sub_win];

	drawn_points++;

	if(sigP->win_size_inited == 0) { //init
		sigP->xmin = x;
		sigP->xmax = x;
		sigP->ymin = y;
		sigP->ymax = y;
		sigP->zmin = z;
		sigP->zmax = z;
		sigP->win_size_inited = 1;
	} else {
		sigP->xmin = min(sigP->xmin, x);
		sigP->xmax = max(sigP->xmax, x);
		sigP->ymin = min(sigP->ymin, y);
		sigP->ymax = max(sigP->ymax, y);
		sigP->zmin = min(sigP->zmin, z);
		sigP->zmax = max(sigP->zmax, z);
	}

	sigP->x[sigP->points % sigP->max_points] = x;
	sigP->y[sigP->points % sigP->max_points] = y;
	if(sWinP->axis_num == 3)
		sigP->z[sigP->points % sigP->max_points] = z;
	
	sigP->points++;

	///////////////////////

	pls->adv(sigP->sub_win + 1); //set sub window(starting from 1, zero means whole??)

	if(sWinP->axis_num == 2) {
		if(sigP->pen == PEN_LINE
				&& sigP->points > 1) { //is not first point
			pls->lsty(sigP->penStyle); //select line style
			pls->col0(sigP->color);

			double pre_x, pre_y;
			pre_x = sigP->x[(sigP->points - 1) % sigP->max_points];
			pre_y = sigP->y[(sigP->points - 1) % sigP->max_points];
			pls->join(pre_x, pre_y, x, y);  //draws line between (x1,y1, x2,y2);
		}

		if(sigP->pen == PEN_POINT) {
			pls->col0(sigP->color);
			pls->poin(1, &x, &y, sigP->penStyle);   //draws individual points
		}
	}

	if(sWinP->axis_num == 3) {
		if(sigP->pen == PEN_LINE
				&& sigP->points > 1) { //is not first point
			pls->lsty(sigP->penStyle); //select line style
			pls->col0(sigP->color);

			double _2x[2], _2y[2], _2z[2];
			_2x[0] = sigP->x[(sigP->points - 1) % sigP->max_points];
			_2y[0] = sigP->y[(sigP->points - 1) % sigP->max_points];
			_2z[0] = sigP->z[(sigP->points - 1) % sigP->max_points];
			_2x[1] = x;
			_2y[1] = y;
			_2z[1] = z;
			pls->line3(2, _2x, _2y, _2z);
		}

		if(sigP->pen == PEN_POINT) {
			pls->col0(sigP->color);
			pls->poin3(1, &x, &y, &z, sigP->penStyle);   //draws individual points
		}
	}

	pthread_mutex_unlock(refresh_lock);
}

void NsrFig::regenerate_subwindow(int sub_win)
{
	if(!enabled) return;

	int i, j, sig;
	char tmp_title[100];
	//	pthread_mutex_lock(refresh_lock);

	SubWinProperties * sWinP = subWinProps[sub_win];

	// Clear sub window and create a labled box to hold the plot.
	//pls->env(..., 0, 0 ); //recalling will cause a pause
	pls->adv(sub_win + 1); //set sub window(starting from 1, zero means whole??)
	pls->clear();
	pls->vsta(); //set sub window viewport as standard

	if(sWinP->axis_num == 2) {

		// Draw box with window dimensions
		pls->wind(sWinP->xReverse ? sWinP->xmax : sWinP->xmin, sWinP->xReverse ? sWinP->xmin : sWinP->xmax,
				  sWinP->yReverse ? sWinP->ymax : sWinP->ymin, sWinP->yReverse ? sWinP->ymin : sWinP->ymax);

		pls->lsty(1);
		pls->col0(COL_BROWN);   //COL_BLACK not works, we changed and used brown
		pls->box("bcnst", 0.0, 0, "bcnstv", 0.0, 0);

		if(sWinP->active == true)
			sprintf(tmp_title, "%s(Active)", sWinP->title);
		else
			strcpy(tmp_title, sWinP->title);
		pls->lab(sWinP->xlable, sWinP->ylable, tmp_title);
	} else { //3d
		//pls->w3d (basex, basey, height,
		//	xReverse?xmax:xmin, xReverse?xmin:xmax, yReverse?ymax:ymin, yReverse?ymin:ymax, zReverse?zmax:zmin, zReverse?zmin:zmax
		//	alt, az); //setup window for 3d plotting
		printf(" 3D plot Not supported yet!\n");
		fflush(stdout);
		//pthread_mutex_unlock(refresh_lock);
		exit(1);
	}

	//create legend
	//no legend on empty name
	//Add legend/////

	if(sWinP->legend_num > 0) {
		PLINT      nlegend = sWinP->legend_num;
		const char *text[sWinP->legend_num];
		//char symbols[sWinP->legend_num][5];
		char *symbols[sWinP->legend_num];
		for(i = 0; i < sWinP->legend_num; i++) symbols[i] = (char*)malloc(8);

		PLINT      text_colors[sWinP->legend_num];
		PLINT      line_colors[sWinP->legend_num];
		PLINT      line_styles[sWinP->legend_num];
		PLFLT      line_widths[sWinP->legend_num];
		PLINT      symbol_numbers[sWinP->legend_num], symbol_colors[sWinP->legend_num];
		PLFLT      symbol_scales[sWinP->legend_num];
		PLINT      opt_array[sWinP->legend_num];
		PLFLT      legend_width, legend_height;

		sig = 0;
		for(i = 0; i < sigsProps.size(); i++) {
			SignalProperties * sigP = sigsProps[i];

			if(sub_win != sigP->sub_win)
				continue;

			if(strlen(sigP->name) == 0)
				continue;

			// First legend entry.
			text_colors[sig] = COL_BROWN;
			text[sig]        = sigP->name;

			if(sigP->pen == PEN_POINT) {
				opt_array[sig]      = PL_LEGEND_SYMBOL;
				symbol_colors[sig]  = sigP->color;
				symbol_scales[sig]  = 1.;
				symbol_numbers[sig] = sigP->penStyle == '.' ? 8 : 4;
				symbols[sig][0]     = sigP->penStyle;
				symbols[sig][1]     = 0;
			} else { //PEN_LINE
				opt_array[sig]   = PL_LEGEND_LINE;
				line_colors[sig] = sigP->color;
				line_styles[sig] = sigP->penStyle;
				line_widths[sig] = 1.0;
				symbols[sig][0] = 0;
			}

			sig++;
		}

		pls->legend(&legend_width, &legend_height,
					PL_LEGEND_BACKGROUND | PL_LEGEND_BOUNDING_BOX, 0,
					0.0, 0.0, 0.1, 15,
					1, 1, 0, 0,
					nlegend, opt_array,
					1.0, 1.0, 2.0,
					1., text_colors, (const char * const *) text,
					NULL, NULL, NULL, NULL,
					line_colors, line_styles, line_widths,
					symbol_colors, symbol_scales, symbol_numbers, (const char **) symbols);

		for(i = 0; i < sWinP->legend_num; i++) free(symbols[i]);

	}

	//redraw points
	if(sWinP->axis_num == 2) {
		for(i = 0; i < sigsProps.size(); i++) {
			SignalProperties * sigP = sigsProps[i];
			if(sigP->sub_win != sub_win) continue;

			if(sigP->pen == PEN_LINE
					&& sigP->points > 1) { //is not first point
				pls->lsty(sigP->penStyle); //select line style
				pls->col0(sigP->color);

				if(sigP->points <= sigP->max_points) {
					pls->line(sigP->points, sigP->x, sigP->y);
				} else {
					pls->line(sigP->max_points - (sigP->points % sigP->max_points),
							  &sigP->x[sigP->points % sigP->max_points],
							  &sigP->y[sigP->points % sigP->max_points]);
					pls->line(sigP->points % sigP->max_points, sigP->x, sigP->y);
				}
			}

			if(sigP->pen == PEN_POINT) {
				pls->col0(sigP->color);
				if(sigP->points <= sigP->max_points) {
					for(j = 0; j < sigP->points; j++)
						pls->poin(1, &sigP->x[j], &sigP->y[j], sigP->penStyle);   //draws individual points
				} else {
					for(j = sigP->points % sigP->max_points; j < sigP->max_points ; j++)
						pls->poin(1, &sigP->x[j], &sigP->y[j], sigP->penStyle);   //draws individual points
					for(j = 0; j < sigP->points % sigP->max_points; j++)
						pls->poin(1, &sigP->x[j], &sigP->y[j], sigP->penStyle);   //draws individual points
				}
			}
		}
	}

	pls->flush();

	sWinP->state = SUBWIN_DRAWN;
	//	pthread_mutex_unlock(refresh_lock);
}

static double pre_refresh_time_s = 0;
static int counter = 0;
static void _refreshPlots()
{
	int i, j, sub_win;
	double len, mean;

	if(all_figures.size() == 0)
		return;

	double time_s = myTime();
	if(time_s - pre_refresh_time_s <= 0.1)
		return;
	pre_refresh_time_s = time_s;

	pthread_mutex_lock(refresh_lock);
	counter++;

	//read keyboard
	static int last_read_index = 0;
	uint16_t ch;
	while((ch = kb->getch(last_read_index))) {
		//printf("haltu\n");

		//change active subwindow
		if(kb->alt_pressed) {
			//printf("haltu2\n");
			switch(ch) {
				case KEY_RIGHT:
					//printf("right!\n");
					all_figures[active_figure]->activate(active_subwin, false);
					active_figure++;
					active_figure = active_figure >= all_figures.size() ?
									0 : active_figure;
					active_subwin = active_subwin >= all_figures[active_figure]->subWinProps.size() ?
									0 : active_subwin;
					all_figures[active_figure]->activate(active_subwin, true);
					last_winchange_time_s = myTime();
					break;

				case KEY_LEFT:
					all_figures[active_figure]->activate(active_subwin, false);
					active_figure--;
					active_figure = active_figure < 0 ?
									all_figures.size() - 1 : active_figure;
					active_subwin = active_subwin >= all_figures[active_figure]->subWinProps.size() ?
									0 : active_subwin;
					all_figures[active_figure]->activate(active_subwin, true);
					last_winchange_time_s = myTime();
					break;

				case KEY_DOWN:
					all_figures[active_figure]->activate(active_subwin, false);
					active_subwin++;
					active_subwin = active_subwin >= all_figures[active_figure]->subWinProps.size() ?
									0 : active_subwin;
					all_figures[active_figure]->activate(active_subwin, true);
					last_winchange_time_s = myTime();
					break;
				case KEY_UP:
					all_figures[active_figure]->activate(active_subwin, false);
					active_subwin--;
					active_subwin = active_subwin < 0 ?
									all_figures[active_figure]->subWinProps.size() - 1 : active_subwin;
					all_figures[active_figure]->activate(active_subwin, true);
					last_winchange_time_s = myTime();
					break;
			}
		}

		SubWinProperties * sWinP;

		//shift view
#define SHIFT_COEF 0.2
		if(kb->shift_pressed)
			switch(ch) {
				case KEY_RIGHT:
					sWinP = all_figures[active_figure]->subWinProps[active_subwin];
					len = (sWinP->xmax - sWinP->xmin) * SHIFT_COEF;
					if(sWinP->xReverse) len = -len;
					sWinP->xmin += len;
					sWinP->xmax += len;
					sWinP->state = SUBWIN_READY2DRAW;
					last_winchange_time_s = myTime();
					break;
				case KEY_LEFT:
					sWinP = all_figures[active_figure]->subWinProps[active_subwin];
					len = -(sWinP->xmax - sWinP->xmin) * SHIFT_COEF;
					if(sWinP->xReverse) len = -len;
					sWinP->xmin += len;
					sWinP->xmax += len;
					sWinP->state = SUBWIN_READY2DRAW;
					last_winchange_time_s = myTime();
					break;

				case KEY_DOWN:
					sWinP = all_figures[active_figure]->subWinProps[active_subwin];
					len = -(sWinP->ymax - sWinP->ymin) * SHIFT_COEF;
					if(sWinP->yReverse) len = -len;
					sWinP->ymin += len;
					sWinP->ymax += len;
					sWinP->state = SUBWIN_READY2DRAW;
					last_winchange_time_s = myTime();
					break;
				case KEY_UP:
					sWinP = all_figures[active_figure]->subWinProps[active_subwin];
					len = (sWinP->ymax - sWinP->ymin) * SHIFT_COEF;
					if(sWinP->yReverse) len = -len;
					sWinP->ymin += len;
					sWinP->ymax += len;
					sWinP->state = SUBWIN_READY2DRAW;
					last_winchange_time_s = myTime();
					break;
			}

		//zoom
#define ZOOM_COEF 0.2
		if(kb->ctrl_pressed)
			switch(ch) {
				case KEY_RIGHT:
					sWinP = all_figures[active_figure]->subWinProps[active_subwin];
					mean = (sWinP->xmax + sWinP->xmin) * 0.5;
					len = (sWinP->xmax - sWinP->xmin) * 0.5 / (1. + ZOOM_COEF);
					sWinP->xmin = mean - len;
					sWinP->xmax = mean + len;
					sWinP->state = SUBWIN_READY2DRAW;
					last_winchange_time_s = myTime();
					break;
				case KEY_LEFT:
					sWinP = all_figures[active_figure]->subWinProps[active_subwin];
					mean = (sWinP->xmax + sWinP->xmin) * 0.5;
					len = (sWinP->xmax - sWinP->xmin) * 0.5 * (1. + ZOOM_COEF);
					sWinP->xmin = mean - len;
					sWinP->xmax = mean + len;
					sWinP->state = SUBWIN_READY2DRAW;
					last_winchange_time_s = myTime();
					break;

				case KEY_DOWN:
					sWinP = all_figures[active_figure]->subWinProps[active_subwin];
					mean = (sWinP->ymax + sWinP->ymin) * 0.5;
					len = (sWinP->ymax - sWinP->ymin) * 0.5 * (1. + ZOOM_COEF);
					sWinP->ymin = mean - len;
					sWinP->ymax = mean + len;
					sWinP->state = SUBWIN_READY2DRAW;
					last_winchange_time_s = myTime();
					break;
				case KEY_UP:
					sWinP = all_figures[active_figure]->subWinProps[active_subwin];
					mean = (sWinP->ymax + sWinP->ymin) * 0.5;
					len = (sWinP->ymax - sWinP->ymin) * 0.5 / (1. + ZOOM_COEF);
					sWinP->ymin = mean - len;
					sWinP->ymax = mean + len;
					sWinP->state = SUBWIN_READY2DRAW;
					last_winchange_time_s = myTime();
					break;
			}

	}

	//printf("(%i, %i\n", active_figure, active_subwin);fflush(stdout);

	for(i = 0; i < all_figures.size(); i++) {
		NsrFig * fig = all_figures[i];

		///////////////
		//check for flush need
		//if(fig->drawn_points > fig->shown_points)
		fig->pls->flush(); //draws undrawn points, redraws hidden-resized windows

		//check & declare resize need
		if((counter % 10) == (i % 10)) {  //distribute win update for figures in different times
			if(fig->drawn_points > fig->shown_points) {
				for(j = 0; j < fig->sigsProps.size(); j++) {
					SignalProperties *sigP = fig->sigsProps[j];
					sub_win = sigP->sub_win;
					SubWinProperties *sWinP = fig->subWinProps[sub_win];

					//Do not modify while user is modifying
					if((sWinP->active && myTime() < last_winchange_time_s + 5.))
						continue;

					if((sigP->auto_scaler & AUTOSCALE_X) &&
							(sWinP->xmin > sigP->xmin
							 || sWinP->xmax < sigP->xmax)) {
						len = fabs(sWinP->xmax - sWinP->xmin) * SHIFT_COEF;
						if(sigP->xmin < sWinP->xmin)
							sWinP->xmin = sigP->xmin - len;
						if(sigP->xmax > sWinP->xmax)
							sWinP->xmax = sigP->xmax + len;

						sWinP->state = SUBWIN_READY2DRAW;
					}

					if((sigP->auto_scaler & AUTOSCALE_Y) &&
							(sWinP->ymin > sigP->ymin
							 || sWinP->ymax < sigP->ymax)) {
						len = fabs(sWinP->ymax - sWinP->ymin) * SHIFT_COEF;
						if(sigP->ymin < sWinP->ymin)
							sWinP->ymin = sigP->ymin - len;
						if(sigP->ymax > sWinP->ymax)
							sWinP->ymax = sigP->ymax + len;

						sWinP->state = SUBWIN_READY2DRAW;
					}

					if((sigP->auto_scaler & AUTOSCALE_Z) &&
							(sWinP->axis_num == 3 &&
							 (sWinP->zmin > sigP->zmin
							  || sWinP->zmax < sigP->zmax))) {
						len = fabs(sWinP->zmax - sWinP->zmin) * SHIFT_COEF;
						if(sigP->zmin < sWinP->zmin)
							sWinP->zmin = sigP->zmin - len;
						if(sigP->zmax > sWinP->zmax)
							sWinP->zmax = sigP->zmax + len;

						sWinP->state = SUBWIN_READY2DRAW;
					}

					//Also make other axis big too
					if(sigP->auto_scaler & AXIS_EQUAL) {
						double addlen;
						double xlen = fabs(sWinP->xmax - sWinP->xmin);
						double ylen = fabs(sWinP->ymax - sWinP->ymin);

						if(xlen > ylen) {
							addlen = (xlen - ylen) / 2;;
							sWinP->ymax += addlen; sWinP->ymin -= addlen;
						}
						if(ylen > xlen) {
							addlen = (ylen - xlen) / 2;;
							sWinP->xmax += addlen; sWinP->xmin -= addlen;
						}

						if(sWinP->axis_num == 3) {
							double zlen = fabs(sWinP->zmax - sWinP->zmin);
							if(xlen > zlen) {
								addlen = (xlen - zlen) / 2;;
								sWinP->zmax += addlen; sWinP->zmin -= addlen;
							}
							if(zlen > xlen) {
								addlen = (zlen - xlen) / 2;;
								sWinP->xmax += addlen; sWinP->xmin -= addlen;
							}
							if(ylen > zlen) {
								addlen = (ylen - zlen) / 2;;
								sWinP->zmax += addlen; sWinP->zmin -= addlen;
							}
							if(zlen > ylen) {
								addlen = (zlen - ylen) / 2;;
								sWinP->ymax += addlen; sWinP->ymin -= addlen;
							}
						}

					}

				}
			}

			fig->shown_points = fig->drawn_points;
		}

		//Fulfil Redraw need
		for(j = 0; j < fig->subWinProps.size(); j++) {
			SubWinProperties *sWinP = fig->subWinProps[j];

			if(sWinP->state == SUBWIN_READY2DRAW) {
				//printf("drawn fig:%i subwin:%i, state:%i\n", i, j, sWinP->state);
				fig->regenerate_subwindow(j);
			}
		}
	}

	pthread_mutex_unlock(refresh_lock);
}

void refreshPlots()
{
#ifndef CREATE_REFRESH_THREAD
	_refreshPlots();
#endif
}

#ifdef CREATE_REFRESH_THREAD
void *refresh_thread_function(void *argument)
{
	printf(" Starting refresh thread...\n");

	while(refreshThreadRun == true) {
		_refreshPlots();
		usleep(0.1e6);
	}

	return 0;
}
#endif

static int last_read_index = 0;
int refreshPlotsOnEnd()
{
	uint16_t ch = 0;

	while(1) {
		ch = kb->getch(last_read_index);
		if(ch == KEY_ESC || ch == KEY_Q) {
			printf("got!\n"); fflush(stdout);
			return -1;
		}
		_refreshPlots();
		usleep(100000);
	}

	return 0;
}

int pauseTrap()
{
	uint16_t ch = 0;

	ch = kb->getch(last_read_index);
	if(ch == KEY_P) {
		ch = 0;
		while(ch != KEY_P && ch != KEY_ESC && ch != KEY_Q) {
			_refreshPlots();
			usleep(100000);
			ch = kb->getch(last_read_index);
		}
	}
	return 0;
}

NsrFig::~NsrFig()
{
	int i;
	pthread_mutex_lock(refresh_lock);
	active_figures--;

	for(i = 0; i < sigsProps.size(); i++)
		delete sigsProps[i];
	sigsProps.clear();

	for(i = 0; i < subWinProps.size(); i++)
		delete subWinProps[i];
	subWinProps.clear();

	if(active_figures == 0) { //should be done after all occurances of "new plstream();"
#ifdef CREATE_REFRESH_THREAD
		refreshThreadRun = false;
		pthread_mutex_unlock(refresh_lock); //so that the thread can be done
		pthread_join(refresh_thread, NULL);
		pthread_mutex_lock(refresh_lock);
#endif

		printf(" Waiting for an enter on plots...\n"); fflush(stdout);

		for(i = 0; i < all_figures.size(); i++) { ///TODO: sigsegvs on deleting last plstream...
			printf(" deleting:%i...\n", i); fflush(stdout);
			//delete all_figures[i]->pls;
		}
		printf(" Deleted all...\n"); fflush(stdout);

		all_figures.clear();

		pthread_mutex_unlock(refresh_lock);
		pthread_mutex_destroy(refresh_lock);
		refresh_lock = NULL;

		delete kb;
		kb = NULL;

		printf(" Deleted all2...\n"); fflush(stdout);
		return;
	}

	pthread_mutex_unlock(refresh_lock);
}

#ifdef __cplusplus
}
#endif
