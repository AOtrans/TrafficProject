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

#ifndef __UTIL__
#define __UTIL__

#include <opencv2/core/core.hpp>

namespace util{

	//! 夋憸偐傜偼傒弌傞嬮宍偺惍宍
	cv::Rect TruncateRect(const cv::Rect& obj_rect, const cv::Size& img_size);

	//! 夋憸偐傜偼傒弌傞嬮宍傪拞怱傪堐帩偟側偑傜惍宍
	cv::Rect TruncateRectKeepCenter(const cv::Rect& obj_rect, const cv::Size& max_size);

	//! 傾僲僥乕僔儑儞僼傽僀儖偺撉傒崬傒
	/*!
	opencv_createsamles.exe偲摨宍幃偺傾僲僥乕僔儑儞僼傽僀儖撉傒彂偒
	ReadCsvFile()娭悢昁恵
	\param[in] gt_file 傾僲僥乕僔儑儞僼傽僀儖柤
	\param[out] imgpathlist 夋憸僼傽僀儖傊偺僷僗
	\param[out] rectlist 奺夋憸偵偮偗傜傟偨傾僲僥乕僔儑儞偺儕僗僩
	\return 撉傒崬傒偺惉斲
	*/
	bool LoadAnnotationFile(const std::string& gt_file, std::vector<std::string>& imgpathlist, std::vector<std::vector<cv::Rect>>& rectlist);

	//! 傾僲僥乕僔儑儞僼傽僀儖傊捛婰
	/*!
	opencv_createsamles.exe偲摨宍幃偺傾僲僥乕僔儑儞僼傽僀儖撉傒彂偒
	\param[in] anno_file 傾僲僥乕僔儑儞僼傽僀儖柤
	\param[in] img_file 夋憸僼傽僀儖傊偺僷僗
	\param[int] obj_rects 奺夋憸偵偮偗傜傟偨傾僲僥乕僔儑儞偺儕僗僩
	\return 曐懚偺惉斲
	*/
	bool AddAnnotationLine(const std::string& anno_file, const std::string& img_file, const std::vector<cv::Rect>& obj_rects, const std::string& sep);

	// 僨傿儗僋僩儕偐傜夋憸僼傽僀儖柤堦棗傪庢摼
	bool ReadImageFilesInDirectory(const std::string& img_dir, std::vector<std::string>& image_lists);

	bool hasImageExtention(const std::string& filename);

	bool ReadCSVFile(const std::string& input_file, std::vector<std::vector<std::string>>& output_strings,
		const std::vector<std::string>& separater_vec = std::vector<std::string>());;

	std::vector<std::string> TokenizeString(const std::string& input_string, const std::vector<std::string>& separater_vec);
}

#endif
