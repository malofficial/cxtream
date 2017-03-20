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
#include <fstream>
#include <iostream>
#include <random>
#include <vector>

#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <cxtream/core.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <range/v3/all.hpp>
#include <yaml-cpp/yaml.h>


namespace mnist_stream {

  namespace fs = std::experimental::filesystem;
  using namespace ranges;
  using cxtream::from;
  using cxtream::to;


  /* define columns */


  CXTREAM_DEFINE_COLUMN(images, cv::Mat)
  CXTREAM_DEFINE_COLUMN(labels, int)
  CXTREAM_DEFINE_COLUMN(id, int)


  /* define transformations */


  // conversion from raw data to opencv matrix
  auto to_cvmat(int rows, int cols, int channels)
  {
    return [rows, cols, channels](const std::vector<uint8_t>& src) {
      assert((int)src.size() == rows * cols * channels);
      cv::Mat srcmat{src, true};
      return srcmat.reshape(channels, rows);
    };
  }


  // opencv matrix rotation
  template <typename Prng>
  auto cv_rotate(double angle, Prng& prng, cv::Scalar border = cv::Scalar{0, 0, 0})
  {
    return [angle, &prng, border](const cv::Mat& src) {
      if (angle == 0.) return std::make_tuple(src);
      std::uniform_real_distribution<> dis(-angle, angle);
      cv::Mat rot_mat = cv::getRotationMatrix2D(
          cv::Point2f(src.cols/2, src.rows/2), dis(prng), 1);
      cv::Mat dst;
      cv::warpAffine(src, dst, rot_mat, src.size(), cv::INTER_CUBIC,
                     cv::BORDER_CONSTANT, border);
      return std::make_tuple(std::move(dst));
    };
  }


  // opencv matrix normalization
  auto cv_normalize(bool enabled = true)
  {
    return [enabled](const cv::Mat& src) {
      if (!enabled) return src;
      cv::Mat dst = src;
      src.convertTo(dst, CV_32FC1, 2. / 255, -1.);
      return dst;
    };
  }


  /* define dataset */


  class Dataset
  {
    private:


      // allocate space for config file
      YAML::Node config_;

      // allocate and seed random generator
      std::mt19937 prng_{std::random_device{}()};

      // allocate space for data
      std::unordered_map<int, int> split_;
      std::unordered_map<int, std::tuple<cv::Mat, int>> data_;

      // load mnist file path
      fs::path mnist_file_ =
        config_["dataset"]["mnist"].as<std::string>();

      // load split csv file path and column id
      fs::path split_file_ =
        config_["dataset"]["split"]["file"].as<std::string>();
      int split_id_ =
        config_["dataset"]["split"]["id"].as<int>();

      // load normalization flag
      double normalize_ =
        config_["dataset"]["normalize"].as<bool>();

      // load batch sizes
      int train_batch_size_ =
        config_["stream"]["train"]["batch_size"].as<int>();
      int valid_batch_size_ =
        config_["stream"]["valid"]["batch_size"].as<int>();
      int test_batch_size_ =
        config_["stream"]["test"]["batch_size"].as<int>();

      // load rotation angle
      double rotate_deg_ =
        config_["stream"]["train"]["rotate"].as<double>();


    public:


      Dataset(fs::path config_path)
        : config_{YAML::LoadFile(config_path)}
      { 
        // load mnist dataset
        cxtream::dataframe<> mnist_df = load_mnist(mnist_file_);

        // decode images
        std::vector<cv::Mat> images = mnist_df.col<std::string>("base64_encoded_img")
          // decode the image from base64
          | view::transform(cxtream::base64_decode)
          // convert it to OpenCV matrix
          | view::transform(to_cvmat(28, 28, 1))
          // normalize the image (if enabled)
          | view::transform(cv_normalize(normalize_));

        // parse labels
        std::vector<int> labels = mnist_df.col<int>("label");

        // parse ids
        std::vector<int> ids = mnist_df.col<int>("id");

        // store the data
        data_ = view::zip(ids, view::zip(images, labels));

        // parse split csv if it exists
        if (fs::exists(split_file_)) {
          split_ = read_split(split_file_, split_id_);
        }
      }


      /* train stream */


      auto train_stream()
      {
        return
          // load and shuffle ids
            get_ids(0)
          | action::shuffle(prng_)
          | view::shared
          // create id column -> switch to cxtream
          | cxtream::create<id>
          // load image and label for the id
          | cxtream::transform(from<id>, to<images, labels>,
              [this](int i){ return this->data_.at(i); })
          // rotate the image
          | cxtream::transform(from<images>, to<images>,
              cv_rotate(rotate_deg_, prng_, cv::Scalar(255, 255, 255)))
          // perform the operations asynchronously
          | cxtream::buffer(20)
          | cxtream::batch(train_batch_size_)
          | cxtream::buffer(2);
      }


      /* valid stream */


      auto noaug_stream(int split_id, int batch_size) const
      {
        // similar to train_stream, but does not shuffle and rotate
        // and uses a different batch size
        return
            get_ids(split_id)
          | view::shared
          | cxtream::create<id>
          | cxtream::transform(from<id>, to<images, labels>,
              [this](int i){ return this->data_.at(i); })
          | cxtream::buffer(20)
          | cxtream::batch(batch_size)
          | cxtream::buffer(2);
      }

      auto valid_stream() const
      {
        return noaug_stream(1, valid_batch_size_);
      }


      /* test stream */


      auto test_stream() const
      {
        return noaug_stream(2, test_batch_size_);
      }


      /* split dataset */


      void split(
        long num_splits,
        double train_ratio,
        double valid_ratio,
        double test_ratio) const
      {
        // generate labels
        auto splits = cxtream::make_many_labels(
          num_splits,
          data_.size(),
          {train_ratio, valid_ratio},
          {test_ratio}
        );

        // generate ids [0, ..., data.size())
        splits.emplace(splits.begin(), view::iota(0, data_.size()));

        // generate header ["split 1", "split 2", ...]
        std::vector<std::string> header{"id"};
        for (long i = 0; i < num_splits; ++i) {
          header.push_back("split " + std::to_string(i));
        }

        // dump csv file
        cxtream::dataframe<> splits_df{splits, header};
        cxtream::write_csv<>(split_file_, splits_df);
      }


      /* read dataset split csv */


      static std::unordered_map<int, int>
      read_split(const fs::path& split_file, long split_id)
      {
        return cxtream::read_csv(split_file).index_irow<int, int>(0, split_id + 1);
      }


      /* read mnist data */ 


      static cxtream::dataframe<> load_mnist(fs::path mnist_file)
      {
        assert(fs::exists(mnist_file) && "Mnist file does not exist");
        namespace bios = boost::iostreams;

        // open mnist file
        std::ifstream mnist_file_stream{mnist_file, std::ios_base::binary};

        // create gzip decompressor
        bios::filtering_istream gz_stream;
        gz_stream.push(bios::gzip_decompressor());
        gz_stream.push(mnist_file_stream);

        // read csv from decompressed stream
        return cxtream::read_csv(gz_stream);
      }


      /* filter ids based on label */ 


      std::vector<int> get_ids(int label) const
      {
        assert(!split_.empty() && "There are no splits loaded, cannot select ids.");
        return
            view::keys(data_)
          | view::filter([this, label](int id){
              return this->split_.at(id) == label;
        });
      }

  };

} // end namespace mnist_stream
#endif
