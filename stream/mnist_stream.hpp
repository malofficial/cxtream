/*********************************************************
 *  cxtream library
 *
 *  Copyright (c) 2017, Filip Matzner
 *
 *  Use, modification and distribution is subject to the
 *  Boost Software License, Version 1.0.
 *  (see http://www.boost.org/LICENSE_1_0.txt)
 *********************************************************/

#ifndef CXTREAM_MNIST_STREAM_HPP
#define CXTREAM_MNIST_STREAM_HPP

#include <experimental/filesystem>
#include <random>
#include <vector>

#include <cxtream.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <range/v3/all.hpp>

#include "mnist/mnist_reader.hpp"

namespace mnist_stream {

  namespace fs = std::experimental::filesystem;
  using namespace ranges;
  using cxtream::from;
  using cxtream::to;


  /* define columns */


  CXTREAM_DEFINE_COLUMN(images, cv::Mat)
  CXTREAM_DEFINE_COLUMN(labels, uint8_t)


  /* define transformations */


  auto to_cvmat(int rows, int cols, int channels)
  {
    return [rows, cols, channels](const std::vector<uint8_t>& src) {
      assert(src.size() == rows * cols * channels);
      cv::Mat srcmat{src, true};
      return srcmat.reshape(channels, rows);
    };
  }


  template <typename Prng>
  auto cv_rotate(double angle, Prng& prng, cv::Scalar border = cv::Scalar{0, 0, 0})
  {
    return [angle, &prng, border](const cv::Mat& src) {
      std::uniform_real_distribution<> dis(-angle, angle);
      cv::Mat rot_mat = cv::getRotationMatrix2D(
          cv::Point2f(src.cols/2, src.rows/2), dis(prng), 1);
      cv::Mat dst;
      cv::warpAffine(src, dst, rot_mat, src.size(), cv::INTER_CUBIC,
                     cv::BORDER_CONSTANT, border);
      return std::make_tuple(std::move(dst));
    };
  }


  /* define dataset */


  class Dataset
  {
    private:


      fs::path data_root_;
      std::mt19937 prng_;

      std::vector<cv::Mat> train_images_;
      std::vector<cv::Mat> test_images_;
      std::vector<uint8_t> train_labels_;
      std::vector<uint8_t> test_labels_;


    public:


      Dataset(fs::path setup_path)
        // TODO parse YAML setup
        : data_root_{"/tmp"},
          prng_{std::random_device{}()}
      { 

        // TODO add path support to this function
        // read the MNIST dataset using external library
        auto data = mnist::read_dataset<std::vector, std::vector, uint8_t, uint8_t>();

        // convert the dataset into OpenCV images
        train_images_ = data.training_images | view::transform(to_cvmat(28, 28, 1));
        test_images_  = data.test_images | view::transform(to_cvmat(28, 28, 1));
        train_labels_ = std::move(data.training_labels);
        test_labels_  = std::move(data.test_labels);
      }


      /* train stream */


      auto train_stream()
      {
        return
            view::zip(train_images_, train_labels_)
          | cxtream::create<images, labels>
          | cxtream::transform(from<images>, to<images>,
              cv_rotate(30, prng_, cv::Scalar(255, 255, 255)))
          | cxtream::buffer(20)
          | cxtream::batch(100)
          | cxtream::buffer(2);
      }


      /* valid stream */


      auto valid_stream()
      {
        return
            view::zip(test_images_, test_labels_)
          | cxtream::create<images, labels>
          | cxtream::buffer(20)
          | cxtream::batch(100)
          | cxtream::buffer(2);
      }


      /* test stream */


      auto test_stream()
      {
        return valid_stream();
      }

  };

} // end namespace mnist_stream
#endif
