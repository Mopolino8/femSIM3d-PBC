// =================================================================== //
// this is file mainDiskNuCte.cpp, created at 10-Jun-2007              //
// maintained by Gustavo Rabello dos Anjos                             //
// e-mail: gustavo.rabello@gmail.com                                   //
// =================================================================== //

#include <cmath>
#include "Model3D.h"
#include "Simulator3D.h"
#include "InOut.h"
#include "GMRes.h"
#include "PetscSolver.h"
#include "PCGSolver.h"

int main(int argc, char **argv)
{
 PetscInitialize(&argc, &argv, PETSC_NULL, PETSC_NULL);

 int iter = 0;
 real Re = 1;
 real Sc = 2000;
 real cfl = 3;
 Solver *solverP = new PetscSolver(KSPBICG,PCJACOBI);
 Solver *solverV = new PCGSolver();
 Solver *solverC = new PCGSolver();


 const char *mesh = "../../db/mesh/3d/disk5-15-60.vtk";
 const char *txtFolder  = "./txt/";
 const char *binFolder  = "./bin/";
 const char *vtkFolder  = "./vtk/";
 const char *datFolder  = "./dat/";
 const char *simFolder  = "./sim/";
 const char *pertFolder  = "./pert/";

 Model3D m1;
 //m1.setMeshDisk(5,25,80);
 m1.setMeshDisk(5,15,60);
 m1.setAdimenDisk();
 m1.setMiniElement();
 m1.setNuCDiskBC();
 m1.readAndSetPressureDiskBC();
 m1.setCDiskBC();
 m1.setOFace();

 Simulator3D s1(m1);

 s1.setRe(Re); // Reynolds do disco (~1)
 s1.setSc(Sc); // Schmidt da concentracao (~2000)
 s1.setCflDisk(cfl);
 s1.setSolverVelocity(solverV);
 s1.setSolverPressure(solverP);
 s1.setSolverConcentration(solverC);
 s1.init();

 if( (*(argv+1)) == NULL )
 {
  cout << endl;
  cout << "--------------> STARTING FROM 0" << endl;
  cout << endl;
 }
 else if( strcmp( *(argv+1),"restart") == 0 )
 {
  cout << endl;
  cout << "--------------> RE-STARTING..." << endl;
  cout << endl;
  s1.loadSolution(binFolder,"sim-last");
  iter = s1.loadIteration(vtkFolder,"sim-last");
  //s1.loadSolution(binFolder,"UVWPC",70);
  //iter = s1.loadIteration(vtkFolder,"sim",70);
 }

 s1.assembleNuC();
 
 InOut save(m1,s1); // cria objeto de gravacao
 save.saveVTK(vtkFolder,"geometry");
 save.saveInfo("./","info",mesh);
 save.printInfo(mesh);

 int nIter = 1000;
 int nR = 10;
 for( int i=0;i<nIter;i++ )
 {
  for( int j=0;j<nR;j++ )
  {
   cout << "____________________________________ Iteration: " 
	    << i*nR+j+iter << endl;

   s1.matMount();
   s1.matMountC();
   s1.setUnCoupledBC();
   s1.setUnCoupledCBC();
   s1.stepSL();
   s1.setRHS();
   s1.setCRHS();
   s1.unCoupled();
   s1.unCoupledC();
   s1.assembleK();
   save.saveVonKarman(simFolder,"vk1",i*nR+j+iter,4);
   save.saveVonKarman(simFolder,"vk2",i*nR+j+iter,5);
   save.saveVonKarman(simFolder,"vk3",i*nR+j+iter,6);
   save.saveVonKarman(simFolder,"vk4",i*nR+j+iter,7);
   save.saveVonKarman(simFolder,"vk5",i*nR+j+iter,8);
   save.saveVonKarman(simFolder,"vk6",i*nR+j+iter,9);
   save.saveVonKarman(simFolder,"vk7",i*nR+j+iter,10);
   save.saveVTK(vtkFolder,"sim",i*nR+j+iter);
   s1.convergenceCriteria(10E-06);

   cout << "__________________________________________ End: " 
	    << i*nR+j+iter << endl;
  }
  save.saveVTK(vtkFolder,"sim",iter+nR+i*nR-1);
  save.saveSol(binFolder,"UVWPC",iter+nR*+i*nR-1);
 }

 PetscFinalize();
 return 0;
}


