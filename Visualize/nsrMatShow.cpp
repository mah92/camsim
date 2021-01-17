#include "./nsrMatShow.h"

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#ifdef __cplusplus
extern "C" {
#endif

using namespace cv;
using namespace std;

MatShow::MatShow(const char* winname_)
{
	name_count = 0;
	names = NULL;
	mode = 0;
	winname = winname_;
	namedWindow(winname, 1);
}

void MatShow::registerNames(int name_count_, char** names_, int mode_)
{
	name_count = name_count_;
	names = names_;
	mode = mode_;
}

void MatShow::matShow(Matrice &mat, int square_size)
{
	int i, j;
	int _rows, _cols;
	_rows = nsrMin(mat.rows(), MAX_DIM);
	_cols = nsrMin(mat.cols(), MAX_DIM);
	double color;

	//mat.print(winname_);

	Mat im(_rows * square_size, _cols * square_size, CV_8UC3);
	im = cv::Scalar(255, 255, 255);

	float red, green, blue;

	double saturatedVar, min_val, max_val;
	if(mode == 0) {
#define MIN_POWER -6.
#define MAX_POWER 6.
		min_val = pow(10, MIN_POWER);
		max_val = pow(10, MAX_POWER);
	}

	for(i = 0; i < _rows; i++)
		for(j = 0; j < _cols; j++) {

			cv::Rect rect(j * square_size, i * square_size, square_size, square_size);

			if(mode == 0) {
				///Logarithmicly scaled cov
				saturatedVar = mat.elemc(i, j);
				saturatedVar = saturate2(saturatedVar, min_val, max_val);
				color = rescale(log10(saturatedVar), MIN_POWER, MAX_POWER, 0., 1.);
				//color = (mat.elemc(i, j) - mini)/(maxi - mini);
				//spread from min color to max color
				red   = rescale(color, 0., 1., 0., 255.);
				green = rescale(color, 0., 1., 255., 0.);
				blue  = rescale(color, 0., 1., 0., 0.);
			} else {
				///Pearson's R2
				color = mat.elemc(i, j) * mat.elemc(i, j) / (mat.elemc(i, i) * mat.elemc(j, j)); //r2
				//color = (mat.elemc(i, j) - mini)/(maxi - mini);
				//spread from min color to max color
				red   = rescale(color, 0., 1., 255., 0.);
				green = rescale(color, 0., 1., 255., 255.);
				blue  = rescale(color, 0., 1., 255., 0.);
			}
			cv::rectangle(im, rect, cv::Scalar(blue, green, red), -1);
		}

	for(i = 1; i < _rows; i++)
		cv::line(im, Point(0, i * square_size), Point(1, i * square_size), cv::Scalar(0, 0, 0));

	for(j = 1; j < _cols; j++)
		cv::line(im, Point(j * square_size, 0), Point(j * square_size, 1), cv::Scalar(0, 0, 0));

	for(i = 0; i < _cols; i++)
		if(isNum(names[i][0])) {
			cv::line(im, Point(i * square_size, 0), Point(i * square_size, im.cols), cv::Scalar(0, 0, 0));
			cv::line(im, Point(0, i * square_size), Point(im.rows, i * square_size), cv::Scalar(0, 0, 0));
		}

	cv::String text;
	for(i = 0; i < name_count; i++) {
		text = names[i];
		cv::putText(im, text, Point(i * square_size, (i + 0.75)*square_size),
					cv::HersheyFonts::FONT_HERSHEY_PLAIN,// int fontFace,
					square_size / 10. / strlen(names[i]), //double fontScale,
					Scalar(0, 0, 0)
					//int thickness = 1, int lineType = LINE_8,
					//bool bottomLeftOrigin = false
				   );
	}

	for(i = 1; i < name_count; i++) {
		text = names[i];
		cv::putText(im, text, Point(0, (i + 0.75)*square_size),
					cv::HersheyFonts::FONT_HERSHEY_PLAIN,// int fontFace,
					square_size / 12. / strlen(names[i]), //double fontScale,
					Scalar(0, 0, 0)
					//int thickness = 1, int lineType = LINE_8,
					//bool bottomLeftOrigin = false
				   );
	}

	for(i = 1; i < name_count; i++) {
		text = names[i];
		cv::putText(im, text, Point(i * square_size, (0 + 0.75)*square_size),
					cv::HersheyFonts::FONT_HERSHEY_PLAIN,// int fontFace,
					square_size / 12. / strlen(names[i]), //double fontScale,
					Scalar(0, 0, 0)
					//int thickness = 1, int lineType = LINE_8,
					//bool bottomLeftOrigin = false
				   );
	}

	imshow(winname, im);
}

#ifdef __cplusplus
}
#endif
