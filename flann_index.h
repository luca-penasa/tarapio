#ifndef FLANN_INDEX_H
#define FLANN_INDEX_H

#include <flann/flann.hpp>
#include <memory>
#include "keypoints.h"
#include <sstream>

using namespace std;

struct FlannIndexConfiguration
{

    FlannIndexConfiguration()
    {
        n_checks_  = 32; //default value
    }

    int n_checks_;
    string getAsString()
    {
        stringstream s;
        s << n_checks_;
        return s.str();

    }

    void printStatus()
    {
        cout << "Flann Index options" << endl;
        cout << " - n checks\t" << n_checks_ << endl;
        cout << endl;
    }

};


template <typename ScalarT = unsigned char>
class FlannIndex
{
public:

    typedef typename flann::L2<ScalarT> flannDistanceType;
    typedef flann::Index<flannDistanceType > flannIndexType;
    typedef shared_ptr<flannIndexType> flannIndexTypePtr;

    typedef std::shared_ptr<FlannIndex> Ptr;

    FlannIndex() {}

    void buildIndex();

    void setInputKeypoints(Keypoints::Ptr kpoints);

    vector<Matches::Ptr > getMatchesMulti(Keypoints::Ptr points, int nn);

    void saveIndexToFile(string filename);

    void loadIndexFromFile(string filename);

    void configure (FlannIndexConfiguration conf) {config_ = conf;}

    Keypoints::Ptr getTrainKeys();


private:
    flannIndexTypePtr index_;
    Keypoints::Ptr keypoints_;

    flann::Matrix<ScalarT> flann_descriptors_;

    vector<ScalarT> desc_as_v_;
    vector<ScalarT> last_query_;

    vector<vector<Keypoint> > matches_;

    FlannIndexConfiguration config_ ;

};


#endif // FLANN_INDEX_H
