#include "cache_manager.h"

DataDB::DataDB()
{

    features_autosave_ = true;
    cache_dir_ = "cache/";
    feat_configuration_ =  KeypointsExtractorConfiguration();
    indices_autosave_ = true;
    force_matches_rebuilding_ == false;

}



Keypoints::Ptr DataDB::getKeypointsForImage( int image_id)
{
    //do we have this keypoint in memory?
    if (keypoints_.at(image_id) != NULL)
    {
        return keypoints_.at(image_id);
    }

    // if it is not in memory see in the cache on disk!
    string image_name = images_.at(image_id);
    string keypoint_fname = cache_dir_ + "/tarapio/descriptors/" + image_name + ".desc";

    if(fexists(keypoint_fname)) //is there in cache?
    {
        //TODO check if it have the right configuration in its header! to be added as a feature
        KeypointsReader reader(keypoint_fname);
        reader.readHeader(); //check in header here

        if (true) //for now always true
        {
            //load this keypoint file
            reader.readKeypoints();
            Keypoints::Ptr keys = reader.getKeypoints();
            keypoints_.at(image_id) = keys;
            return keys;
        }
    }
    else //there is no keypoint in cache -> compute them!
    {
        KeypointsExtractor extractor;
        extractor.setFilename(image_name);
        extractor.loadImage();
        extractor.configure(feat_configuration_);
        extractor.compute();
        Keypoints::Ptr keys = extractor.getDescriptors();

        keypoints_.at(image_id) = keys;

        if (features_autosave_) // save to cache this file
        {
            ensureCacheDirStructure();
            KeypointsWriter w;
            w.setFilename(keypoint_fname);
            w.setKeypoints(keys);
            w.write();
        }


        return keys;
    }
}


FlannIndex<>::Ptr DataDB::getFlannIndexForImage( int image_id)
{
    //do we have this keypoint in memory?
    if (flann_indices_.at(image_id) != NULL)
    {
        return flann_indices_.at(image_id);
    }

    // if it is not in memory see in the cache on disk!
    string image_name = images_.at(image_id);
    string flann_fname = cache_dir_ + "/tarapio/flann/" + image_name + ".flann";

    //create the index object and set its keypoints
    Keypoints::Ptr keys = getKeypointsForImage(image_id);

    if(fexists(flann_fname)) //is there in cache?
    {
        FlannIndex<>::Ptr index = FlannIndex<>::Ptr (new FlannIndex<>);

        index->setInputKeypoints(keys);
        //try to load the file
        index->loadIndexFromFile(flann_fname);

        //TODO catch the exception for which the keys and index do not belong to each other - flann is throwing it!
        if (true) //for now always true
        {
            flann_indices_.at(image_id) = index;
            return index;
        }
    }

    else //there is no index in cache -> compute it!
    {
        FlannIndex<>::Ptr index = FlannIndex<>::Ptr (new FlannIndex<>);

        index->setInputKeypoints(keys);
        index->buildIndex();
        flann_indices_.at(image_id) = index;

        keypoints_.at(image_id) = keys;

        if (indices_autosave_) // save to cache this file
        {
            ensureCacheDirStructure();
            index->saveIndexToFile(flann_fname);
        }


        return index;
    }
}



Matches::Ptr DataDB::getMatchesForCouple(int image_a_id, int image_b_id)
{
    //is this match in memory?
    Matches::Ptr match = matches_.getMatches(image_a_id, image_b_id);
    if (match != NULL)
        return match;

    //maybe this match was previously computed!
    string name_a = images_.at(image_a_id);
    string name_b = images_.at(image_b_id);

    string match_fname = cache_dir_ + "/tarapio/matches/" + name_a + "_vs_" + name_b + ".lock";

    if (fexists(match_fname)) //no need to recompute this match!
    {
        return NULL; //return a null pointer!
    }

    else //we need to compute the match!
    {
        string ima = images_.at(image_a_id);
        string imb = images_.at(image_b_id);

        Keypoints::Ptr kps_a = getKeypointsForImage(image_a_id);
        Keypoints::Ptr kps_b = getKeypointsForImage(image_b_id);

        vector<Matches::Ptr> matches;

        // inverted mode permits to achieve better performances
        // with it we always use the minor number of points as query points
        // while the bigger dataset is used as flann index!

        bool work_in_inverted_mode = (kps_a->keypoints_.size() < kps_b->keypoints_.size());

        if (!work_in_inverted_mode) //normal mode
        {
            //this is the normal situation
            FlannIndex<>::Ptr finder = getFlannIndexForImage(image_a_id);
            matches = finder->getMatchesMulti(kps_b, 2);
        }
        else //simply the inverse thing - we also need to tell the Match object the two ids are inverted!
        {
            FlannIndex<>::Ptr finder = getFlannIndexForImage(image_b_id);
            matches = finder->getMatchesMulti(kps_a, 2);

        }

        //now do some cleaning of these multi-matches (multi means that for each query point we have 2 n-neighbors)
        Matches::Ptr good_matches = Matches::Ptr (new Matches);

        MatchesFilter filter;
        filter.setFilterType(MatchesFilter::FIRST_NEAREST);
        filter.setInputMatches(matches);
        filter.filter(good_matches, 0.2);

        cout << "IM A : " << ima << endl;
        cout << "IM B : " << imb << endl;
        cout << "Found " << good_matches->size() << " matches!" << endl;

        if (!work_in_inverted_mode)
            matches_.setMatches(good_matches, image_a_id, image_b_id);
        else
            matches_.setMatches(good_matches, image_b_id, image_a_id);

        if (matches_autosave_) //write a lock file for this match
        {
            ensureCacheDirStructure();
            std::ofstream outfile (match_fname);
            outfile.close();
        }

        return good_matches;
    }




}
