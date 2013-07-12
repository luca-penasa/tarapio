Tarapio
-------

Tarapio is a tool for tie points computation. Should be a faster and somehow extended alternative to the original tool developed for [MICMAC](http://logiciels.ign.fr/?Telechargement,20), tapioca. 

It gets more speed in this way:

* using [OpenCV](http://opencv.org/) as backend for keypoints and descriptors computation
* using [Flann](http://www.cs.ubc.ca/~mariusm/index.php/FLANN/FLANN) for fast nearest neighbor finding in the feature space. Flann also allows for some level of approximation, permitting to achieve better performance (if an approximation can be accounted)
* using a disk cache for saving temporary files that can be reused at further tarapio runs.
* avoiding to compute two times the same match (if image A is matched with B then the inverse is not repeated)
* If image A have 25245 keypoints while B have 1245 keypoints the image A will be used for constructing the Flann kd-tree and the keypoints of B will be used as query points. And NOT vice-versa. In fact building the index is a fast operation while querying is not so fast.

Dependencies
------------

A couple of dependencies:

* [OpenCV](http://opencv.org/)
* [Flann](http://www.cs.ubc.ca/~mariusm/index.php/FLANN/FLANN) (probably if you have opencv you yet have Flann installed)
* [Boost](http://www.boost.org/) (used for parsing the regex at input)
* [OpenMP](http://openmp.org) that is used for parallelizing the computations
* A recent compiler. Actually some C++2011 standards have been used (mainly `shared_ptr` - the boost version of it could be used actually). I wrote it using gcc-4.7.3, but gcc-4.6.* can compile it

Syntax
------

`tarapio masterimgs [options]`

where `masterimgs` is a regex pattern for matching the master images

tarapio permits to use a `-q` option for specifying a secondary set of query images. If master images are A and B, while query images are C, D and E, it will compute these matches:


    A <-> C
    A <-> D
    A <-> E
    B <-> C
    B <-> D
    B <-> E    
See `tarapio -h` for further options

Notes
-----
- The homologous points are saved in the Homol directory, as expected by Tapas. But remember to call Tapas with the **ExpTxt=1** option! Because the points are saved as ascii files.
- The multiscale approach must still be implemented.
- The caching system is far away from being perfect.


