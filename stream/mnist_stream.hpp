/*********************************************************
 *  cxtream library
 *
 *  Copyright (c) 2017, Filip Matzner
 *
 *  This file is distributed under the MIT License.
 *  See the accompanying file LICENSE.txt for the complete
 *  license agreement.
 *********************************************************/

#ifndef CXTREAM_MNIST_STREAM_HPP
#define CXTREAM_MNIST_STREAM_HPP

#include <experimental/filesystem>
#include <random>
#include <vector>

#include <cxtream/core.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <range/v3/all.hpp>
#include <yaml-cpp/yaml.h>

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
      assert((int)src.size() == rows * cols * channels);
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


      YAML::Node setup_;
      std::mt19937 prng_;

      std::vector<cv::Mat> train_images_;
      std::vector<cv::Mat> test_images_;
      std::vector<uint8_t> train_labels_;
      std::vector<uint8_t> test_labels_;


      int train_batch_size_ =
        setup_["stream"]["train"]["batch_size"].as<int>();
      int valid_batch_size_ =
        setup_["stream"]["valid"]["batch_size"].as<int>();
      int test_batch_size_ =
        setup_["stream"]["test"]["batch_size"].as<int>();


    public:


      Dataset(fs::path setup_path)
        : setup_{YAML::LoadFile(setup_path)},
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
          | cxtream::batch(train_batch_size_)
          | cxtream::buffer(2);
      }


      /* valid stream */


      auto noaug_stream(int batch_size)
      {
        return
            view::zip(test_images_, test_labels_)
          | cxtream::create<images, labels>
          | cxtream::buffer(20)
          | cxtream::batch(batch_size)
          | cxtream::buffer(2);
      }

      auto valid_stream()
      {
        return noaug_stream(valid_batch_size_);
      }


      /* test stream */


      auto test_stream()
      {
        return noaug_stream(test_batch_size_);
      }

  };

} // end namespace mnist_stream
#endif
