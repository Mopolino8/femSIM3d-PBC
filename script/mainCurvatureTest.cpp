// =================================================================== //
// this is file mainRisingBubble.cpp, created at 10-Jun-2009           //
// maintained by Gustavo Rabello dos Anjos                             //
// e-mail: gustavo.rabello@gmail.com                                   //
// =================================================================== //

#include <cmath>
#include "Model3D.h"
#include "Simulator3D.h"
#include "CGSolver.h"
#include "PCGSolver.h"
#include "TElement.h"
#include "GMRes.h"
#include "InOut.h"
#include "Helmholtz3D.h"
#include "PetscSolver.h"
#include "petscksp.h"
#include "colors.h"

#define NUMPHASES 2

int main(int argc, char **argv)
{
 PetscInitialize(&argc,&argv,PETSC_NULL,PETSC_NULL);
 //PetscInitializeNoArguments();

 string meshFile = "0.30.msh";
 
 string meshDir = "../../db/gmsh/3d/sphere/curvature/" + meshFile;
 const char *mesh = meshDir.c_str();
 
 cout << meshDir << endl;

 Model3D m1;

 m1.readMSH(mesh);
 m1.setInterfaceBC();
 m1.setTriEdge();
 m1.mesh2Dto3D();
#if NUMGLEU == 5
 m1.setMiniElement();
#else
 m1.setQuadElement();
#endif
 m1.setOFace();

 m1.setSurfaceConfig();

 InOut i1(m1);

 const char *datFolder  = "./dat/";
 i1.saveKappaErrorSphere(datFolder);

 PetscFinalize();
 return 0;
}


