// =================================================================== //
// this is file main.cpp, created at 10-Jun-2007                       //
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
#include "PetscSolver.h"
#include "petscksp.h"
#include "colors.h"

#define NUMPHASES 2

int main(int argc, char **argv)
{
 PetscInitialize(&argc,&argv,PETSC_NULL,PETSC_NULL);
 
 // set each bubble length
 vector< real > triEdge;
 triEdge.resize(2);
 triEdge[0] = 1.0; // wall
 triEdge[1] = 0.08; // bubble 1 

 // static bubble test (Fabricio's thesis (2005))
 real Re = 100;
 real We = 0.2;
 real c1 = 0.0;  // lagrangian
 real c2 = 0.0;  // smooth vel
 real c3 = 0.0;  // smooth coord (fujiwara)
 real d1 = 1.0;  // surface tangent velocity u_n=u-u_t 
 real d2 = 0.0;  // surface smooth cord (fujiwara)
 real alpha = 1;
 real beta = 1;

 real sigma = 1.0;

 real mu_in = 1.0;
 real mu_out = 0.01;

 real rho_in = 1.0;
 real rho_out = 0.01;

 real cfl = 1.0;

 string meshFile = "staticDumped1.msh";

 Solver *solverP = new PetscSolver(KSPGMRES,PCILU);
 Solver *solverV = new PetscSolver(KSPCG,PCJACOBI);
 Solver *solverC = new PetscSolver(KSPCG,PCICC);

 const char *binFolder  = "./bin/";
 const char *vtkFolder  = "./vtk/";
 const char *mshFolder  = "./msh/";
 const char *datFolder  = "./dat/";
 string meshDir = (string) getenv("DATA_DIR");
 meshDir += "/gmsh/3d/sphere/" + meshFile;
 const char *mesh = meshDir.c_str();

 Model3D m1;
 Simulator3D s1;

 const char *mesh1 = mesh;
 m1.readMSH(mesh1);
 m1.setInterfaceBC();
 m1.setTriEdge(triEdge);
 m1.checkTriangleOrientation();
 m1.mesh2Dto3D();
#if NUMGLEU == 5
 m1.setMiniElement();
#else
 m1.setQuadElement();
#endif
 m1.setOFace();
 m1.setSurfaceConfig();
 m1.setInitSurfaceVolume();
 m1.setInitSurfaceArea();
 m1.setWallBC();

 s1(m1);

 s1.setRe(Re);
 s1.setWe(We);
 s1.setC1(c1);
 s1.setC2(c2);
 s1.setC3(c3);
 s1.setD1(d1);
 s1.setD2(d2);
 s1.setAlpha(alpha);
 s1.setBeta(beta);
 s1.setSigma(sigma);
 //s1.setDtALETwoPhase(dt);
 s1.setMu(mu_in,mu_out);
 s1.setRho(rho_in,rho_out);
 s1.setCfl(cfl);
 s1.init();
 s1.setDtALETwoPhase();
 s1.setSolverPressure(solverP);
 s1.setSolverVelocity(solverV);
 s1.setSolverConcentration(solverC);

 InOut save(m1,s1); // cria objeto de gravacao
 save.saveVTK(vtkFolder,"geometry");
 save.saveVTKSurface(vtkFolder,"geometry");
 save.saveMeshInfo(datFolder);
 save.saveInfo(datFolder,"info",mesh);
 save.printInfo(meshFile.c_str());

 int nIter = 1000;
 for( int i=1;i<nIter;i++ )
 {
  cout << color(none,magenta,black);
  cout << "____________________________________ Iteration: " 
       << i << endl << endl;
  cout << resetColor();

  //s1.stepLagrangian();
  //s1.stepALE();
  s1.stepALEVel();
  s1.setDtALETwoPhase();
  s1.movePoints();
  s1.assemble();
  s1.matMount();
  s1.setUnCoupledBC();
  s1.setRHS();
  //s1.setInterface();
  s1.setInterfaceGeo();
  s1.unCoupled();

  InOut save(m1,s1); // cria objeto de gravacao
  save.saveMSH(mshFolder,"newMesh",i);
  save.saveVTK(vtkFolder,"sim",i);
  save.saveVTKHalf(vtkFolder,"simCutPlane",i);
  save.saveVTKSurface(vtkFolder,"sim",i);
  save.saveSol(binFolder,"sim",i);
  save.saveBubbleInfo(datFolder);
  save.saveParasiticCurrent(datFolder);
  save.chordalPressure(datFolder,"chordalPressure",i);
  //save.saveMeshInfo(datFolder);
  save.printMeshReport();

  s1.saveOldData();

  cout << color(none,magenta,black);
  cout << "________________________________________ END of " 
       << i << endl << endl;;
  cout << resetColor();
 }

 PetscFinalize();
 return 0;
}


