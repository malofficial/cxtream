/// \file
// Stream prototype library
//
//  Copyright Filip Matzner 2017
//
//  Use, modification and distribution is subject to the
//  Boost Software License, Version 1.0.
//  (see http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef STREAM_SIMPLE_STREAM_HPP
#define STREAM_SIMPLE_STREAM_HPP

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <range/v3/view.hpp>
#include <vector>
#include <random>
#include <experimental/filesystem>

namespace stream {

	namespace fs = std::experimental::filesystem;
  namespace view = ranges::view;


  extern std::mt19937 global_prng;


  std::vector<fs::path> list_dir(const fs::path& dataRoot)
	{
		return {fs::directory_iterator(dataRoot), fs::directory_iterator()};
	}


	auto cv_load()
	{
		return view::transform([](const fs::path& p) {
			return cv::imread(p.string());
		});
	}


	auto cv_resize(int width, int height)
	{
		cv::Size size(width, height);
		return view::transform([size](const cv::Mat& src) {
			cv::Mat dst;
			cv::resize(src, dst, size, 0, 0, cv::INTER_CUBIC);
			return dst;
		});
	}


	template <typename Prng>
	auto cv_rotate(double angle, Prng& prng)
	{
		return view::transform([angle, &prng](const cv::Mat& src) {
			std::uniform_real_distribution<> dis(-angle, angle);
			cv::Mat rot_mat = cv::getRotationMatrix2D(
					cv::Point2f(src.cols/2, src.rows/2), dis(prng), 1);
			cv::Mat dst;
			cv::warpAffine(src, dst, rot_mat, src.size(), cv::INTER_CUBIC);
			return dst;
		});
	}


	auto cv_show(int delay = 0)
	{
		return view::for_each([delay](const cv::Mat& img) {
			cv::imshow("Image", img);
			cv::waitKey(delay);
			return ranges::yield(img);
		});
	}


  auto build_stream()
  {
    return list_dir("pictures") | view::shared
                                | view::cycle
                                | cv_load()
                                | cv_resize(256, 256)
                                | cv_rotate(20, global_prng)
                                | cv_show(500);
  }

} //end namespace stream
#endif
