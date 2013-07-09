#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <memory>


#include <flann/flann.hpp>

#include <sys/stat.h>

#include "keypoints.h"
#include "matches.h"
#include "io.h"
#include "matches_filter.h"
#include "keypoints_extractor.h"
#include "cache_manager.h"

#include "in_file_handling.h"


#include <time.h>





using namespace std;



///
/// \brief test_create_save_load_flann
/// \param filenames
/// \return
///
int test_create_save_load_flann(vector<string> filenames)
{
    string fname = filenames.at(0);
    string flannName = fname + ".flann";
    string keysName = fname + ".keys";

    //compute keys
    KeypointsExtractor extractor;
    extractor.setFilename(fname);
    extractor.loadImage();
    extractor.compute();
    Keypoints::Ptr keys = extractor.getDescriptors();

    //save keys
    KeypointsWriter w;
    w.setFilename(keysName);
    w.setKeypoints(keys);
    w.write();


    // create index and save it
    FlannIndex<unsigned char> flann;
    flann.setInputKeypoints(keys);
    flann.buildIndex();
    flann.saveIndexToFile(flannName);

    //now realod keys
    KeypointsReader r(keysName);
    r.readHeader();
    r.readKeypoints();
    Keypoints::Ptr reloaded_keys = r.getKeypoints();

    //try to setup w flann with saved things
    FlannIndex<unsigned char> newflann;
    newflann.setInputKeypoints(reloaded_keys);
    newflann.loadIndexFromFile(flannName);





}



///
/// \brief test_simple_match_2_images_and_save A SIMPLE TEST CASE OF MY CLASSES
/// \param filenames
/// \return
///
int test_simple_match_2_images_and_save(vector<string> filenames)
{

    KeypointsExtractor extractor;
    extractor.setScale( 0.5 );
    extractor.setFilename(filenames.at(0));
    extractor.loadImage();
    extractor.compute();
    Keypoints::Ptr keys = extractor.getDescriptors();

    KeypointsWriter w;
    w.setFilename("k1.dat");
    w.setKeypoints(keys);
    w.write();

    //try reloading
    KeypointsReader r("k1.dat");
    r.read();

    keys = r.getKeypoints();


    KeypointsExtractor extractor2;
    extractor2.setScale( 0.5 );
    extractor2.setFilename(filenames.at(1));
    extractor2.loadImage();
    extractor2.compute();
    Keypoints::Ptr keys2 = extractor2.getDescriptors();

    w.setFilename("k2.dat");
    w.setKeypoints(keys2);
    w.write();

    KeypointsReader r2("k2.dat");
    r2.read();

    keys2 = r2.getKeypoints();

    FlannIndex<unsigned char> index;
    index.setInputKeypoints(keys);
    index.buildIndex();

    vector<Matches::Ptr > matches  = index.getMatchesMulti(keys2, 2);

    MatchesFilter filter;
    filter.setInputMatches(matches);

    Matches::Ptr good_matches = Matches::Ptr(new  Matches);

    filter.filter(good_matches);

    //try to write out the good matches

    MatchesWriter writer;
    writer.setFilename("matches.txt");
    writer.setMatchesAndKeypoints(good_matches, *keys, *keys2);
    writer.write();

    cout << "Found " << good_matches->size() << " good matches" << endl;

    return 1;
}



//struct Arg: public option::Arg
//{
//    static void printError(const char* msg1, const option::Option& opt, const char* msg2)
//    {
//        fprintf(stderr, "%s", msg1);
//        fwrite(opt.name, opt.namelen, 1, stderr);
//        fprintf(stderr, "%s", msg2);
//    }

//    static option::ArgStatus Unknown(const option::Option& option, bool msg)
//    {
//        if (msg) printError("Unknown option '", option, "'\n");
//        return option::ARG_ILLEGAL;
//    }

//    static option::ArgStatus Required(const option::Option& option, bool msg)
//    {
//        if (option.arg != 0)
//            return option::ARG_OK;

//        if (msg) printError("Option '", option, "' requires an argument\n");
//        return option::ARG_ILLEGAL;
//    }

//    static option::ArgStatus NonEmpty(const option::Option& option, bool msg)
//    {
//        if (option.arg != 0 && option.arg[0] != 0)
//            return option::ARG_OK;

//        if (msg) printError("Option '", option, "' requires a non-empty argument\n");
//        return option::ARG_ILLEGAL;
//    }

//    static option::ArgStatus Numeric(const option::Option& option, bool msg)
//    {
//        char* endptr = 0;
//        if (option.arg != 0 && strtol(option.arg, &endptr, 10)){};
//        if (endptr != option.arg && *endptr == 0)
//            return option::ARG_OK;

//        if (msg) printError("Option '", option, "' requires a numeric argument\n");
//        return option::ARG_ILLEGAL;
//    }
//};


//enum  optionIndex { UNKNOWN , HELP, QUERYIMGS };
//const option::Descriptor usage[] =
//{
//    {UNKNOWN, 0,"" , ""    , Arg::Unknown, "Usage: tarapio master_regexp [options] \n\n"
//     "Options:" },
//    {HELP,    0,"" , "help",option::Arg::None, "  --help  \tPrint usage and exit." },
//    {QUERYIMGS,    0,"q", "query",Arg::Required, "  --query, -q  \tRegExp for query images." },
//    //  {UNKNOWN, 0,"" ,  ""   ,option::Arg::None, "\nExamples:\n"
//    //                                             "  example --unknown -- --this_is_no_option\n"
//    //                                             "  example -unk --plus -ppp file1 file2\n" },
//    {0,0,0,0,0,0}
//};


////////////////////////////////////////////////////////////////////////////////
int
find_argument (int argc, char** argv, const char* argument_name)
{
    for (int i = 1; i < argc; ++i)
    {
        // Search for the string
        if (strcmp (argv[i], argument_name) == 0)
        {
            return (i);
        }
    }
    return (-1);
}





////////////////////////////////////////////////////////////////////////////////
int
parse_argument (int argc, char** argv, const char* str, std::string &val)
{
    int index = find_argument (argc, argv, str) + 1;
    if (index > 0 && index < argc )
        val = argv[index];

    return index - 1;
}

void print_arg(string argname, string argvalue)
{
    cout << "Found argument -" << argname << "- with value " << argvalue << endl;
}

void print_error(string errorstring)
{
    cout << endl << "ERROR: " << errorstring << endl << endl;
}

bool
find_switch (int argc, char** argv, const char* argument_name)
{
    return (find_argument (argc, argv, argument_name) != -1);
}

void print_help()
{
    cout << "USAGE: tarapio \"regex\" [OPTIONS]\t with regex the pattern for the master images" << endl;
    cout << endl;
    cout << "GENERIC OPTIONS are:" << endl;
    cout << "-q\t" << "regex\t" << "a secondary regex to for the images to be used as query images" << endl;
    cout << "-b\t" << "string\t" << "name of the directory to be used for chaching/reading results" << endl;
    cout << "-r\t" << "\t" << "force to recopute/rewrite the matches" << endl;
    cout << "-h \t \t" << "also --help this help message" << endl;

    cout << endl;

    cout << "FOR KEYPOINTS:" << endl;
    cout << "-s\t" << "float\t" << "rescale factor before keypoint extraction with values from 0 to 1" << endl;
    cout << endl;

    cout << "FOR FLANN INDEX SEARCH:" << endl;
    cout << "-c\t" << "int\t" << "number of checks per query. It controls the level of approximation" << endl;
    cout << endl;

    cout << "FOR MATCHES FILTERING:" << endl;
    cout << "-t\t" << "float\t" << "filtering threshold, lower numbers means more secure matches" << endl;

    cout << endl;
    cout << "Note that with master/query option the master images will be matched only against the query images!" << endl;
}

void do_compute_all(vector<string> images, DataDBConfiguration conf)
{
    DataDB datadb;
    datadb.setConfig(conf);
    datadb.setImages(images);
//    datadb.precomputeKeypoints();
//    datadb.precomputeFlannIndices();
    datadb.precomputeMatches();

    cout << "writing out homol files..." << endl;
    datadb.writeOutMatchesToFinalPath("Homol");
}

void do_compute_master_vs_query(vector<string> images, vector<string> query, DataDBConfiguration conf)
{
    DataDB datadb;
    datadb.setConfig(conf);
    datadb.setImages(images);

    datadb.setImagesFromMasterQuery(images, query);

//    datadb.precomputeKeypoints();
//    datadb.precomputeFlannIndices();
    datadb.precomputeMatches();

    cout << "writing out homol files..." << endl;
    datadb.writeOutMatchesToFinalPath("Homol");
}


int main(int argc, char ** argv)
{
    string master_pattern;

    if (argc >= 2)
    {
        master_pattern = argv[1];
    }
    else
    {
        print_error("Cannot find any pattern for master images");
        print_help();
        return -1;
    }

    print_arg("pattern for master images", master_pattern);

    if ((find_switch(argc, argv, "-h")) | (find_switch(argc, argv, "--help")))
    {
        print_help();
        return 1;
    }

    string query_pattern;
    bool use_query;
    int status = parse_argument(argc, argv, "-q", query_pattern);
    if (status > 0)
    {
        use_query = true;
    }
    else
    {
        use_query = false;
    }


    if (use_query & (query_pattern.length() != 0))
    {
        print_arg("pattern for query images" , query_pattern);
    }

    //now find the correspotive images in director
    vector<string> all_files = read_files_in_dir("./");

    vector<string> master_files, query_files;
    master_files= apply_regex(all_files, master_pattern);
    query_files= apply_regex(all_files, query_pattern);

    if (master_files.size() == 0)
    {
        print_error("Cannot find any match for your master images! Wrong pattern?" );
        return -1;
    }

    if (use_query & query_files.size() == 0)
    {
        print_error("Cannot find any match for your query images although you requested it! wrong regexp?" );
        return -1;
    }


    cout << "Images that will be used as masters: "  << endl;

    for (auto s: master_files)
        cout << s << " ";
    cout << endl;

    if (use_query)
    {
        cout << "Images that will be used as query: "  << endl;

        for (auto s: query_files)
            cout << s << " ";
        cout << endl;
    }


    ///// NOW WE HAVE A CERTAIN NUMBER OF PARAMETERS TO CATCH
    DataDBConfiguration conf;

    string tmp;
    parse_argument(argc, argv, "-b", tmp);
    if (tmp.size() != 0)
        conf.cache_dir_ = tmp;

    tmp.clear();
    parse_argument(argc, argv, "-s", tmp);
    if (tmp.size() != 0)
        conf.keys_.image_major_side_ = atoi(tmp.c_str());

    tmp.clear();
    parse_argument(argc, argv, "-c", tmp);
    if (tmp.size() != 0)
        conf.flann_.n_checks_ = atoi(tmp.c_str());

    tmp.clear();
    parse_argument(argc, argv, "-t", tmp);
    if (tmp.size() != 0)
    {
        conf.match_filter_.filter_threshold = atof(tmp.c_str());
        cout << atof(tmp.c_str());
    }

    if (find_switch(argc, argv, "-r"))
        conf.force_matches_rewrite_ = true;


    conf.printStatus();


    if (use_query == false)
    {
        do_compute_all(master_files, conf);
    }
    else
    {
        cout << "HERE" << endl;
        do_compute_master_vs_query(master_files, query_files, conf);
    }







    //// TESTS
    //    test_simple_match_2_images_and_save(filenames);

    //    test_create_save_load_flann(filenames);

    //    test_new_datadb(fnames);

    ///////////////////////////////////////////////////

    //// ACTUAL CODE



    return 1;
}
