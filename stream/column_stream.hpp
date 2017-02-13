/// \file
// Stream prototype library
//
//  Copyright Filip Matzner 2017
//
//  Use, modification and distribution is subject to the
//  Boost Software License, Version 1.0.
//  (see http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef STREAM_COLUMN_STREAM_HPP
#define STREAM_COLUMN_STREAM_HPP

#include <vector>
#include <random>
#include <random>
#include <experimental/filesystem>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <range/v3/view.hpp>

#include <column_transform.hpp>

namespace stream {

  namespace fs = std::experimental::filesystem;
  using namespace ranges;
  using namespace stream;

  extern std::mt19937 global_prng;


  /* define columns */


#define STREAM_COLUMNED
  STREAM_DEFINE_COLUMN(fpath, fs::path);
  STREAM_DEFINE_COLUMN(image, cv::Mat);
  STREAM_DEFINE_COLUMN(rimage, cv::Mat);


  /* define augmenters */


  std::vector<fs::path> list_dir(const fs::path& dataRoot)
  {
    return {fs::directory_iterator(dataRoot),
            fs::directory_iterator()};
  }


  auto cv_load()
  {
    return [](const fs::path& p) {
      return std::make_tuple(cv::imread(p.string()));
    };
  }


  auto cv_resize(int width, int height)
  {
    cv::Size size(width, height);
    return [size](const cv::Mat& src) {
      cv::Mat dst;
      cv::resize(src, dst, size, 0, 0, cv::INTER_CUBIC);
      return std::make_tuple(dst);
    };
  }


  template <typename Prng>
  auto cv_rotate(double angle, Prng& prng)
  {
    return [angle, &prng](const cv::Mat& src) {
      std::uniform_real_distribution<> dis(-angle, angle);
      cv::Mat rot_mat = cv::getRotationMatrix2D(
          cv::Point2f(src.cols/2, src.rows/2), dis(prng), 1);
      cv::Mat dst;
      cv::warpAffine(src, dst, rot_mat, src.size(), cv::INTER_CUBIC);
      return std::make_tuple(dst);
    };
  }


  auto cv_show(int delay = 0)
  {
    return [delay](const fs::path& p, const cv::Mat& img) {
      cv::imshow(p.string(), img);
      cv::waitKey(delay);
    };
  }


  /* build stream */


  auto build_stream(const fs::path& path)
  {
    return list_dir(path) | view::shared
      | column_create<fpath>
      | column_transform(from<fpath>, to<image>, cv_load())
      | column_transform(from<image>, to<image>, cv_resize(256, 256))
      | column_transform(from<image>, to<rimage>, cv_rotate(20, global_prng))
      | column_for_each(from<fpath, rimage>, cv_show());
  }


} //end namespace stream
#endif
