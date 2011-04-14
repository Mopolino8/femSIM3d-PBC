/**=========================================================================
 *\file Id:Simulator3D.h, GESAR
 *\author Gustavo Rabello dos Anjos
 *\date   23-jun-2007
 *\comment Classe Simulator 3D para o modulo HYDRO
 *\references
 *\version 1.0
 *\updates
 	version    date         author             comment
	1.0        23/08/2007   Gustavo            incio de implementacao
 =========================================================================**/

#ifndef Simulator3D_H
#define Simulator3D_H

#include <iostream>
#include <fstream>
#include "Model3D.h"
#include "FEMMiniElement3D.h"
#include "FEMLinElement3D.h"
#include "Galerkin.h"
#include "SemiLagrangean.h"
#include "MeshSmooth.h"
#include "Solver.h"
#include "CGSolver.h"
#include "PCGSolver.h"
#include "clVector.h"
#include "clMatrix.h"
#include "clDMatrix.h"
#include "Interface3D.h"
#include "colors.h"

class Simulator3D
{
 public:
  Simulator3D(); // construtor padrao
  Simulator3D( Model3D &_m ); // construtor 
  Simulator3D( Model3D &_m, Simulator3D &_s );  // copia
  Simulator3D( Model3D &_mNew, Model3D &_mOld, Simulator3D &_s );  // copia
  virtual ~Simulator3D(); // destrutor padrao

  void getModel3DAttrib(Model3D &_m);
  void allocateMemoryToAttrib();
  void init();
  void initFixedBubbleZ();
  void init2Bubbles();
  void assemble();
  void assembleK();
  void assembleM();
  void assembleC();
  void assembleSlip();
  void assembleNuCte();
  void assembleNuZ();
  void assembleNuC();
  void matMount();
  void matMountC();

  void step();
  void stepSL();
  void stepLagrangian();
  void stepLagrangianZ();
  void stepALE();
  void stepALEVel();
  void stepSmooth();
  void setInterfaceVel();
  void setInterfaceVelNormal();

  void setRHS();
  void setCRHS();
  void setGravity();
  void setGravityBoussinesq();
  void setInterfaceGeo();
  void setInterfaceLevelSet();

  void coupled();
  void unCoupled();
  void unCoupledC();

  void setCoupledBC();
  void setUnCoupledBC();
  void setUnCoupledCBC();

  void setMuZ();
  void setMuC();
  void setRe(real _Re);
  real getRe();
  void setSc(real _Sc);
  real getSc();
  void setFr(real _Fr);
  real getFr();
  void setWe(real _We);
  real getWe();
  void setAlpha(real _alpha);
  real getAlpha();
  void setBeta(real _beta);
  real getBeta();
  void setSigma(real _sigma);
  real getSigma();
  void setDt(real _dt);
  void setTime(real _time);
  real getDt();
  real getTime2();
  real* getTime();
  void setCfl(real _cfl);
  void setCflDisk(real _cfl);
  void setCflBubble(real _cfl);
  real getCfl();
  void setIter(real _Iter);
  int getIter();
  real getC1();
  real getC2();
  real getC3();
  void setMu(real _mu_l);
  void setMu(real _mu_l,real _mu_g);
  void setMuSmooth(real _mu_l,real _mu_g);
  real getMu_l();
  real getMu_g();
  void setRho(real _rho_l);
  void setRho(real _rho_l,real _rho_g);
  void setRho_l(real _rho_l);
  void setRhoSmooth(real _rho_l,real _rho_g);
  void setHSmooth();
  real getRho_l();
  void setRho_g(real _rho_g);
  real getRho_g();
  void setUAnt(clVector &_uAnt);
  void setCSol(clVector &_cSol);
  void setUSol(clVector &_uSol);
  void setVSol(clVector &_vSol);
  void setWSol(clVector &_wSol);
  void updateIEN();
  clVector setCentroid(clVector &_vector);

  void setSolverVelocity(Solver *s);
  void setSolverPressure(Solver *s);
  void setSolverConcentration(Solver *s);

  clVector* getUSol();
  clVector* getVSol();
  clVector* getWSol();
  clVector* getPSol();
  clVector* getCSol();
  clVector* getUALE();
  clVector* getVALE();
  clVector* getWALE();
  clVector* getUAnt();
  clVector* getCAnt();
  clVector* getFint();
  clDMatrix* getKappa();
  clMatrix* getK();
  clMatrix* getM();
  clMatrix* getG();
  clMatrix* getD();
  clMatrix* getGx();
  clMatrix* getGy();
  clMatrix* getGz();
  clVector* getMu();
  clVector* getRho();
  clVector* getHSmooth();
  void operator=(Simulator3D &_s);
  void operator()(Model3D &_m);
  void operator()(Model3D &_m,Simulator3D &_s);
  int loadSolution( const char* _filename,int _iter );
  void applyLinearInterpolation(Model3D &_mOld);
  void getBubbleVelocity();
  void getBubbleVelocity(clVector _uVel,clVector _vVel,clVector _wVel);
  void setALEVelBC();

 private:
  Model3D *m;
  Solver *solverV,*solverP,*solverC;

  int numVerts,numElems,numNodes;
  int numVertsOld,numElemsOld,numNodesOld;
  real Re,Sc,Fr,We,alpha,beta,dt,cfl,time,sigma;
  real c1,c2,c3;
  real bubbleXVel,bubbleYVel,bubbleZVel;
  real rho_l,rho_g,mu_l,mu_g;
  real rho_0,mu_0;
  real rho_lAdimen,rho_gAdimen,mu_lAdimen,mu_gAdimen;
  int iter;
  real triEdge;

  clVector *X,*Y,*Z;
  clVector *uc,*vc,*wc,*pc,*cc;
  clVector *idbcu,*idbcv,*idbcw,*idbcp,*idbcc;
  clVector *outflow,*surface;
  clMatrix *IEN;
  clDMatrix *curvature;
  SurfaceMesh *surfMesh;
  Mesh3D *mesh3d;
  clVector* interfaceDistance;

  clMatrix K,Kc,Mrho,M,Mc,G,D;
  clMatrix mat,matc;

  clDMatrix MrhoLumped,McLumped;

  clMatrix gx,gy,gz;
  clVector uAnt,cAnt;

  clVector va,vcc;
  clVector convUVW,convC;

  clMatrix A;
  clVector b;
  
  clMatrix ATilde,AcTilde,GTilde,DTilde,ETilde,E;
  clDMatrix invA,invC,invMrhoLumped,invMcLumped;
  clDMatrix MLumped,invMLumped;
  clVector uTilde,cTilde,pTilde,b1,b1c,b2,ip,ipc;
  clVector velU,velV,velW,uSol,vSol,wSol,pSol,cSol;
  clVector uSL,vSL,wSL,cSL;
  clVector uALE,vALE,wALE;
  clVector uSmooth,vSmooth,wSmooth,uSmoothCoord,vSmoothCoord,wSmoothCoord;
  clVector uALEOld,vALEOld,wALEOld;
  clVector uSolOld,vSolOld,wSolOld,pSolOld,cSolOld;
  clVector fintOld;
  clDMatrix kappaOld;
  clDMatrix kappa;
  clVector fint;
  clVector mu,muOld,rho,rhoOld;
  clVector Fold;
  clVector hSmooth,hSmoothOld;

};

#endif
