#ifndef IN_FILE_HANDLING_H
#define IN_FILE_HANDLING_H


//#include <regex>

#include <boost/regex.hpp>

#include <dirent.h>


//#include <vector>

using namespace std;

vector<string> read_files_in_dir(string dir_name)
{

    vector<string> list;
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir (dir_name.c_str())) != NULL)
    {
        /* print all the files and directories within directory */
        while ((ent = readdir (dir)) != NULL)
        {
            list.push_back(ent->d_name);
        }
        closedir (dir);
        return list;
    }
}

vector<string> apply_regex(vector<string> in_list , string reg_string)
{
    vector<string> cleaned;

    boost::regex reg (reg_string);


    for (auto s: in_list)
    {   boost::cmatch what;
        if (regex_match(s.c_str(), what, reg))
        {
            cleaned.push_back(s);
        }
    }

    return cleaned;
}



#endif // IN_FILE_HANDLING_H
