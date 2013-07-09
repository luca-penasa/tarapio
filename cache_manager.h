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

struct DataDBConfiguration
{
    DataDBConfiguration()
    {
        keys_ = KeypointsExtractorConfiguration();
        flann_ = FlannIndexConfiguration();
        match_filter_ = MatchesFilterConfiguration();

        features_autosave_ = true;
        indices_autosave_ = true;
        matches_autosave_ = true;

        force_matches_rewrite_ = false;


        cache_dir_ = "cache";
    }

    KeypointsExtractorConfiguration keys_;
    FlannIndexConfiguration flann_;
    MatchesFilterConfiguration match_filter_;

    bool features_autosave_, indices_autosave_, matches_autosave_, force_matches_rewrite_;

    string cache_dir_;

    string getKeypointPartAsString()
    {
        return keys_.getAsString() ;
    }

    void printStatus()
    {
        cout << "Overall options" << endl;
        cout << " - cache directory\t"<< cache_dir_ << endl;
        cout << " - forcing matches recomputing?\t" << force_matches_rewrite_ << endl;
        cout << endl;

        keys_.printStatus();
        flann_.printStatus();
        match_filter_.printStatus();

    }

};


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


    void setImagesFromMasterQuery(vector<string> master, vector<string> query)
    {
        std::set<string> aslist;
        for (auto e: master)
            aslist.insert(e);

        for (auto e: query)
            aslist.insert(e);

        std::vector<string> asvec(aslist.size());
        std::copy(aslist.begin(), aslist.end(), asvec.begin());
        //now we have all the images names

        images_ = asvec;
        keypoints_.resize(images_.size());
        flann_indices_.resize(images_.size());
        matches_.setNumberOfImages(images_.size());
        mask_.setFromMasterQuery(master, query);
//        mask_.setUpperTriangularNoDiagonal(); //this is the default
    }

    void ensureCacheDirStructure()
    {
        //ensure all the dirs are setted up
        mkdir(getCacheDir().c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        mkdir(getConfigIdentifierDir().c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

        mkdir(getCurrentCacheDescriptorsDir().c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        mkdir((getConfigIdentifierDir() + "/matches/").c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        mkdir(getCurrentCacheMatchesDir().c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    }

    ImageMatchesMask getCurrentMask()
    {
        return mask_;
    }

    void setImageMask(ImageMatchesMask mask)
    {
        mask_ = mask;
    }

    string getCacheDir()
    {
        return config_.cache_dir_;
    }

    string getConfigIdentifierDir()
    {
        return config_.cache_dir_ + "/" + config_.getKeypointPartAsString();
    }

    string getCurrentCacheDescriptorsDir()
    {
        return config_.cache_dir_ + "/" + config_.getKeypointPartAsString() + "/descriptors/";
    }

    string getCurrentCacheMatchesDir()
    {
        float fthres = config_.match_filter_.filter_threshold;
        int nchecks = config_.flann_.n_checks_;

        stringstream stream;
        stream << fthres << "_" << nchecks;
        string prep = stream.str();


        return config_.cache_dir_ + "/" + config_.getKeypointPartAsString() + "/matches/" + prep + "/";
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

    void setCacheDir(string cache_dir) { config_.cache_dir_ = cache_dir; }

    Keypoints::Ptr getKeypointsForImage( int image_id);

    FlannIndex<>::Ptr getFlannIndexForImage( int image_id);

    Matches::Ptr getMatchesForCouple(int image_a_id, int image_b_id);

    void setFeatureExtractorConfig(KeypointsExtractorConfiguration conf) {config_.keys_ = conf;}

    void setFlannConfig(FlannIndexConfiguration conf) {config_.flann_ = conf;}

    void setMatchesFilterConfig(MatchesFilterConfiguration conf) {config_.match_filter_ = conf;}

    void setConfig(DataDBConfiguration conf){config_ = conf;}

    void setFeaturesAutosave(bool flag) {config_.features_autosave_ = flag;}

    void setFlannIndicesAutosave(bool flag) {config_.indices_autosave_ = flag;}

    void setMatchesAutosave(bool flag) {config_.matches_autosave_ = flag;}

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

//    string cache_dir_;

    vector<Keypoints::Ptr> keypoints_;

    vector<FlannIndex<unsigned char>::Ptr> flann_indices_;

    ImageMatchesMatrix matches_;

    ImageMatchesMask mask_;

    DataDBConfiguration config_;



};

#endif // CACHE_MANAGER_H
