#ifndef MATCHES_FILTER_H
#define MATCHES_FILTER_H

//STD
#include <vector>
#include <limits>
#include <sstream>

//LOCAL
#include "matches.h"


using namespace std;

struct MatchesFilterConfiguration
{
    enum FILTER_TYPE {MINIMUM, FIRST_NEAREST};

    MatchesFilterConfiguration()
    {
        filter_threshold = 0.2;
        filter_type_ = FIRST_NEAREST;
    }

    float filter_threshold;
    FILTER_TYPE filter_type_;

    string getAsString()
    {
        stringstream s;
        s << filter_threshold << "_" << filter_type_ ;
        return s.str();
    }

    void printStatus()
    {
        cout << "Matches filter options\t" << endl;
        cout << " - filter threshold\t" << filter_threshold << endl;
        cout << endl;
    }


};


///
/// \brief The MatchesFilter class
///
class MatchesFilter
{

public:
    enum FILTER_TYPE {MINIMUM, FIRST_NEAREST};

    MatchesFilter() {config_ = MatchesFilterConfiguration(); }

    void setInputMatches(vector<Matches::Ptr> multi_matches)
    {
        in_matches_ = multi_matches;
    }


    void setConfig(MatchesFilterConfiguration conf)
    {
        config_ = conf;
    }

    void filter(Matches::Ptr matches)
    {
        switch (config_.filter_type_)
        {
            case MINIMUM:
            {
            float old_dist = std::numeric_limits<float>::max();
            for (int i = 0 ; i < in_matches_.size(); ++i)
            {
                Match match = in_matches_.at(i)->at(0);
                float dist = match.distance_;
                if (old_dist > dist )
                    old_dist = dist;

            }

            float min_distance = old_dist;
            cout << "Found min distance: " << min_distance <<  endl;

            float discriminant_distance = config_.filter_threshold * min_distance;
            for (int i = 0 ; i < in_matches_.size(); ++i)
            {
                Match match = in_matches_.at(i)->at(0);
                if (match.distance_ < discriminant_distance)
                {
                    matches->push_back(match);
                }

            }



            break;
            }


            case FIRST_NEAREST:
            {

            //filtering!
            for (int i = 0; i < in_matches_.size(); ++i)
            {
                Match m1 = in_matches_.at(i)->at(0);
                Match m2 = in_matches_.at(i)->at(1);

//                int nn = in_matches_.at(0).size();
                if (m1.distance_ < m2.distance_ * config_.filter_threshold)
                {

                    matches->push_back(m1);
                }

            }

                break;
            }
            default:
            {
                break;
            }
        }
    }


    vector<Matches::Ptr> in_matches_;


    MatchesFilterConfiguration config_;

};






#endif // MATCHES_FILTER_H
