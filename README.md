SemiGlobal-Matching
===================

[![Build Status](https://travis-ci.org/reisub/SemiGlobal-Matching.svg?branch=master)](https://travis-ci.org/reisub/SemiGlobal-Matching)

About
-----

This is a simple implementation of the SGM algorithm with the Birchfield/Tomasi pixelwise matching cost using the OpenCV library. It was created for a school project. It currently runs on one thread only and is relatively slow. Also, there is no postprocessing of the disparity map so there are visible errors in the disparity map.

Resources
---------

1. Hirschmüller, H., Accurate and Efficient Stereo Processing by Semi-Global Matching and Mutual Information, IEEE Conference on Computer Vision and Pattern Recognition, June 2005
2. Hirschmüller, H., Stereo Processing by Semiglobal Matching and Mutual Information, IEEE Transactions on pattern analysis and machine intelligence, Vol. 30, No. 2, February 2008
3. Birchfield, S., Tomasi, C., Depth Discontinuities by Pixel-to-Pixel Stereo, Proceedings of the 1998 IEEE International Conference on Computer Vision, 
4. OpenCV reference manual, release 2.4.9.0,  21.4.2014.
5. Middlebury Stereo Datasets, http://vision.middlebury.edu/stereo/data/, 1.12.2014.
