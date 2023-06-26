#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <iostream>

#include<ray_tracing.hpp>
#include<path_tracing.hpp>
#include<sppm.hpp>
#include<render.hpp>

using namespace std;
using namespace ppm;

int main(int argc, char *argv[])
{
    for (int argNum = 1; argNum < argc; ++argNum)
    {
        std::cout << "Argument " << argNum << " is: " << argv[argNum] << std::endl;
    }

    if (argc != 3)
    {
        cout << "Usage: ./bin/PA1 <input scene file> <output bmp file>" << endl;
        return 1;
    }
    string inputFile = argv[1];
    string outputFile = argv[2]; // only bmp is allowed.

    // SPPM sppm(photon_num, photon_iter, inputFile, outputFile);
    // sppm.renderScene();

    // Ray_Tracing ray_tracing;
    // ray_tracing.renderScene(inputFile, outputFile, 5);

    Path_Tracing path_tracing;
    path_tracing.renderScene(inputFile, outputFile);

    // PT *pt = new PT(inputFile, outputFile);
    // pt->renderScene();
}   
