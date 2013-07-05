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
        image_scale_factor_ = 0.2;
        method_ = 0; //SIFT only for now
    }

    float image_scale_factor_;
    int method_; //in a future we could introduce methods other than sift!
};

///
/// \brief The KeypointsExtractor class
///
class KeypointsExtractor
{
public:
    void setFilename (const string filename);

    void loadImage();

    void compute();

    Keypoints::Ptr getDescriptors();

    void setScale(float scale);

    void configure(KeypointsExtractorConfiguration configuration)
    {
        this->setScale(configuration.image_scale_factor_);
        //one should also use method for setting it here - for now only SIFT
    }

private:
    typedef shared_ptr<KeypointsExtractor> Ptr;

    string filename_;

    cv::Mat image_, image_low_, descriptors_;

    vector<cv::KeyPoint> cv_keypoints_;

    float scale_;
};


#endif // KEYPOINTS_EXTRACTOR_H
