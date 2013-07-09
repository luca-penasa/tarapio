#ifndef IMAGE_MATCHES_MASK_H
#define IMAGE_MATCHES_MASK_H

//STD
#include <vector>
#include <string>
#include <iostream>
#include <set>

#include "helpers.h"

using namespace std;
///
/// \brief The ImageMatchesMask class
///
class ImageMatchesMask
{
public:
    ImageMatchesMask() {}

    void reset()
    {
        mask_.assign(n_images_ * n_images_, false);
    }

    void setNumberOfImages(size_t n)
    {
        n_images_ = n;
        reset();
    }


    void setImageNames(vector<string> names)
    {
        images_names_ = names;
        setNumberOfImages(images_names_.size());
    }


    void setElement(int i, int j, bool val);


    bool getElement(int i, int j);

    int getIDOfImageName(string imagename)
    {
        for (int i =0; i < images_names_.size(); ++i)
        {
            if (images_names_.at(i) == imagename)
            {
                return i;
            }

        }
        return -1;
    }


    void setUpperTriangularNoDiagonal();

    void setFromMasterQuery(vector<string> master, vector<string> query)
    {
        std::set<string> aslist;
        for (auto e: master)
            aslist.insert(e);

        for (auto e: query)
            aslist.insert(e);

        std::vector<string> asvec(aslist.size());
        std::copy(aslist.begin(), aslist.end(), asvec.begin());
        //now we have all the images names
        setImageNames(asvec);

        for (string m: master)
        {
            int m_id = getIDOfImageName(m);
            for (string q: query)
            {
                int q_id = getIDOfImageName(q);
                if (q_id != m_id)
                {
                    setElement(m_id, q_id, true);
                }
            }
        }
    }

//    void setFromMissingMatchFiles(string directory);


private:
    vector<bool> mask_;

    size_t n_images_;

    vector<string> images_names_;

};

#endif // IMAGE_MATCHES_MASK_H
