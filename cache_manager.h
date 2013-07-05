#ifndef CACHE_MANAGER_H
#define CACHE_MANAGER_H

#include <vector>
#include <string>
#include <sys/stat.h>

//local
#include "keypoints.h"
#include "flann_index.h"
#include "helpers.h"
#include "io.h"
#include "keypoints_extractor.h"
#include "images_matches_matrix.h"
#include "image_matches_mask.h"
#include "matches_filter.h"

using namespace std;

class DataDB

{
public:
    DataDB();

    void setImages(vector<string> images)
    {
        images_ = images;
        keypoints_.resize(images.size());
        flann_indices_.resize(images.size());
        matches_.setNumberOfImages(images_.size());
        mask_.setNumberOfImages(images_.size());
        mask_.setUpperTriangularNoDiagonal(); //this is the default



    }

    void ensureCacheDirStructure()
    {
        //ensure all the dirs are setted up
        mkdir(cache_dir_.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        mkdir((cache_dir_ + "/tarapio/").c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        mkdir((cache_dir_ + "/tarapio/flann/").c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        mkdir((cache_dir_ + "/tarapio/descriptors/").c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        mkdir((cache_dir_ + "/tarapio/matches/").c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    }

    void precomputeKeypoints()
    {
#pragma omp parallel for
        for (int i = 0; i < images_.size() ; ++i)
            getKeypointsForImage(i);
    }

    void precomputeFlannIndices()
    {
#pragma omp parallel for
        for (int i = 0; i < images_.size() ; ++i)
            getFlannIndexForImage(i);
    }

    void precomputeMatches()
    {
        //for each possible match in mask
#pragma omp parallel for
        for (int i = 0 ; i < images_.size(); ++i)
        {
            for (int j = 0; j < images_.size(); ++j)
            {
                if (mask_.getElement(i, j))
                {
                    getMatchesForCouple(i, j);
                }
            }
        }

    }

    void setCacheDir(string cache_dir) { cache_dir_ = cache_dir; }

    Keypoints::Ptr getKeypointsForImage( int image_id);

    FlannIndex<>::Ptr getFlannIndexForImage( int image_id);

    Matches::Ptr getMatchesForCouple(int image_a_id, int image_b_id);

    void setFeatureExtractorConfig(KeypointsExtractorConfiguration conf) {feat_configuration_ = conf;}

    void setFeaturesAutosave(bool flag) {features_autosave_ = true;}

    void setFlannIndicesAutosave(bool flag) {indices_autosave_ = true;}

    void setMatchesAutosave(bool flag) {matches_autosave_ = true;}

    void setImageMatrixMask(ImageMatchesMask mask) {mask_ = mask;}

    void writeOutMatchesToFinalPath(string path)
    {
        string main_dir_name = path + "/";

        mkdir(main_dir_name.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

        for (int i = 0; i < images_.size(); ++i)
        {
            string namea = images_.at(i);

            string this_pict_dir_name = main_dir_name + "Pastis" + namea + "/";
            mkdir(this_pict_dir_name.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

            Keypoints::Ptr keya = getKeypointsForImage(i);

            for (int j =0 ; j < images_.size(); j++)
            {

                if ( (matches_.getMatches(i, j) != NULL ))
                {

                    string nameb = images_.at(j);

                    cout << "WRITE " << namea << " vs " << nameb << endl;
                    Matches::Ptr matches = matches_.getMatches(i, j);

                    Keypoints::Ptr keyb = keypoints_.at(j);

                    string this_dat_filename = this_pict_dir_name + nameb + ".txt";
                    MatchesWriter w;
                    w.setFilename(this_dat_filename);
                    w.setMatchesAndKeypoints(matches, *keya, *keyb);
                    w.write();

                }
            }
        }
    }


private:
    vector<string> images_;

    string cache_dir_;

    vector<Keypoints::Ptr> keypoints_;

    vector<FlannIndex<unsigned char>::Ptr> flann_indices_;

    ImageMatchesMatrix matches_;

    ImageMatchesMask mask_;

    KeypointsExtractorConfiguration feat_configuration_;

    bool features_autosave_, indices_autosave_, matches_autosave_;

    bool force_matches_rebuilding_;
};

#endif // CACHE_MANAGER_H
