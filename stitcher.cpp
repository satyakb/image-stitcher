#include <stdio.h>
#include <iostream>
#include <sys/types.h>
#include <dirent.h>
 
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
// #include "opencv2/nonfree/nonfree.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/stitching/stitcher.hpp"
 
using namespace cv;
using namespace std;

int getdir (string dir, vector<string> &files)
{
    DIR *dp;
    struct dirent *dirp;
    if((dp  = opendir(dir.c_str())) == NULL) {
        cout << "Error(" << 1 << ") opening " << dir << endl;
        return 1;
    }

    string prefix = "f";

    while ((dirp = readdir(dp)) != NULL) {
      string name = string(dirp->d_name);
      if (name.substr(0, prefix.size()) == prefix) {
        // files.push_back(name);
        files.insert(files.begin(), name);
      }
    }
    closedir(dp);
    return 0;
}

int main(int argc, char const *argv[])
{
  vector< Mat > imgs;
  Mat pano;
  bool try_use_gpu = false;
  float img_scale = 0.5;

  printf("%d\n", argc);

  if (argc < 1)
  {
    fprintf(stderr, "NOT ENOUGH ARGS\n");
    return 1;
  }

  vector<string> files;
  int num;
  string dir = argv[1];

  getdir(dir, files);

  if (argc > 2) {
    sscanf (argv[2],"%d",&num);
  } else {
    num = (int)files.size();
  }
  printf("NUM: %d\n", num);

  sort(files.begin(), files.end());

  int interval = 3;
  for (int i = 0; i < num; i++) {
    if (i % interval == 0) {
      string file = dir + files[i];
      cout << file << endl;
      Mat img = imread(file);
      resize(img, img, Size(), img_scale, img_scale, CV_INTER_AREA);
      imgs.push_back(img);
    }
  }

  // for (int i = 1; i < argc; ++i)
  // {

  //   Mat img = imread(argv[i]);
  //   // resize(img, img, Size(), img_scale, img_scale, CV_INTER_AREA);

  //   imgs.push_back( img );
  // }

  // imgs.push_back( imread(argv[2]) );
  // imgs.push_back( imread(argv[1]) );
  // vImg.push_back( imread("./stitching_img/S3.jpg") );
  // vImg.push_back( imread("./stitching_img/S4.jpg") );
  // vImg.push_back( imread("./stitching_img/S5.jpg") );
  // vImg.push_back( imread("./stitching_img/S6.jpg") );


  Stitcher stitcher = Stitcher::createDefault(try_use_gpu);
  stitcher.setWarper(new PlaneWarper());
  stitcher.setFeaturesFinder(new detail::SurfFeaturesFinder(1000,3,4,3,4));
  stitcher.setRegistrationResol(0.1);
  stitcher.setSeamEstimationResol(0.1);
  stitcher.setCompositingResol(1);
  stitcher.setPanoConfidenceThresh(1);
  stitcher.setWaveCorrection(true);
  stitcher.setWaveCorrectKind(detail::WAVE_CORRECT_HORIZ);
  stitcher.setFeaturesMatcher(new detail::BestOf2NearestMatcher(false,0.3));
  stitcher.setBundleAdjuster(new detail::BundleAdjusterRay());

  unsigned long AAtime=0, BBtime=0; //check processing time
  AAtime = getTickCount(); //check processing time

  Stitcher::Status status = stitcher.stitch(imgs, pano);

  BBtime = getTickCount(); //check processing time 
  printf("%.2lf sec \n",  (BBtime - AAtime)/getTickFrequency() ); //check processing time

  if (status != Stitcher::OK)
  {
    std::cout << "ERROR STITCHING " << status << std::endl;
  }
  else {
    imshow("Stitching Result", pano);
  }

  waitKey(0);

  return 0;
}