# WeRecCover - Weighted Rectangle Covers of Rectilinear Polygons

## About

This repository contains the algorithm implementations used in *Covering Rectilinear Polygons with Area-Weighted Rectangles*
by Kathrin Hanauer, Martin P. Seybold, Julian Unterweger, which is to appear at the SIAM Symposium on Algorithm Engineering and Experiments (ALENEX24).
If you use our work, please acknowledge it by citing the paper.

## Building

This project was built and tested on Ubuntu 22.04, but these general steps should in theory work on other operating
systems.

To build this project, perform the following steps:

1. [Download](https://github.com/CGAL/cgal/releases/download/v5.5.1/CGAL-5.5.1.tar.xz) and unpack CGAL to a directory (e.g. `/opt/CGAL-5.5.1`) and
   set the variable `CGAL_HOME` to this directory (e.g., `export CGAL_HOME=/opt/CGAL-5.5.1`).
2. Make sure [CGAL's Essential Third Party Libraries](https://doc.cgal.org/latest/Manual/thirdparty.html) are available
   on your system (`sudo apt-get install libgmp3-dev libmpfr-dev libmpfr-doc libmpfr6 libmpfrc++-dev libboost-all-dev` should work)
3. Make sure [CMake 3.16](https://cmake.org/) or above and git are available on your system (`sudo apt install cmake` and
   `sudo apt install git` should be fine)
4. Make sure [Gurobi](https://www.gurobi.com/) is available on your system and has a valid license if you want to use
   the ILP formulations.
   Recompile the Gurobi integration if there are linking issues on your system, e.g., via `cd /opt/gurobi1002/linux64/src/build; make; cp libgurobi_c++.a ../../lib/`.
5. Make sure you have a working C++ compiler (`sudo apt install build-essential` should be good enough)
6. Make sure you have a working internet connection so that CMake can download additional dependencies when needed
7. If you want to generate the documentation, install [Doxygen](https://www.doxygen.nl/), Doxygen may require dot from
   graphviz, so if that is the case and dot is missing install graphviz too (`sudo apt install graphviz` on Ubuntu)
8. From this root directory, execute the following commands: `mkdir build && cd build`
   , `cmake -DCMAKE_BUILD_TYPE=Release ..` and `make`
9. You should be left with `src` and, potentially, `src/doc_doxygen` folders in your working directory

The result should be an executable `src/covering_run`.
Note that the `ilp` and `ilp-pixel` algorithms are only
available if Gurobi was found by CMake, otherwise attempting to use them will lead to an error.

## Example run
The following command-line call will execute the strip algorithm with prune and trim postprocessing on the
polygon(s) described by `instances/caltech/image_0382.wkt` with rectangle creation cost 100 and rectangle area cost 1.
The resulting JSON file will be stored as `./result.json`.

```commandline
    ./covering_run --input instances/caltech/image_0382.wkt --costs 100 1 --algorithm strip --postprocessors prune trim --output result.json
```

## License

*WeReCover* is licensed under MIT license. Please see the `LICENSE` file for further information.

Copyright (c) 2023:
- Julian Unterweger
- Kathrin Hanauer
- Martin Seybold
