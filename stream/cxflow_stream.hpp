/*********************************************************
 *  cxtream library
 *
 *  Copyright (c) 2017, Filip Matzner
 *
 *  Use, modification and distribution is subject to the
 *  Boost Software License, Version 1.0.
 *  (see http://www.boost.org/LICENSE_1_0.txt)
 *********************************************************/

#ifndef CXTREAM_CXFLOW_STREAM_HPP
#define CXTREAM_CXFLOW_STREAM_HPP

#include <fstream>
#include <iostream>
#include <experimental/filesystem>
#include <random>
#include <unordered_map>
#include <vector>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <range/v3/all.hpp>

#include <cxtream.hpp>

namespace cxtream {

  namespace fs = std::experimental::filesystem;
  using namespace ranges;
  using cxtream::from;
  using cxtream::to;

  extern std::mt19937 global_prng;


  /* define columns */


  CXTREAM_DEFINE_COLUMN(fpaths, fs::path)
  CXTREAM_DEFINE_COLUMN(images, cv::Mat)
  CXTREAM_DEFINE_COLUMN(labels, int)
  CXTREAM_DEFINE_COLUMN(str_labels, std::string)


  // TODO separate file
  /* csv utilities */


  std::unordered_map<std::string, std::vector<std::string>>
    load_csv(const fs::path& csv_path, int drop = 0)
  {
    assert(fs::exists(csv_path));
    std::ifstream fin{csv_path.c_str()};
    std::vector<std::vector<std::string>> data =
       getlines(fin)
     | view::drop(drop)
     | view::transform(view::split(','));

    std::unordered_map<std::string, std::vector<std::string>> csv;
    for (std::vector<std::string> row : data) {
      csv.emplace(row[0], row | view::drop(1));
    }
    return csv;
  }


  std::unordered_map<std::string, int> encode_csv_column(
    const std::unordered_map<std::string, std::vector<std::string>>& csv,
    std::size_t column_index)
  {
    // get all the values from the requested column
    std::vector<std::string> values =
        csv
      | view::values
      | view::transform([column_index](std::vector<std::string> row){
          assert(column_index < row.size());
          return row[column_index];
    });

    // take the unique values
    values |= action::sort | action::unique;
    std::unordered_map<std::string, int> map;
    int id = 0;
    for (std::string& value : values) {
      map.emplace(value, id++);
    }
    return map;
  }


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


  /* build dataset */


  class Dataset
  {
    private:

      fs::path data_root_;
      std::unordered_map<std::string, std::vector<std::string>> labels_;
      std::unordered_map<std::string, int> label_enc_;


      auto assign_label()
      {
        return [this](const fs::path& path) {
          std::string str_label = labels_.at(path.stem()).at(0);
          int label = label_enc_.at(str_label);
          return std::make_tuple(label, str_label);
        };
      }


    public:


      Dataset(fs::path data_root)
        // TODO parse YAML
        //: data_root_{std::move(data_root)},
        : data_root_{"/tmp/CIFAR-10"},
          labels_{load_csv(data_root_/"trainLabels.csv", 1)},
          label_enc_{encode_csv_column(labels_, 0)}
      { }


      /* common stream */


      auto common_stream(const fs::path &path)
      {
        return list_dir(path) | view::shared
          | cxtream::create<fpaths>
          | cxtream::transform(from<fpaths>, to<images>, cv_load())
          | cxtream::transform(from<images>, to<images>, cv_resize(28, 28))
          | cxtream::transform(from<fpaths>, to<labels, str_labels>, assign_label())
          | cxtream::batch(100);
      }


      /* python train stream */


      auto create_train_stream_py()
      {
        return make_python_iterator(
          common_stream(data_root_/"train")
            // | cxtream::transform(from<images>, to<images>, cv_rotate(20, global_prng))
            // | cxtream::for_each(from<fpaths, rimage>, cv_show())
        );
      }


      /* python valid stream */


      auto create_valid_stream_py()
      {
        return make_python_iterator(
          common_stream(data_root_/"valid"));
      }


      /* python test stream */


      auto create_test_stream_py()
      {
        return make_python_iterator(
          common_stream(data_root_/"test"));
      }


  }; // end class Dataset


  /* register the dataset for python */
  /* TODO put module registration in here */


} //end namespace cxtream
#endif
