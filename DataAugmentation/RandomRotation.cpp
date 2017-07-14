/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                           License Agreement
//
// Copyright (C) 2014 Takuya MINAGAWA.
// Third party copyrights are property of their respective owners.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
// of the Software, and to permit persons to whom the Software is furnished to do
// so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
// INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
// PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//M*/

#include "RandomRotation.h"
#include "Util.h"
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>



//!儓乕乛僺僢僠乛儘乕儖偲暯峴堏摦惉暘偐傜丄僇儊儔奜晹峴楍乮夞揮亄暲恑乯傪媮傔傞
/*!
\param[in] yaw 儓乕
\param[in] pitch 僺僢僠
\param[in] roll 儘乕儖
\param[in] trans_x 暲恑堏摦X惉暘
\param[in] trans_y 暲恑堏摦Y惉暘
\param[in] trans_z 暲恑堏摦Z惉暘
\param[out] external_matrix 奜晹峴楍
*/
void composeExternalMatrix(float yaw, float pitch, float roll, float trans_x, float trans_y, float trans_z, cv::Mat& external_matrix)
{
	external_matrix.release();
	external_matrix.create(3, 4, CV_64FC1);

	double sin_yaw = sin((double)yaw * CV_PI / 180);
	double cos_yaw = cos((double)yaw * CV_PI / 180);
	double sin_pitch = sin((double)pitch * CV_PI / 180);
	double cos_pitch = cos((double)pitch * CV_PI / 180);
	double sin_roll = sin((double)roll * CV_PI / 180);
	double cos_roll = cos((double)roll * CV_PI / 180);

	external_matrix.at<double>(0, 0) = cos_pitch * cos_yaw;
	external_matrix.at<double>(0, 1) = -cos_pitch * sin_yaw;
	external_matrix.at<double>(0, 2) = sin_pitch;
	external_matrix.at<double>(1, 0) = cos_roll * sin_yaw + sin_roll * sin_pitch * cos_yaw;
	external_matrix.at<double>(1, 1) = cos_roll * cos_yaw - sin_roll * sin_pitch * sin_yaw;
	external_matrix.at<double>(1, 2) = -sin_roll * cos_pitch;
	external_matrix.at<double>(2, 0) = sin_roll * sin_yaw - cos_roll * sin_pitch * cos_yaw;
	external_matrix.at<double>(2, 1) = sin_roll * cos_yaw + cos_roll * sin_pitch * sin_yaw;
	external_matrix.at<double>(2, 2) = cos_roll * cos_pitch;

	external_matrix.at<double>(0, 3) = trans_x;
	external_matrix.at<double>(1, 3) = trans_y;
	external_matrix.at<double>(2, 3) = trans_z;
}


//! 嬮宍偺巐嬿偺嵗昗傪偦傟偧傟惸師嵗昗宯傊曄姺
cv::Mat Rect2Mat(const cv::Rect& img_rect)
{
	// 夋憸僾儗乕僩偺巐嬿偺嵗昗
	cv::Mat srcCoord(3, 4, CV_64FC1);
	srcCoord.at<double>(0, 0) = img_rect.x;
	srcCoord.at<double>(1, 0) = img_rect.y;
	srcCoord.at<double>(2, 0) = 1;
	srcCoord.at<double>(0, 1) = img_rect.x + img_rect.width;
	srcCoord.at<double>(1, 1) = img_rect.y;
	srcCoord.at<double>(2, 1) = 1;
	srcCoord.at<double>(0, 2) = img_rect.x + img_rect.width;
	srcCoord.at<double>(1, 2) = img_rect.y + img_rect.height;
	srcCoord.at<double>(2, 2) = 1;
	srcCoord.at<double>(0, 3) = img_rect.x;
	srcCoord.at<double>(1, 3) = img_rect.y + img_rect.height;
	srcCoord.at<double>(2, 3) = 1;

	return srcCoord;
}


//! 擖椡夋憸偺巐嬿傪transM偵増偭偰摟帇曄姺偟丄弌椡夋憸偺奜愙挿曽宍傪媮傔傞
/*!
\param[in] img_size 擖椡夋憸僒僀僘
\param[in] transM 3x3偺摟帇曄姺峴楍(CV_64FC1)
\param[out] CircumRect 弌椡夋憸偺奜愙挿曽宍
*/
void CircumTransImgRect(const cv::Size& img_size, const cv::Mat& transM, cv::Rect_<double>& CircumRect)
{
	// 擖椡夋憸偺巐嬿傪惸師嵗昗傊曄姺
	cv::Mat cornersMat = Rect2Mat(cv::Rect(0, 0, img_size.width, img_size.height));

	// 嵗昗曄姺偟丄斖埻傪庢摼
	cv::Mat dstCoord = transM * cornersMat;
	double min_x = std::min(dstCoord.at<double>(0, 0) / dstCoord.at<double>(2, 0), dstCoord.at<double>(0, 3) / dstCoord.at<double>(2, 3));
	double max_x = std::max(dstCoord.at<double>(0, 1) / dstCoord.at<double>(2, 1), dstCoord.at<double>(0, 2) / dstCoord.at<double>(2, 2));
	double min_y = std::min(dstCoord.at<double>(1, 0) / dstCoord.at<double>(2, 0), dstCoord.at<double>(1, 1) / dstCoord.at<double>(2, 1));
	double max_y = std::max(dstCoord.at<double>(1, 2) / dstCoord.at<double>(2, 2), dstCoord.at<double>(1, 3) / dstCoord.at<double>(2, 3));

	CircumRect.x = min_x;
	CircumRect.y = min_y;
	CircumRect.width = max_x - min_x;
	CircumRect.height = max_y - min_y;
}



//! 擖椡夋憸偲弌椡夋憸偺嵗昗偺懳墳娭學傪寁嶼
/*!
\param[in] src_size 擖椡夋憸僒僀僘
\param[in] dst_rect 擖椡夋憸傪摟帇曄姺偟偨帪偺弌椡夋憸偺奜愙挿曽宍
\param[in] transMat 4x4偺夞揮/暯峴堏摦峴楍(CV_64FC1)丅尨揰偱夞揮偝偣偰丄Z幉曽岦偵暯峴堏摦偟偨傕偺
\param[out] map_x 弌椡夋憸偺奺嵗昗偵懳偡傞擖椡夋憸偺x嵗昗
\param[out] map_y 弌椡夋憸偺奺嵗昗偵懳偡傞擖椡夋憸偺y嵗昗

transMat偼擖椡夋憸傪俁師尦揑偵夞揮偟丄偦偺拞怱傪(0,0,Z)偵抲偔傛偆偵曄姺偡傞峴楍丅
弌椡夋憸偼徟揰嫍棧偑1偺僇儊儔傪憐掕偟丄擖椡夋憸傪偙偙偵摟帇曄姺偡傞丅
偨偩偟丄僗働乕儖傪崌傢偣傞偨傔偵弌椡夋憸偺X,Y嵗昗傪1/Z偡傞丅
弌椡夋憸忋偺嵗昗偑(dx, dy)偱梌偊傜傟偨帪丄尨揰偲偦偺揰傪寢傇捈慄偼(dx*r, dy*r, Z*r)偱昞偝傟傞丅
擖椡夋憸忋偺嵗昗(sx,sy)傪俁師尦嵗昗偱昞偡偲transMat*(sx, sy, 0, 1)^T 偲側傞偺偱丄(sx, sy)偲(dx, dy)偺娭學偼
(sx, sy, 0, 1)^T = transMat^(-1) * (dx*r, dy*r, Z*r)
偲側傞丅
偙偙偐傜丄r傪徚偡偙偲偱dx偲dy偵懳墳偡傞sx偲sy偑媮傑傞丅
*/
void CreateMap(const cv::Size& src_size, const cv::Rect_<double>& dst_rect, const cv::Mat& transMat, cv::Mat& map_x, cv::Mat& map_y)
{
	map_x.create(dst_rect.size(), CV_32FC1);
	map_y.create(dst_rect.size(), CV_32FC1);

	double Z = transMat.at<double>(2, 3);

	cv::Mat invTransMat = transMat.inv();	// 媡峴楍
	cv::Mat dst_pos(3, 1, CV_64FC1);	// 弌椡夋憸忋偺嵗昗
	dst_pos.at<double>(2, 0) = Z;
	for (int dy = 0; dy<map_x.rows; dy++){
		dst_pos.at<double>(1, 0) = dst_rect.y + dy;
		for (int dx = 0; dx<map_x.cols; dx++){
			dst_pos.at<double>(0, 0) = dst_rect.x + dx;
			cv::Mat rMat = -invTransMat(cv::Rect(3, 2, 1, 1)) / (invTransMat(cv::Rect(0, 2, 3, 1)) * dst_pos);
			cv::Mat src_pos = invTransMat(cv::Rect(0, 0, 3, 2)) * dst_pos * rMat + invTransMat(cv::Rect(3, 0, 1, 2));
			map_x.at<float>(dy, dx) = src_pos.at<double>(0, 0) + (float)src_size.width / 2;
			map_y.at<float>(dy, dx) = src_pos.at<double>(1, 0) + (float)src_size.height / 2;
		}
	}
}


void RotateImage(const cv::Mat& src, cv::Mat& dst, float yaw, float pitch, float roll,
	float Z = 1000, int interpolation = cv::INTER_LINEAR, int boarder_mode = cv::BORDER_CONSTANT, const cv::Scalar& border_color = cv::Scalar(0, 0, 0))
{
	// rotation matrix
	cv::Mat rotMat_3x4;
	composeExternalMatrix(yaw, pitch, roll, 0, 0, Z, rotMat_3x4);

	cv::Mat rotMat = cv::Mat::eye(4, 4, rotMat_3x4.type());
	rotMat_3x4.copyTo(rotMat(cv::Rect(0, 0, 4, 3)));

	// From 2D coordinates to 3D coordinates
	// The center of image is (0,0,0)
	cv::Mat invPerspMat = cv::Mat::zeros(4, 3, CV_64FC1);
	invPerspMat.at<double>(0, 0) = 1;
	invPerspMat.at<double>(1, 1) = 1;
	invPerspMat.at<double>(3, 2) = 1;
	invPerspMat.at<double>(0, 2) = -(double)src.cols / 2;
	invPerspMat.at<double>(1, 2) = -(double)src.rows / 2;

	// 俁師尦嵗昗偐傜俀師尦嵗昗傊摟帇曄姺
	cv::Mat perspMat = cv::Mat::zeros(3, 4, CV_64FC1);
	perspMat.at<double>(0, 0) = Z;
	perspMat.at<double>(1, 1) = Z;
	perspMat.at<double>(2, 2) = 1;

	// 嵗昗曄姺偟丄弌椡夋憸偺嵗昗斖埻傪庢摼
	cv::Mat transMat = perspMat * rotMat * invPerspMat;
	cv::Rect_<double> CircumRect;
	CircumTransImgRect(src.size(), transMat, CircumRect);
	// 弌椡夋憸偲擖椡夋憸偺懳墳儅僢僾傪嶌惉
	cv::Mat map_x, map_y;
	CreateMap(src.size(), CircumRect, rotMat, map_x, map_y);
	cv::remap(src, dst, map_x, map_y, interpolation, boarder_mode, border_color);
}


// Keep center and expand rectangle for rotation
cv::Rect ExpandRectForRotate(const cv::Rect& area)
{
	cv::Rect exp_rect;
	
	int w = cvRound(std::sqrt((double)(area.width * area.width + area.height * area.height)));
	
	exp_rect.width = w;
	exp_rect.height = w;
	exp_rect.x = area.x - (exp_rect.width - area.width) / 2;
	exp_rect.y = area.y - (exp_rect.height - area.height) / 2;

	return exp_rect;
}


void RandomRotateImage(const cv::Mat& src, cv::Mat& dst, float yaw_sigma, float pitch_sigma, float roll_sigma,cv::RNG &rng,const cv::Rect& area,
    float Z, int interpolation, int boarder_mode, const cv::Scalar& boarder_color)
{
    double yaw = rng.gaussian(yaw_sigma);
    double pitch = rng.gaussian(pitch_sigma);
    double roll = rng.gaussian(roll_sigma);
    //double yaw = rng.uniform(-yaw_range / 2, yaw_range / 2);
    //double pitch = rng.uniform(-pitch_range / 2, pitch_range / 2);
    //double roll = rng.uniform(-roll_range / 2, roll_range / 2);

    cv::Rect rect = (area.width <= 0 || area.height <= 0) ? cv::Rect(0, 0, src.cols, src.rows) :
            ExpandRectForRotate(area);
        rect = util::TruncateRectKeepCenter(rect, src.size());

        cv::Mat rot_img;
        RotateImage(src(rect).clone(), rot_img, yaw, pitch, roll, Z, interpolation, boarder_mode, boarder_color);

        cv::Rect dst_area((rot_img.cols - area.width) / 2, (rot_img.rows - area.height) / 2, area.width, area.height);
        dst_area = util::TruncateRectKeepCenter(dst_area, rot_img.size());
        dst = rot_img(dst_area).clone();
    //dst=rot_img.clone();
}
