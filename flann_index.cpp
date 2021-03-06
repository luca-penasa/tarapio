#include "flann_index.h"

template<typename ScalarT>
void FlannIndex<ScalarT>::buildIndex()
{
    cout << "building index..." << endl;
    index_->buildIndex();
    cout << "built!" << endl;
}

template<typename ScalarT>
void FlannIndex<ScalarT>::setInputKeypoints(Keypoints::Ptr kpoints)
{
    keypoints_ = kpoints;

    desc_as_v_ = keypoints_->getAllDescriptorAsStdVector<ScalarT>();
    flann_descriptors_ = flann::Matrix<ScalarT>(&desc_as_v_[0], keypoints_->getNumberOfKeypoints(), keypoints_->getSizeOfFeature());

    flann::KDTreeIndexParams params; //standard parameters for flann
    index_ = flannIndexTypePtr( new flannIndexType(flann_descriptors_, params) );
}

template<typename ScalarT>
vector<Matches::Ptr> FlannIndex<ScalarT>::getMatchesMulti(Keypoints::Ptr points, int nn)
{
    last_query_ = points->getAllDescriptorAsStdVector<ScalarT>();
    flann::Matrix<ScalarT> query_flann (&last_query_[0], points->getNumberOfKeypoints(), points->getSizeOfFeature());

    vector<int> ids(points->getNumberOfKeypoints() * nn);
    vector<float> dists(points->getNumberOfKeypoints() * nn);

    flann::Matrix<int> flann_ids (ids.data(), points->getNumberOfKeypoints(), nn);
    flann::Matrix<float> flann_dists(dists.data(), points->getNumberOfKeypoints(), nn);


    cout << "data points: " << this->keypoints_->keypoints_.size() ;
    cout << " query points: " << points->getNumberOfKeypoints() << endl;


    clock_t begin, end;
    begin = clock();

    flann::SearchParams parameters (config_.n_checks_);
    index_->knnSearch(query_flann, flann_ids, flann_dists, nn, parameters);

    cout << "using checks " << config_.n_checks_ << endl;


    end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    cout << time_spent << endl;


    vector<Matches::Ptr> matches;


    //putting in matches
    for (int i = 0; i < points->getNumberOfKeypoints() ; ++i)
    {
        Matches::Ptr this_matches = Matches::Ptr (new Matches);
        for (int j=0; j < nn; ++j)
        {
            Match match(ids.at(i*nn+j), i);
            match.distance_ = dists.at(i*nn + j);
            this_matches->push_back(match);
        }

        matches.push_back(this_matches);


    }

    cout << "Got " << matches.size() << " matches!" << endl;
    return matches;
}

template<typename ScalarT>
void FlannIndex<ScalarT>::saveIndexToFile(string filename)
{
    index_->save(filename);
}

template<typename ScalarT>
void FlannIndex<ScalarT>::loadIndexFromFile(string filename)
{
    index_ = flannIndexTypePtr ( new flannIndexType (flann_descriptors_, flann::SavedIndexParams(filename)) );
}

template<typename ScalarT>
Keypoints::Ptr FlannIndex<ScalarT>::getTrainKeys()
{
    return keypoints_;
}

//// FORCED INSTANTATION FOR SOME TYPES
template class FlannIndex<unsigned char>;
//template class FlannIndex<float>;
//template class FlannIndex<double>;
//template class FlannIndex<char>;
