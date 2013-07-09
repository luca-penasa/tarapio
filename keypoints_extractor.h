#ifndef KEYPOINTS_EXTRACTOR_H
#define KEYPOINTS_EXTRACTOR_H

//std
#include <string>

//local
#include "keypoints.h"

//opencv
#include <opencv2/opencv.hpp>
//#include <opencv2/features2d/features2d.hpp>
#include <opencv2/nonfree/features2d.hpp>

using namespace std;

struct KeypointsExtractorConfiguration
{
    KeypointsExtractorConfiguration()
    {
        image_major_side_ = 800;
        image_major_side_low_= 400; //use only when the multi-scale mathod will be required
        method_ = 0; //SIFT only for now
    }

    int image_major_side_;
    int image_major_side_low_;

    int method_; //in a future we could introduce methods other than sift!

    string getAsString()
    {
        stringstream stream;
        stream << image_major_side_ << "_" << method_;
        return stream.str();
    }

    void printStatus()
    {
        cout << "Keypoint extraction options" << endl;
        cout << " - scale factor\t" << image_major_side_ << endl;
        cout << endl;

    }
};

///
/// \brief The KeypointsExtractor class
///
class KeypointsExtractor
{
public:

    KeypointsExtractor()
    {
        config_ = KeypointsExtractorConfiguration();
    }

    void setFilename (const string filename);

    void loadImage();

    void compute();

    Keypoints::Ptr getDescriptors();

    void setScale(int dimension);

    void configure(KeypointsExtractorConfiguration configuration)
    {
        config_ = configuration;
    }

    KeypointsExtractorConfiguration getConfig()
    {
        return  config_;
    }

private:
    typedef shared_ptr<KeypointsExtractor> Ptr;

    string filename_;

    cv::Mat image_, image_low_, descriptors_;

    vector<cv::KeyPoint> cv_keypoints_;

    KeypointsExtractorConfiguration config_;

    float current_scale_;
};


#endif // KEYPOINTS_EXTRACTOR_H
