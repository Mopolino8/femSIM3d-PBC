// =================================================================== //
// this is file Simulator3D.cpp, created at 23-Ago-2007                //
// maintained by Gustavo Rabello dos Anjos                             //
// e-mail: gustavo.rabello@gmail.com                                   //
// =================================================================== //

#include "Simulator3D.h"

using namespace std;

Simulator3D::Simulator3D( Model3D &_m )  
{
 // mesh information vectors
 m = &_m;
 numVerts = m->getNumVerts();
 numElems = m->getNumElems();
 numNodes = m->getNumNodes();
 numGLEP = m->getNumGLEP();
 numGLEU = m->getNumGLEU();
 numGLEC = m->getNumGLEC();
 X = m->getX();
 Y = m->getY();
 Z = m->getZ();
 uc = m->getUC();
 vc = m->getVC();
 wc = m->getWC();
 pc = m->getPC();
 cc = m->getCC();
 idbcu = m->getIdbcu();
 idbcv = m->getIdbcv();
 idbcw = m->getIdbcw();
 idbcp = m->getIdbcp();
 idbcc = m->getIdbcc();
 outflow = m->getOutflow();
 IEN = m->getIEN();
 surface = m->getSurface();

 // Simulator3D pre-configures parameters
 // =================================================================== //
 // Re   <= 1   -> viscous flow
 // Re   >> 2000 -> turbulent flow
 // ------------------------------------------------------------------- //
 // alpha = 0   -> explicity
 // alpha = 0.5 -> crank-nicholson
 // alpha = 1   -> implicity
 // ------------------------------------------------------------------- //
 // dt          -> time between 2 iterations
 // ------------------------------------------------------------------- //
 // cfl         -> stability condiction (velocity x lenght)
 // ------------------------------------------------------------------- //
 // Solver      -> Pre-Conjugate Gradiente     :PCGSolver
 //             -> Conjugate Gradiente         :CGSolver
 //             -> GMRes                       :GMRes
 //             -> direct method (library: GSL):GSLSolver
 // =================================================================== //
 Re    = 10;
 Sc    = 2;
 Fr    = 0.1;
 We    = 10;
 sigma = 1;
 alpha = 1;
 beta  = 0;
 dt    = 0.01;
 time  = 0.0;
 cfl   = 0.5;
 setSolverVelocity( new PCGSolver() );
 setSolverPressure( new PCGSolver() );
 setSolverConcentration( new PCGSolver() );

 // assembly matrix
 K.Dim( 3*numNodes,3*numNodes );
 Kc.Dim( numVerts,numVerts );
 M.Dim( 3*numNodes,3*numNodes );
 Mc.Dim( numVerts,numVerts );
 MLumped.Dim( 3*numNodes );
 McLumped.Dim( numVerts );
 G.Dim( 3*numNodes,numVerts );
 D.Dim( numVerts,3*numNodes );
 gx.Dim( numNodes,numVerts );
 gy.Dim( numNodes,numVerts );
 gz.Dim( numNodes,numVerts );

 // COUPLED method matrix and vector
 A.Dim( 3*numNodes+numVerts,3*numNodes+numVerts );
 b.Dim( 3*numNodes+numVerts );

 // right hand side vectors
 va.Dim( 3*numNodes );
 vcc.Dim( numVerts );
 b1.Dim( 3*numNodes );
 b1c.Dim( numVerts );
 b2.Dim( numVerts );


 // boundary condiction configured matrix
 ATilde.Dim( 3*numNodes,3*numNodes );
 AcTilde.Dim( numVerts,numVerts );
 GTilde.Dim( 3*numNodes,numVerts );
 DTilde.Dim( numVerts,3*numNodes );
 ETilde.Dim( numVerts,numVerts );
 E.Dim( numVerts, numVerts );

 // K + M matrix set
 mat.Dim( 3*numNodes,3*numNodes );
 matc.Dim( numVerts,numVerts );
 invA.Dim( 3*numNodes );
 invMLumped.Dim( 3*numNodes );
 invC.Dim( numVerts );
 invMcLumped.Dim( numVerts );

 // vetores solucao
 uTilde.Dim( 3*numNodes );
 pTilde.Dim( numVerts );
 cTilde.Dim( numVerts );
 uSol.Dim( numNodes );
 vSol.Dim( numNodes );
 wSol.Dim( numNodes );
 pSol.Dim( numVerts );
 cSol.Dim( numVerts );

 // vetores do termo de conveccao
 convUVW.Dim( 3*numNodes );
 convC.Dim( numVerts );

 // auxiliar vectors
 Fold.Dim( 3*numNodes+numVerts );
 uAnt.Dim( 3*numNodes+numVerts );
 cAnt.Dim( numVerts );
 ip.Dim( 3*numNodes,1 );
 ipc.Dim( numVerts,1 );

 // convective term vectors (semi-lagrangian)
 uSL.Dim( numNodes );
 vSL.Dim( numNodes );
 wSL.Dim( numNodes );
 cSL.Dim( numVerts );

 // convective term vectors (ALE)
 uALE.Dim( numNodes );
 vALE.Dim( numNodes );
 wALE.Dim( numNodes );
 uSmooth.Dim( numNodes );
 vSmooth.Dim( numNodes );
 wSmooth.Dim( numNodes );
 uSmoothCoord.Dim( numNodes );
 vSmoothCoord.Dim( numNodes );
 wSmoothCoord.Dim( numNodes );

 // interface vectors (two-phase)
 distance.Dim( numVerts );
 kappa.Dim( 3*numNodes );
 fint.Dim ( 3*numNodes );
 Hsmooth.Dim( numNodes );
 nu.Dim( numNodes );
 rho.Dim( numNodes );
}

Simulator3D::Simulator3D( Model3D &_m, Simulator3D &_s)  
{
 // mesh information vectors
 m = &_m;
 numVerts = m->getNumVerts();
 numElems = m->getNumElems();
 numNodes = m->getNumNodes();
 numGLEP = m->getNumGLEP();
 numGLEU = m->getNumGLEU();
 numGLEC = m->getNumGLEC();
 X = m->getX();
 Y = m->getY();
 Z = m->getZ();
 uc = m->getUC();
 vc = m->getVC();
 wc = m->getWC();
 pc = m->getPC();
 cc = m->getCC();
 idbcu = m->getIdbcu();
 idbcv = m->getIdbcv();
 idbcw = m->getIdbcw();
 idbcp = m->getIdbcp();
 idbcc = m->getIdbcc();
 outflow = m->getOutflow();
 IEN = m->getIEN();
 surface = m->getSurface();

 Re    = _s.getRe();
 Sc    = _s.getSc();
 Fr    = _s.getFr();
 We    = _s.getWe();
 sigma = _s.getSigma();
 alpha = _s.getAlpha();
 beta  = _s.getBeta();
 dt    = _s.getDt();
 time  = _s.getTime2();
 cfl   = _s.getCfl();

 setSolverVelocity( new PCGSolver() );
 setSolverPressure( new PCGSolver() );
 setSolverConcentration( new PCGSolver() );

 // assembly matrix
 K.Dim( 3*numNodes,3*numNodes );
 Kc.Dim( numVerts,numVerts );
 M.Dim( 3*numNodes,3*numNodes );
 Mc.Dim( numVerts,numVerts );
 MLumped.Dim( 3*numNodes );
 McLumped.Dim( numVerts );
 G.Dim( 3*numNodes,numVerts );
 D.Dim( numVerts,3*numNodes );
 gx.Dim( numNodes,numVerts );
 gy.Dim( numNodes,numVerts );
 gz.Dim( numNodes,numVerts );

 // COUPLED method matrix and vector
 A.Dim( 3*numNodes+numVerts,3*numNodes+numVerts );
 b.Dim( 3*numNodes+numVerts );

 // right hand side vectors
 va.Dim( 3*numNodes );
 vcc.Dim( numVerts );
 b1.Dim( 3*numNodes );
 b1c.Dim( numVerts );
 b2.Dim( numVerts );

 // boundary condiction configured matrix
 ATilde.Dim( 3*numNodes,3*numNodes );
 AcTilde.Dim( numVerts,numVerts );
 GTilde.Dim( 3*numNodes,numVerts );
 DTilde.Dim( numVerts,3*numNodes );
 ETilde.Dim( numVerts,numVerts );
 E.Dim( numVerts, numVerts );

 // K + M matrix set
 mat.Dim( 3*numNodes,3*numNodes );
 matc.Dim( numVerts,numVerts );
 invA.Dim( 3*numNodes );
 invMLumped.Dim( 3*numNodes );
 invC.Dim( numVerts );
 invMcLumped.Dim( numVerts );

 // solution vectors 
 // vetores solucao
 uTilde.Dim( 3*numNodes );
 pTilde.Dim( numVerts );
 cTilde.Dim( numVerts );
 uSol.Dim( numNodes );
 vSol.Dim( numNodes );
 wSol.Dim( numNodes );
 pSol.Dim( numVerts );
 cSol.Dim( numVerts );

 // auxiliar vectors
 Fold.Dim( 3*numNodes+numVerts );
 uAnt.Dim( 3*numNodes+numVerts );
 cAnt.Dim( numVerts );
 ip.Dim( 3*numNodes,1 );
 ipc.Dim( numVerts,1 );

 // convective term vectors
 convUVW.Dim( 3*numNodes );
 convC.Dim( numVerts );

 // convective term vectors (semi-lagrangian)
 uSL.Dim( numNodes );
 vSL.Dim( numNodes );
 wSL.Dim( numNodes );
 cSL.Dim( numVerts );

 // convective term vectors (ALE)
 uALE.Dim( numNodes );
 vALE.Dim( numNodes );
 wALE.Dim( numNodes );
 uSmooth.Dim( numNodes );
 vSmooth.Dim( numNodes );
 wSmooth.Dim( numNodes );
 uSmoothCoord.Dim( numNodes );
 vSmoothCoord.Dim( numNodes );
 wSmoothCoord.Dim( numNodes );

 // interface vectors (two-phase)
 distance.Dim( numVerts );
 kappa.Dim( 3*numNodes );
 fint.Dim ( 3*numNodes );
 Hsmooth.Dim( numNodes );
 nu.Dim( numNodes );
 rho.Dim( numNodes );

 // recuperando campo de velocidade e pressao da malha antiga
 uSolOld = *_s.getUSol();
 vSolOld = *_s.getVSol();
 wSolOld = *_s.getWSol();
 pSolOld = *_s.getPSol();
 uALEOld = *_s.getUALE();
 vALEOld = *_s.getVALE();
 wALEOld = *_s.getWALE();

}

Simulator3D::~Simulator3D()
{
 delete solverV;
 delete solverP;
 delete solverC;
}

void Simulator3D::init()
{
 uSol.CopyFrom( 0,*uc );
 vSol.CopyFrom( 0,*vc );
 wSol.CopyFrom( 0,*wc );
 pSol.CopyFrom( 0,*pc );
 cSol.CopyFrom( 0,*cc );
//--------------------------------------------------
//  for( int i=0;i<numNodes;i++ )
//  {
//   real aux = 1.0;
//   wSol.Set(i,aux);
//  }
//  for( int i=0;i<idbcw->Dim();i++ )
//   wSol.Set( (int) idbcw->Get(i),0.0 ); 
//-------------------------------------------------- 

/* two bubbles */
//--------------------------------------------------
//  for( int i=0;i<numNodes;i++ )
//  {
//   real aux = X->Get(i);
//   uSol.Set(i,aux);
//   aux = -1.0*Y->Get(i);
//   vSol.Set(i,aux);
//   aux = 0.0;
//   wSol.Set(i,aux);
//  }
//-------------------------------------------------- 
}

void Simulator3D::assemble()
{
 int i,j,ii,jj;
 int v1,v2,v3,v4,v5,v[5];
 real aux;
 clMatrix Kxx( numNodes,numNodes );
 clMatrix Kxy( numNodes,numNodes );
 clMatrix Kxz( numNodes,numNodes );
 clMatrix Kyx( numNodes,numNodes );
 clMatrix Kyy( numNodes,numNodes );
 clMatrix Kyz( numNodes,numNodes );
 clMatrix Kzx( numNodes,numNodes );
 clMatrix Kzy( numNodes,numNodes );
 clMatrix Kzz( numNodes,numNodes );
 clMatrix Mx( numNodes,numNodes );
 clMatrix Gx( numNodes,numVerts );
 clMatrix Gy( numNodes,numVerts );
 clMatrix Gz( numNodes,numVerts );
 clMatrix KcMat( numVerts,numVerts );
 clMatrix McMat( numVerts,numVerts );
 FEMMiniElement3D miniElem(*m);
 FEMLinElement3D linElem(*m);

 real eme = 0.81315;

 for( int mele=0;mele<numElems;mele++ )
 {
  v[0]= v1 = (int) IEN->Get(mele,0);
  v[1]= v2 = (int) IEN->Get(mele,1);
  v[2]= v3 = (int) IEN->Get(mele,2);
  v[3]= v4 = (int) IEN->Get(mele,3);
  v[4]= v5 = (int) IEN->Get(mele,4);
  //cout << (float) mele/numElems << endl;
  real c = ( cSol.Get(v1)+
	         cSol.Get(v2)+
	         cSol.Get(v3)+
	         cSol.Get(v4) )*0.25;
  real nuC = exp(eme*c);
  real dif = 1.0/nuC;
  nuC=1.0;
  dif=1.0;

  miniElem.getM(v1,v2,v3,v4,v5);  // para problemas SEM deslizamento
  linElem.getM(v1,v2,v3,v4); 

  for( i=0;i<numGLEU;i++ )
  {
   ii=v[i];
   for( j=0;j<numGLEU;j++ )
   {
	jj=v[j];

	// bloco 11
	aux = Kxx.Get(ii,jj) + nuC*( 2*miniElem.kxx[i][j] + 
	                               miniElem.kyy[i][j] + 
								   miniElem.kzz[i][j] ); 
	Kxx.Set(ii,jj,aux);

	aux = Mx.Get(ii,jj) + miniElem.massele[i][j];
	Mx.Set(ii,jj,aux); // matriz de massa

	// bloco 12
	aux = Kxy.Get(ii,jj) + nuC*( miniElem.kxy[i][j] ); 
	Kxy.Set(ii,jj,aux);

	// bloco 13
	aux = Kxz.Get(ii,jj) + nuC*( miniElem.kxz[i][j] ); 
	Kxz.Set(ii,jj,aux);

	//--------------------------------------------------
	// // bloco 21
	// aux = Kyx.Get(ii,jj) + nuC*( miniElem.kyx[i][j] ); 
	// Kyx.Set(ii,jj,aux);
	//-------------------------------------------------- 

	// bloco 22
	aux = Kyy.Get(ii,jj) + nuC*( miniElem.kxx[i][j] + 
	                           2*miniElem.kyy[i][j] + 
							     miniElem.kzz[i][j] ); 
	Kyy.Set(ii,jj,aux);

	// bloco 23
	aux = Kyz.Get(ii,jj) + nuC*( miniElem.kyz[i][j] ); 
	Kyz.Set(ii,jj,aux);

	//--------------------------------------------------
	// // bloco 31
	// aux = Kzx.Get(ii,jj) + nuC*( miniElem.kzx[i][j] ); 
	// Kzx.Set(ii,jj,aux);
	//-------------------------------------------------- 

	//--------------------------------------------------
	// // bloco 32
	// aux = Kzy.Get(ii,jj) + nuC*( miniElem.kzy[i][j] ); 
	// Kzy.Set(ii,jj,aux);
	//-------------------------------------------------- 

	// bloco 33
	aux = Kzz.Get(ii,jj) + nuC*( miniElem.kxx[i][j] + 
	                             miniElem.kyy[i][j] + 
							   2*miniElem.kzz[i][j] ); 
	Kzz.Set(ii,jj,aux);

   };

   for( j=0;j<numGLEP;j++ )
   {
	jj=v[j];
	
	// bloco 1
	aux = Gx.Get(ii,jj) + miniElem.gxele[i][j];
	Gx.Set(ii,jj,aux);
	// bloco 2
	aux = Gy.Get(ii,jj) + miniElem.gyele[i][j];
	Gy.Set(ii,jj,aux);
	// bloco 3
	aux = Gz.Get(ii,jj) + miniElem.gzele[i][j];
	Gz.Set(ii,jj,aux);
   }
  }
  for( i=0;i<numGLEC;i++ )
  {
   ii=v[i];
   for( j=0;j<numGLEC;j++ )
   {
	jj=v[j];
	aux = KcMat.Get(ii,jj) + dif*( linElem.kxxc[i][j] + 
	                               linElem.kyyc[i][j] + 
								   linElem.kzzc[i][j] );
	KcMat.Set(ii,jj,aux);

	aux = McMat.Get(ii,jj) + linElem.masselec[i][j];
	McMat.Set(ii,jj,aux);
   }
  }
 }
 
 /* GALERKIN */
 // gx = Gx;
 // gy = Gy;
 // gz = Gz;
 M.CopyFrom(          0,          0,              Mx );
 M.CopyFrom(   numNodes,   numNodes,              Mx );
 M.CopyFrom( 2*numNodes, 2*numNodes,              Mx );

//--------------------------------------------------
//  K.CopyFrom(          0,          0,             Kxx );
//  K.CopyFrom(          0,   numNodes,             Kxy );
//  K.CopyFrom(          0, 2*numNodes,             Kxz );
//  K.CopyFrom(   numNodes,          0,             Kyx );
//  K.CopyFrom(   numNodes,   numNodes,             Kyy );
//  K.CopyFrom(   numNodes, 2*numNodes,             Kyz );
//  K.CopyFrom( 2*numNodes,          0,             Kzx );
//  K.CopyFrom( 2*numNodes,   numNodes,             Kzy );
//  K.CopyFrom( 2*numNodes, 2*numNodes,             Kzz );
//-------------------------------------------------- 

 K.CopyFrom(          0,          0,             Kxx );
 K.CopyFrom(          0,   numNodes,             Kxy );
 K.CopyFrom(          0, 2*numNodes,             Kxz );
 K.CopyFrom(   numNodes,          0, Kxy.Transpose() );
 K.CopyFrom(   numNodes,   numNodes,             Kyy );
 K.CopyFrom(   numNodes, 2*numNodes,             Kyz );
 K.CopyFrom( 2*numNodes,          0, Kxz.Transpose() );
 K.CopyFrom( 2*numNodes,   numNodes, Kyz.Transpose() );
 K.CopyFrom( 2*numNodes, 2*numNodes,             Kzz );

//--------------------------------------------------
//  K.CopyFrom(          0,          0,             Kxx );
//  K.CopyFrom(          0,   numNodes, Kyx.Transpose() );
//  K.CopyFrom(          0, 2*numNodes, Kzx.Transpose() );
//  K.CopyFrom(   numNodes,          0,             Kyx );
//  K.CopyFrom(   numNodes,   numNodes,             Kyy );
//  K.CopyFrom(   numNodes, 2*numNodes, Kzy.Transpose() );
//  K.CopyFrom( 2*numNodes,          0,             Kzx );
//  K.CopyFrom( 2*numNodes,   numNodes,             Kzy );
//  K.CopyFrom( 2*numNodes, 2*numNodes,             Kzz );
//-------------------------------------------------- 

 G.CopyFrom(          0,          0,              Gx );
 G.CopyFrom(   numNodes,          0,              Gy );
 G.CopyFrom( 2*numNodes,          0,              Gz );
 D.CopyFrom(          0,          0,  Gx.Transpose() );
 D.CopyFrom(          0,   numNodes,  Gy.Transpose() );
 D.CopyFrom(          0, 2*numNodes,  Gz.Transpose() );
 Kc.CopyFrom(         0,          0,           KcMat );
 Mc.CopyFrom(         0,          0,           McMat );
 
}; // fecha metodo ASSEMBLE

void Simulator3D::assembleNuCte()
{
 int i,j,ii,jj;
 int v1,v2,v3,v4,v5,v[5];
 real aux;
 clMatrix Kxx( numNodes,numNodes );
 clMatrix Mx( numNodes,numNodes );
 clMatrix Gx( numNodes,numVerts );
 clMatrix Gy( numNodes,numVerts );
 clMatrix Gz( numNodes,numVerts );
 clMatrix Dx( numVerts,numNodes );
 clMatrix Dy( numVerts,numNodes );
 clMatrix Dz( numVerts,numNodes );

 FEMMiniElement3D miniElem(*m);

 for( int mele=0;mele<numElems;mele++ )
 {
  v[0]= v1 = (int) IEN->Get(mele,0);
  v[1]= v2 = (int) IEN->Get(mele,1);
  v[2]= v3 = (int) IEN->Get(mele,2);
  v[3]= v4 = (int) IEN->Get(mele,3);
  v[4]= v5 = (int) IEN->Get(mele,4);
  //cout << (float) mele/numElems << endl;

  //miniElem.getM(v1,v2,v3,v4,v5);  // para problemas SEM deslizamento
  miniElem.getMSlip(v1,v2,v3,v4,v5);  // para problemas COM deslizamento

  for( i=0;i<numGLEU;i++ )
  {
   ii=v[i];
   for( j=0;j<numGLEU;j++ )
   {
	jj=v[j];

	// bloco 11
	aux = Kxx.Get(ii,jj) + ( miniElem.kxx[i][j] + 
	                         miniElem.kyy[i][j] + 
							 miniElem.kzz[i][j] ); 
	Kxx.Set(ii,jj,aux);

	aux = Mx.Get(ii,jj) + miniElem.massele[i][j];
	Mx.Set(ii,jj,aux); // matriz de massa
   };

   for( j=0;j<numGLEP;j++ )
   {
	jj=v[j];
	
	// bloco 1
	aux = Gx.Get(ii,jj) + miniElem.gxele[i][j];
	Gx.Set(ii,jj,aux);
	// bloco 2
	aux = Gy.Get(ii,jj) + miniElem.gyele[i][j];
	Gy.Set(ii,jj,aux);
	// bloco 3
	aux = Gz.Get(ii,jj) + miniElem.gzele[i][j];
	Gz.Set(ii,jj,aux);
   }
  }
  for( i=0;i<numGLEP;i++ )
  {
   ii=v[i];
   for( j=0;j<numGLEU;j++ )
   {
	jj=v[j];
	
	// bloco 1
	aux = Dx.Get(ii,jj) + miniElem.dxele[i][j];
	Dx.Set(ii,jj,aux);
	// bloco 2
	aux = Dy.Get(ii,jj) + miniElem.dyele[i][j];
	Dy.Set(ii,jj,aux);
	// bloco 3
	aux = Dz.Get(ii,jj) + miniElem.dzele[i][j];
	Dz.Set(ii,jj,aux);
   }
  }
 }
 
 M.CopyFrom(          0,          0,              Mx );
 M.CopyFrom(   numNodes,   numNodes,              Mx );
 M.CopyFrom( 2*numNodes, 2*numNodes,              Mx );

 K.CopyFrom(          0,          0,             Kxx );
 K.CopyFrom(   numNodes,   numNodes,             Kxx );
 K.CopyFrom( 2*numNodes, 2*numNodes,             Kxx );

 G.CopyFrom(          0,          0,              Gx );
 G.CopyFrom(   numNodes,          0,              Gy );
 G.CopyFrom( 2*numNodes,          0,              Gz );
 D.CopyFrom(          0,          0,              Dx );
 D.CopyFrom(          0,   numNodes,              Dy );
 D.CopyFrom(          0, 2*numNodes,              Dz );
//--------------------------------------------------
//  D.CopyFrom(          0,          0,  Gx.Transpose() );
//  D.CopyFrom(          0,   numNodes,  Gy.Transpose() );
//  D.CopyFrom(          0, 2*numNodes,  Gz.Transpose() );
//-------------------------------------------------- 

}; // fecha metodo ASSEMBLENuCte

void Simulator3D::assembleNuC()
{
 int i,j,ii,jj;
 int v1,v2,v3,v4,v5,v[5];
 real aux;
 clMatrix Kxx( numNodes,numNodes );
 clMatrix Kxy( numNodes,numNodes );
 clMatrix Kxz( numNodes,numNodes );
 clMatrix Kyy( numNodes,numNodes );
 clMatrix Kyz( numNodes,numNodes );
 clMatrix Kzz( numNodes,numNodes );
 clMatrix Mx( numNodes,numNodes );
 clMatrix Gx( numNodes,numVerts );
 clMatrix Gy( numNodes,numVerts );
 clMatrix Gz( numNodes,numVerts );
 clMatrix Dx( numVerts,numNodes );
 clMatrix Dy( numVerts,numNodes );
 clMatrix Dz( numVerts,numNodes );
 clMatrix KcMat( numVerts,numVerts );
 clMatrix McMat( numVerts,numVerts );
 FEMMiniElement3D miniElem(*m);
 FEMLinElement3D linElem(*m);

 for( int mele=0;mele<numElems;mele++ )
 {
  v[0]= v1 = (int) IEN->Get(mele,0);
  v[1]= v2 = (int) IEN->Get(mele,1);
  v[2]= v3 = (int) IEN->Get(mele,2);
  v[3]= v4 = (int) IEN->Get(mele,3);
  v[4]= v5 = (int) IEN->Get(mele,4);
  //cout << (float) mele/numElems << endl;
  real c = ( cSol.Get(v1)+
	         cSol.Get(v2)+
	         cSol.Get(v3)+
	         cSol.Get(v4) )*0.25;

  real eme = 0.81315;
  real nuC = exp(eme*c);
  real dif = 1.0/nuC;

  miniElem.getMSlip(v1,v2,v3,v4,v5);  // para problemas COM deslizamento
  linElem.getM(v1,v2,v3,v4); 

  for( i=0;i<numGLEU;i++ )
  {
   ii=v[i];
   for( j=0;j<numGLEU;j++ )
   {
	jj=v[j];

	// bloco 11
	aux = Kxx.Get(ii,jj) + nuC*( 2*miniElem.kxx[i][j] + 
	                               miniElem.kyy[i][j] + 
								   miniElem.kzz[i][j] ); 
	Kxx.Set(ii,jj,aux);

	aux = Mx.Get(ii,jj) + miniElem.massele[i][j];
	Mx.Set(ii,jj,aux); // matriz de massa

	// bloco 12
	aux = Kxy.Get(ii,jj) + nuC*( miniElem.kxy[i][j] ); 
	Kxy.Set(ii,jj,aux);

	// bloco 13
	aux = Kxz.Get(ii,jj) + nuC*( miniElem.kxz[i][j] ); 
	Kxz.Set(ii,jj,aux);

	// bloco 22
	aux = Kyy.Get(ii,jj) + nuC*( miniElem.kxx[i][j] + 
	                           2*miniElem.kyy[i][j] + 
							     miniElem.kzz[i][j] ); 
	Kyy.Set(ii,jj,aux);

	// bloco 23
	aux = Kyz.Get(ii,jj) + nuC*( miniElem.kyz[i][j] ); 
	Kyz.Set(ii,jj,aux);

	// bloco 33
	aux = Kzz.Get(ii,jj) + nuC*( miniElem.kxx[i][j] + 
	                             miniElem.kyy[i][j] + 
							   2*miniElem.kzz[i][j] ); 
	Kzz.Set(ii,jj,aux);

   };

   for( j=0;j<numGLEP;j++ )
   {
	jj=v[j];
	
	// bloco 1
	aux = Gx.Get(ii,jj) + miniElem.gxele[i][j];
	Gx.Set(ii,jj,aux);
	// bloco 2
	aux = Gy.Get(ii,jj) + miniElem.gyele[i][j];
	Gy.Set(ii,jj,aux);
	// bloco 3
	aux = Gz.Get(ii,jj) + miniElem.gzele[i][j];
	Gz.Set(ii,jj,aux);
   }
  }
  for( i=0;i<numGLEP;i++ )
  {
   ii=v[i];
   for( j=0;j<numGLEU;j++ )
   {
	jj=v[j];
	
	// bloco 1
	aux = Dx.Get(ii,jj) + miniElem.dxele[i][j];
	Dx.Set(ii,jj,aux);
	// bloco 2
	aux = Dy.Get(ii,jj) + miniElem.dyele[i][j];
	Dy.Set(ii,jj,aux);
	// bloco 3
	aux = Dz.Get(ii,jj) + miniElem.dzele[i][j];
	Dz.Set(ii,jj,aux);
   }
  }
  for( i=0;i<numGLEC;i++ )
  {
   ii=v[i];
   for( j=0;j<numGLEC;j++ )
   {
	jj=v[j];
	aux = KcMat.Get(ii,jj) + dif*( linElem.kxxc[i][j] + 
	                               linElem.kyyc[i][j] + 
								   linElem.kzzc[i][j] );
	KcMat.Set(ii,jj,aux);

	aux = McMat.Get(ii,jj) + linElem.masselec[i][j];
	McMat.Set(ii,jj,aux);
   }
  }
 }
 
 /* GALERKIN */
 // gx = Gx;
 // gy = Gy;
 // gz = Gz;
 M.CopyFrom(          0,          0,              Mx );
 M.CopyFrom(   numNodes,   numNodes,              Mx );
 M.CopyFrom( 2*numNodes, 2*numNodes,              Mx );
 K.CopyFrom(          0,          0,             Kxx );
 K.CopyFrom(          0,   numNodes,             Kxy );
 K.CopyFrom(          0, 2*numNodes,             Kxz );
 K.CopyFrom(   numNodes,          0, Kxy.Transpose() );
 K.CopyFrom(   numNodes,   numNodes,             Kyy );
 K.CopyFrom(   numNodes, 2*numNodes,             Kyz );
 K.CopyFrom( 2*numNodes,          0, Kxz.Transpose() );
 K.CopyFrom( 2*numNodes,   numNodes, Kyz.Transpose() );
 K.CopyFrom( 2*numNodes, 2*numNodes,             Kzz );
 G.CopyFrom(          0,          0,              Gx );
 G.CopyFrom(   numNodes,          0,              Gy );
 G.CopyFrom( 2*numNodes,          0,              Gz );
 D.CopyFrom(          0,          0,              Dx );
 D.CopyFrom(          0,   numNodes,              Dy );
 D.CopyFrom(          0, 2*numNodes,              Dz );
 Kc.CopyFrom(         0,          0,           KcMat );
 Mc.CopyFrom(         0,          0,           McMat );
 
}; // fecha metodo ASSEMBLENUC

void Simulator3D::assembleSlip()
{
 int i,j,ii,jj;
 int v1,v2,v3,v4,v5,v[5];
 real aux;
 clMatrix Kxx( numNodes,numNodes );
 clMatrix Kxy( numNodes,numNodes );
 clMatrix Kxz( numNodes,numNodes );
 clMatrix Kyy( numNodes,numNodes );
 clMatrix Kyz( numNodes,numNodes );
 clMatrix Kzz( numNodes,numNodes );
 clMatrix Mx( numNodes,numNodes );
 clMatrix Gx( numNodes,numVerts );
 clMatrix Gy( numNodes,numVerts );
 clMatrix Gz( numNodes,numVerts );
 clMatrix Dx( numVerts,numNodes );
 clMatrix Dy( numVerts,numNodes );
 clMatrix Dz( numVerts,numNodes );
 clMatrix KcMat( numVerts,numVerts );
 clMatrix McMat( numVerts,numVerts );
 FEMMiniElement3D miniElem(*m);
 FEMLinElement3D linElem(*m);

 for( int mele=0;mele<numElems;mele++ )
 {
  v[0]= v1 = (int) IEN->Get(mele,0);
  v[1]= v2 = (int) IEN->Get(mele,1);
  v[2]= v3 = (int) IEN->Get(mele,2);
  v[3]= v4 = (int) IEN->Get(mele,3);
  v[4]= v5 = (int) IEN->Get(mele,4);
  //cout << (float) mele/numElems << endl;

  real nuC = 1.0;
  real dif = 1.0;

  miniElem.getMSlip(v1,v2,v3,v4,v5);  // para problemas COM deslizamento
  linElem.getM(v1,v2,v3,v4); 

  for( i=0;i<numGLEU;i++ )
  {
   ii=v[i];
   for( j=0;j<numGLEU;j++ )
   {
	jj=v[j];

	// bloco 11
	aux = Kxx.Get(ii,jj) + nuC*( 2*miniElem.kxx[i][j] + 
	                               miniElem.kyy[i][j] + 
								   miniElem.kzz[i][j] ); 
	Kxx.Set(ii,jj,aux);

	aux = Mx.Get(ii,jj) + miniElem.massele[i][j];
	Mx.Set(ii,jj,aux); // matriz de massa

	// bloco 12
	aux = Kxy.Get(ii,jj) + nuC*( miniElem.kxy[i][j] ); 
	Kxy.Set(ii,jj,aux);

	// bloco 13
	aux = Kxz.Get(ii,jj) + nuC*( miniElem.kxz[i][j] ); 
	Kxz.Set(ii,jj,aux);

	// bloco 22
	aux = Kyy.Get(ii,jj) + nuC*( miniElem.kxx[i][j] + 
	                           2*miniElem.kyy[i][j] + 
							     miniElem.kzz[i][j] ); 
	Kyy.Set(ii,jj,aux);

	// bloco 23
	aux = Kyz.Get(ii,jj) + nuC*( miniElem.kyz[i][j] ); 
	Kyz.Set(ii,jj,aux);

	// bloco 33
	aux = Kzz.Get(ii,jj) + nuC*( miniElem.kxx[i][j] + 
	                             miniElem.kyy[i][j] + 
							   2*miniElem.kzz[i][j] ); 
	Kzz.Set(ii,jj,aux);

   };

   for( j=0;j<numGLEP;j++ )
   {
	jj=v[j];
	
	// bloco 1
	aux = Gx.Get(ii,jj) + miniElem.gxele[i][j];
	Gx.Set(ii,jj,aux);
	// bloco 2
	aux = Gy.Get(ii,jj) + miniElem.gyele[i][j];
	Gy.Set(ii,jj,aux);
	// bloco 3
	aux = Gz.Get(ii,jj) + miniElem.gzele[i][j];
	Gz.Set(ii,jj,aux);
   }
  }
  for( i=0;i<numGLEP;i++ )
  {
   ii=v[i];
   for( j=0;j<numGLEU;j++ )
   {
	jj=v[j];
	
	// bloco 1
	aux = Dx.Get(ii,jj) + miniElem.dxele[i][j];
	Dx.Set(ii,jj,aux);
	// bloco 2
	aux = Dy.Get(ii,jj) + miniElem.dyele[i][j];
	Dy.Set(ii,jj,aux);
	// bloco 3
	aux = Dz.Get(ii,jj) + miniElem.dzele[i][j];
	Dz.Set(ii,jj,aux);
   }
  }
  for( i=0;i<numGLEC;i++ )
  {
   ii=v[i];
   for( j=0;j<numGLEC;j++ )
   {
	jj=v[j];
	aux = KcMat.Get(ii,jj) + dif*( linElem.kxxc[i][j] + 
	                               linElem.kyyc[i][j] + 
								   linElem.kzzc[i][j] );
	KcMat.Set(ii,jj,aux);

	aux = McMat.Get(ii,jj) + linElem.masselec[i][j];
	McMat.Set(ii,jj,aux);
   }
  }
 }
 
 /* GALERKIN */
 // gx = Gx;
 // gy = Gy;
 // gz = Gz;
 M.CopyFrom(          0,          0,              Mx );
 M.CopyFrom(   numNodes,   numNodes,              Mx );
 M.CopyFrom( 2*numNodes, 2*numNodes,              Mx );
 K.CopyFrom(          0,          0,             Kxx );
 K.CopyFrom(          0,   numNodes,             Kxy );
 K.CopyFrom(          0, 2*numNodes,             Kxz );
 K.CopyFrom(   numNodes,          0, Kxy.Transpose() );
 K.CopyFrom(   numNodes,   numNodes,             Kyy );
 K.CopyFrom(   numNodes, 2*numNodes,             Kyz );
 K.CopyFrom( 2*numNodes,          0, Kxz.Transpose() );
 K.CopyFrom( 2*numNodes,   numNodes, Kyz.Transpose() );
 K.CopyFrom( 2*numNodes, 2*numNodes,             Kzz );
 G.CopyFrom(          0,          0,              Gx );
 G.CopyFrom(   numNodes,          0,              Gy );
 G.CopyFrom( 2*numNodes,          0,              Gz );
 D.CopyFrom(          0,          0,              Dx );
 D.CopyFrom(          0,   numNodes,              Dy );
 D.CopyFrom(          0, 2*numNodes,              Dz );
 Kc.CopyFrom(         0,          0,           KcMat );
 Mc.CopyFrom(         0,          0,           McMat );
 
}; // fecha metodo ASSEMBLESLIP

void Simulator3D::assembleNuZ()
{
 int i,j,ii,jj;
 int v1,v2,v3,v4,v5,v[5];
 real aux;
 clMatrix Kxx( numNodes,numNodes );
 clMatrix Kxy( numNodes,numNodes );
 clMatrix Kxz( numNodes,numNodes );
 clMatrix Kyy( numNodes,numNodes );
 clMatrix Kyz( numNodes,numNodes );
 clMatrix Kzz( numNodes,numNodes );
 clMatrix Mx( numNodes,numNodes );
 clMatrix Gx( numNodes,numVerts );
 clMatrix Gy( numNodes,numVerts );
 clMatrix Gz( numNodes,numVerts );
 clMatrix Dx( numVerts,numNodes );
 clMatrix Dy( numVerts,numNodes );
 clMatrix Dz( numVerts,numNodes );

 setNuZ();      // carregando o arquivo de perfil nuZ
 
 FEMMiniElement3D miniElem(*m);

 for( int mele=0;mele<numElems;mele++ )
 {
  v[0]= v1 = (int) IEN->Get(mele,0);
  v[1]= v2 = (int) IEN->Get(mele,1);
  v[2]= v3 = (int) IEN->Get(mele,2);
  v[3]= v4 = (int) IEN->Get(mele,3);
  v[4]= v5 = (int) IEN->Get(mele,4);
  real nuVar = nu.Get(v5);
  //cout << (float) mele/numElems << endl;

  miniElem.getMSlip(v1,v2,v3,v4,v5); 

  for( i=0;i<numGLEU;i++ )
  {
   ii=v[i];
   for( j=0;j<numGLEU;j++ )
   {
	jj=v[j];

	// bloco 11
	aux = Kxx.Get(ii,jj) + nuVar*( 2*miniElem.kxx[i][j] + 
	                                 miniElem.kyy[i][j] + 
									 miniElem.kzz[i][j] ); 
	Kxx.Set(ii,jj,aux);

	aux = Mx.Get(ii,jj) + miniElem.massele[i][j];
	Mx.Set(ii,jj,aux);

	// bloco 12
	aux = Kxy.Get(ii,jj) + nuVar*( miniElem.kxy[i][j] ); 
	Kxy.Set(ii,jj,aux);

	// bloco 13
	aux = Kxz.Get(ii,jj) + nuVar*( miniElem.kxz[i][j] ); 
	Kxz.Set(ii,jj,aux);

	// bloco 22
	aux = Kyy.Get(ii,jj) + nuVar*( miniElem.kxx[i][j] + 
	                             2*miniElem.kyy[i][j] + 
								   miniElem.kzz[i][j] ); 
	Kyy.Set(ii,jj,aux);

	// bloco 23
	aux = Kyz.Get(ii,jj) + nuVar*( miniElem.kyz[i][j] ); 
	Kyz.Set(ii,jj,aux);

	// bloco 33
	aux = Kzz.Get(ii,jj) + nuVar*( miniElem.kxx[i][j] + 
	                               miniElem.kyy[i][j] + 
								 2*miniElem.kzz[i][j] ); 
	Kzz.Set(ii,jj,aux);

   };
   for( j=0;j<numGLEP;j++ )
   {
	jj=v[j];
	
	// bloco 1
	aux = Gx.Get(ii,jj) + miniElem.gxele[i][j];
	Gx.Set(ii,jj,aux);
	// bloco 2
	aux = Gy.Get(ii,jj) + miniElem.gyele[i][j];
	Gy.Set(ii,jj,aux);
	// bloco 3
	aux = Gz.Get(ii,jj) + miniElem.gzele[i][j];
	Gz.Set(ii,jj,aux);
   }
  }
  for( i=0;i<numGLEP;i++ )
  {
   ii=v[i];
   for( j=0;j<numGLEU;j++ )
   {
	jj=v[j];
	// bloco 1
	aux = Dx.Get(ii,jj) + miniElem.dxele[i][j];
	Dx.Set(ii,jj,aux);
	// bloco 2
	aux = Dy.Get(ii,jj) + miniElem.dyele[i][j];
	Dy.Set(ii,jj,aux);
	// bloco 3
	aux = Dz.Get(ii,jj) + miniElem.dzele[i][j];
	Dz.Set(ii,jj,aux);
   }
  }
 };
 
 gx = Gx;
 gy = Gy;
 gz = Gz;
 M.CopyFrom(          0,          0,             Mx );
 M.CopyFrom(   numNodes,   numNodes,             Mx );
 M.CopyFrom( 2*numNodes, 2*numNodes,             Mx );
 K.CopyFrom(          0,          0,             Kxx );
 K.CopyFrom(          0,   numNodes,             Kxy );
 K.CopyFrom(          0, 2*numNodes,             Kxz );
 K.CopyFrom(   numNodes,          0, Kxy.Transpose() );
 K.CopyFrom(   numNodes,   numNodes,             Kyy );
 K.CopyFrom(   numNodes, 2*numNodes,             Kyz );
 K.CopyFrom( 2*numNodes,          0, Kxz.Transpose() );
 K.CopyFrom( 2*numNodes,   numNodes, Kyz.Transpose() );
 K.CopyFrom( 2*numNodes, 2*numNodes,             Kzz );
 G.CopyFrom(          0,          0,             Gx );
 G.CopyFrom(   numNodes,          0,             Gy );
 G.CopyFrom( 2*numNodes,          0,             Gz );
 D.CopyFrom(          0,          0,             Dx );
 D.CopyFrom(          0,   numNodes,             Dy );
 D.CopyFrom(          0, 2*numNodes,             Dz );
 
}; // fecha metodo ASSEMBLENUZ

void Simulator3D::assembleK()
{
 int i,j,ii,jj;
 int v1,v2,v3,v4,v5,v[5];
 real aux;
 clMatrix Kxx( numNodes,numNodes );
 clMatrix Kxy( numNodes,numNodes );
 clMatrix Kxz( numNodes,numNodes );
 clMatrix Kyy( numNodes,numNodes );
 clMatrix Kyz( numNodes,numNodes );
 clMatrix Kzz( numNodes,numNodes );
 clMatrix KcMat( numVerts,numVerts );
 FEMMiniElement3D miniElem(*m);
 FEMLinElement3D linElem(*m);

 for( int mele=0;mele<numElems;mele++ )
 {
  v[0]= v1 = (int) IEN->Get(mele,0);
  v[1]= v2 = (int) IEN->Get(mele,1);
  v[2]= v3 = (int) IEN->Get(mele,2);
  v[3]= v4 = (int) IEN->Get(mele,3);
  v[4]= v5 = (int) IEN->Get(mele,4);
  //cout << (float) mele/numElems << endl;
  real c = ( cSol.Get(v1)+
	         cSol.Get(v2)+
	         cSol.Get(v3)+
	         cSol.Get(v4) )*0.25;

  real eme = 0.81315;
  real nuC = exp(eme*c);
  real dif = 1.0/nuC;

  miniElem.getK(v1,v2,v3,v4,v5);
  linElem.getK(v1,v2,v3,v4); 

  for( i=0;i<numGLEU;i++ )
  {
   ii=v[i];
   for( j=0;j<numGLEU;j++ )
   {
	jj=v[j];

	// bloco 11
	aux = Kxx.Get(ii,jj) + nuC*( 2*miniElem.kxx[i][j] + 
	                               miniElem.kyy[i][j] + 
								   miniElem.kzz[i][j] ); 
	Kxx.Set(ii,jj,aux);

	// bloco 12
	aux = Kxy.Get(ii,jj) + nuC*( miniElem.kxy[i][j] ); 
	Kxy.Set(ii,jj,aux);

	// bloco 13
	aux = Kxz.Get(ii,jj) + nuC*( miniElem.kxz[i][j] ); 
	Kxz.Set(ii,jj,aux);

	// bloco 22
	aux = Kyy.Get(ii,jj) + nuC*( miniElem.kxx[i][j] + 
	                           2*miniElem.kyy[i][j] + 
							     miniElem.kzz[i][j] ); 
	Kyy.Set(ii,jj,aux);

	// bloco 23
	aux = Kyz.Get(ii,jj) + nuC*( miniElem.kyz[i][j] ); 
	Kyz.Set(ii,jj,aux);

	// bloco 33
	aux = Kzz.Get(ii,jj) + nuC*( miniElem.kxx[i][j] + 
	                             miniElem.kyy[i][j] + 
							   2*miniElem.kzz[i][j] ); 
	Kzz.Set(ii,jj,aux);

   };
  }
  for( i=0;i<numGLEC;i++ )
  {
   ii=v[i];
   for( j=0;j<numGLEC;j++ )
   {
	jj=v[j];
	aux = KcMat.Get(ii,jj) + dif*( linElem.kxxc[i][j] + 
	                               linElem.kyyc[i][j] + 
								   linElem.kzzc[i][j] );
	KcMat.Set(ii,jj,aux);
   }
  }
 }
 
 K.CopyFrom(          0,          0,             Kxx );
 K.CopyFrom(          0,   numNodes,             Kxy );
 K.CopyFrom(          0, 2*numNodes,             Kxz );
 K.CopyFrom(   numNodes,          0, Kxy.Transpose() );
 K.CopyFrom(   numNodes,   numNodes,             Kyy );
 K.CopyFrom(   numNodes, 2*numNodes,             Kyz );
 K.CopyFrom( 2*numNodes,          0, Kxz.Transpose() );
 K.CopyFrom( 2*numNodes,   numNodes, Kyz.Transpose() );
 K.CopyFrom( 2*numNodes, 2*numNodes,             Kzz );
 Kc.CopyFrom(         0,          0,           KcMat );
 
}; // fecha metodo ASSEMBLEK

//este m�todo cria os vetores uSL, vSL e wSL relacionados ao 
//termo convectivo da equa��o de Navier-Stokes. 
//Mais precisamente u \cdot \Nabla u.
//convUVW = uSL, vSL e wSL em um unico vetor.
//Como o metodo eh explicito para calculo do termo convectivo, 
//convUVW vai para setRHS.
void Simulator3D::stepSL()
{

 clVector velU = uSol-uALE;
 clVector velV = vSol-vALE;
 clVector velW = wSol-wALE;

 SemiLagrangean sl(*m,uSol,vSol,wSol,velU,velV,velW,cSol);

 sl.compute(dt);
 uSL = *sl.getUSL();
 vSL = *sl.getVSL();
 wSL = *sl.getWSL();

 convUVW.CopyFrom(0,uSL);
 convUVW.CopyFrom(numNodes,vSL);
 convUVW.CopyFrom(2*numNodes,wSL);
 convC = *sl.getCSL();

//--------------------------------------------------
//  clVector convSurface = sl.computeSurface(dt);
//  convSurface.CopyTo(0,uSLSurface);
//  convSurface.CopyTo(numNodes,vSLSurface);
//  convSurface.CopyTo(2*numNodes,wSLSurface);
//-------------------------------------------------- 
 
} // fecha metodo stepSL 

void Simulator3D::step()
{
 Galerkin galerkin(*m,uSol,vSol,wSol,cSol,gx,gy,gz);

 galerkin.compute(dt);
 clVector uGalerkin = *galerkin.getConvU();
 clVector vGalerkin = *galerkin.getConvV();
 clVector wGalerkin = *galerkin.getConvW();
 convUVW.CopyFrom(0,uGalerkin);
 convUVW.CopyFrom(numNodes,vGalerkin);
 convUVW.CopyFrom(2*numNodes,wGalerkin);
 convC = *galerkin.getConvC();

 clVector uvwSol(3*numNodes);
 uAnt.CopyTo(0,uvwSol);

 // sem correcao na pressao
 va = ( (1.0/dt) * M + (1-alpha) * -(1.0/Re) * K ) * uvwSol;
 
 // com correcao na pressao
 //va = ( (1.0/dt) * M + (1-alpha) * -(1.0/Re) * K ) * uvwSol - (G*pSol);

 // ainda nao funcionando
 //vcc = ( (1.0/dt) * Mc + (1-alpha) * -(1.0/(Re*Sc)) * Kc ) * cSol;
 vcc = ( (1.0/dt) * McLumped ) * cSol;

 va = va + convUVW;
 vcc = vcc + convC;
} // fecha metodo step 

// metodo para movimentacao dos pontos da malha atraves da velocidade do
// escoamento (uSol, vSol e wSol), caracterizando a movimentacao
// puramente lagrangiana
void Simulator3D::stepLagrangian()
{
 m->setX(*m->getX()+(uSol*dt));
 m->setY(*m->getY()+(vSol*dt));
 m->setZ(*m->getZ()+(wSol*dt));

 //assemble();
 assembleSlip();

 convUVW.CopyFrom(0,uSol);
 convUVW.CopyFrom(numNodes,vSol);
 convUVW.CopyFrom(2*numNodes,wSol);
 convC = cSol;

} // fecha metodo stepLagrangian

// metodo para movimentacao dos pontos da malha na direcao Z atraves da 
// velocidade do escoamento (wSol), caracterizando a movimentacao
// puramente lagrangiana em Z. Para os pontos nas direcoes X e Y eh
// utilizado o metodo explicito semi lagrangiano
void Simulator3D::stepLagrangianZ()
{
 m->setZ(*m->getZ()+(wSol*dt));

//--------------------------------------------------
//  cout << uSol.Norm() << endl;
//  cout << vSol.Norm() << endl;
//  cout << wSol.Norm() << endl;
//-------------------------------------------------- 

 //SemiLagrangean sl(*m,uSol,vSol,wSol,cSol);
 SemiLagrangean sl(*m,uSol,vSol,wSol,velU,velV,velW,cSol);

 sl.computeFreeSurface(dt);
 uSL = *sl.getUSL();
 vSL = *sl.getVSL();
 //wSL = *sl.getWSL();
 convC = *sl.getCSL();

 // testar!
 convUVW.CopyFrom(0,uSL);
 convUVW.CopyFrom(numNodes,vSL);
 convUVW.CopyFrom(2*numNodes,wSol);

 // atualizacao de todas as matrizes do sistema
 //assemble();
 //assembleSlip();

} // fecha metodo stepLagragianZ

void Simulator3D::stepALE()
{
 // calcula velocidade elastica - dependente das coordenadas dos pontos
 stepSmooth();

 c1 = 1.0; 
 c2 = 1.0;
 c3 = 0.0; // uSLSurface vSLSurface apresentam problema para c3=1.0

 uALE = c1*uSol+c2*uSmooth;
 vALE = c1*vSol+c2*vSmooth;
 wALE = c1*wSol+c2*wSmooth;

//--------------------------------------------------
//  // atualiza ALE nos pontos de contorno 
//  int aux;
//  for( int i=0;i<idbcu->Dim();i++ )
//  {
//   aux = (int) idbcu->Get(i);
//   uALE.Set( aux,0.0 ); 
//   vALE.Set( aux,0.0 ); 
//   wALE.Set( aux,0.0 ); 
//  }
// 
//  for( int i=0;i<idbcv->Dim();i++ )
//  {
//   aux = (int) idbcv->Get(i);
//   uALE.Set( aux,0.0 ); 
//   vALE.Set( aux,0.0 ); 
//   wALE.Set( aux,0.0 ); 
//  }
// 
//  for( int i=0;i<idbcw->Dim();i++ )
//  {
//   aux = (int) idbcw->Get(i);
//   uALE.Set( aux,0.0 ); 
//   vALE.Set( aux,0.0 ); 
//   wALE.Set( aux,0.0 ); 
//  }
// 
//  for( int i=0;i<idbcp->Dim();i++ )
//  {
//   aux = (int) idbcp->Get(i);
//   uALE.Set( aux,0.0 ); 
//   vALE.Set( aux,0.0 ); 
//   wALE.Set( aux,0.0 ); 
//  }
//-------------------------------------------------- 

 // impoe velocidade do fluido na interface
 //setInterfaceVel();

 // calcula velocidade do fluido atraves do metodo semi-lagrangeano
 stepSL();

 // movimentando os pontos da malha com velocidade ALE
 m->setX(*m->getX()+(uALE*dt));
 m->setY(*m->getY()+(vALE*dt));
 m->setZ(*m->getZ()+(wALE*dt));

 // atualizacao de todas as matrizes do sistema
 //assemble();
 assembleSlip();

 convUVW.CopyFrom(0,uSL);
 convUVW.CopyFrom(numNodes,vSL);
 convUVW.CopyFrom(2*numNodes,wSL);
 convC = cSol;

} // fecha metodo stepALE

void Simulator3D::stepALEVel()
{
 setALEVelBC();

 // calcula velocidade elastica - dependente das velocidades dos pontos
 setInterfaceVelNormal();
 for( int i=0;i<1;i++ )
 {
  // smoothing - coordenadas
  MeshSmooth e1(*m,dt); // criando objeto MeshSmooth
  e1.stepSmoothSurface();
  uSmoothCoord = *e1.getUSmooth();
  vSmoothCoord = *e1.getVSmooth();
  wSmoothCoord = *e1.getWSmooth();

  // smoothing - velocidade
  MeshSmooth e2(*m,dt); // criando objeto MeshSmooth
  e2.stepSmooth(uALE,vALE,wALE);
  e2.setCentroid();
  uSmooth = *e2.getUSmooth();
  vSmooth = *e2.getVSmooth();
  wSmooth = *e2.getWSmooth();

  // impoe velocidade (componente normal) do fluido na interface
  setInterfaceVelNormal();
 }

 c1 = 0.0; 
 c2 = 1.0;
 c3 = 0.0; // uSLSurface vSLSurface apresentam problema para c3=1.0

 uALE = c1*uSol+c2*uSmooth;
 vALE = c1*vSol+c2*vSmooth;
 wALE = c1*wSol+c2*wSmooth;

 // impoe velocidade ALE = 0 no contorno
 setALEVelBC();

 // impoe velocidade (componente normal) do fluido na interface
 setInterfaceVelNormal();

 // calcula velocidade do fluido atraves do metodo semi-lagrangeano
 stepSL();

 // movimentando os pontos da malha com velocidade ALE
 m->setX(*m->getX()+(uALE*dt));
 m->setY(*m->getY()+(vALE*dt));
 m->setZ(*m->getZ()+(wALE*dt));
 
 // atualizacao de todas as matrizes do sistema
 assemble();
 //assembleSlip();

} // fecha metodo stepALE3

void Simulator3D::stepSmooth()
{
 MeshSmooth e1(*m,dt); // criando objeto MeshSmooth

 clVector smooth = e1.compute();
 smooth.CopyTo(0,uSmooth);
 smooth.CopyTo(numNodes,vSmooth);
 smooth.CopyTo(2*numNodes,wSmooth);

} // fecha metodo stepSmooth

void Simulator3D::setInterfaceVel()
{
 //real bubbleZVelocity = getBubbleVelocity();

 for( int i=0;i<surface->Dim();i++ )
 {
  int aux = surface->Get(i);
  uALE.Set(aux,uSol.Get(aux));
  vALE.Set(aux,vSol.Get(aux));
  wALE.Set(aux,wSol.Get(aux));
  
  //wALE.Set(aux,wSol.Get(aux)-bubbleZVelocity);

 }
} // fecha metodo setInterfaceVel 

void Simulator3D::setInterfaceVelNormal()
{
 list<int> plist,plist2;
 list<int>::iterator face,vert;
 vector< list<int> > *elemSurface = m->getElemSurface();
 vector< list<int> > *neighbourFaceVert = m->getNeighbourFaceVert();

 //real bubbleZVelocity = getBubbleVelocity();

 for( int i=0;i<surface->Dim();i++ )
 {
  int surfaceNode = surface->Get(i);

  real sumXCrossUnit = 0;
  real sumYCrossUnit = 0;
  real sumZCrossUnit = 0;

  plist = elemSurface->at (surfaceNode);
  for( face=plist.begin();face!=plist.end();++face )
  {
   // 3D: 2 pontos pois a face em 3D pertencente a superficie contem 
   //    // 3 pontos (P0 - surfaceNode, P1 e P2 que sao pontos do triangulo)
   real P0x = X->Get(surfaceNode);
   real P0y = Y->Get(surfaceNode);
   real P0z = Z->Get(surfaceNode);
   plist2 = neighbourFaceVert->at (*face);
   vert=plist2.begin();
   int v1 = *vert;++vert;
   int v2 = *vert;
   vert=plist2.end();
   real P1x = X->Get(v1);
   real P1y = Y->Get(v1);
   real P1z = Z->Get(v1);
   real P2x = X->Get(v2);
   real P2y = Y->Get(v2);
   real P2z = Z->Get(v2);

   // vetores
   real x1 = P1x-P0x;
   real y1 = P1y-P0y;
   real z1 = P1z-P0z;
   real x2 = P2x-P0x;
   real y2 = P2y-P0y;
   real z2 = P2z-P0z;

   // produto vetorial para encontrar a normal a superficie do triangulo
   real xCross = (y1*z2)-(z1*y2);
   real yCross = -( (x1*z2)-(z1*x2) );
   real zCross = (x1*y2)-(y1*x2);

   // norma do vetor cross( xCross,yCross,zCross )
   real len = sqrt( (xCross*xCross)+(yCross*yCross)+(zCross*zCross) );

   // area calculada atraves da normal
   //real area = 0.5*len;

   // ponderando a normal (resultante do produto vetorial) com a area do
   // triangulo, i.e., quem tem a menor area tem um peso maior na
   // tangente
   sumXCrossUnit += xCross/len;
   sumYCrossUnit += yCross/len;
   sumZCrossUnit += zCross/len;

   //wALE.Set(aux,wSol.Get(aux)-bubbleZVelocity);

  }
  
  // norma do vetor sumCrossUnit( sumXCrossUnit,sumYCrossUnit,sumZCrossUnit )
  real len2 = sqrt( sumXCrossUnit*sumXCrossUnit +
	                sumYCrossUnit*sumYCrossUnit +
					sumZCrossUnit*sumZCrossUnit );

  // unitario do vetor normal (ponderado com a area) resultante
  real xNormalUnit = sumXCrossUnit/len2;
  real yNormalUnit = sumYCrossUnit/len2;
  real zNormalUnit = sumZCrossUnit/len2;

//--------------------------------------------------
//   cout << surfaceNode << " " 
//        << len2 << " " 
//        << sumXCrossUnit << " " << sumYCrossUnit << " " << sumZCrossUnit << " "
//        << xNormalUnit << " " << yNormalUnit << " " << zNormalUnit << endl;
//-------------------------------------------------- 

  // produto escalar --> projecao do vetor normalUnit no segmento de reta
  // | Unit.RetaUnit | . RetaUnit
  // resultado = vetor normal a reta situado na superficie
  real prod = uSol.Get(surfaceNode)*xNormalUnit+ 
              vSol.Get(surfaceNode)*yNormalUnit + 
			  wSol.Get(surfaceNode)*zNormalUnit;
  real uSolNormal = xNormalUnit*prod;
  real vSolNormal = yNormalUnit*prod;
  real wSolNormal = zNormalUnit*prod;

  // tratamento da superficie
  // produto escalar --> projecao do vetor normalUnit no segmento de reta
  // | Unit.RetaUnit | . RetaUnit
  // resultado = vetor normal a reta situado na superficie
  real prod2 = uSmoothCoord.Get(surfaceNode)*xNormalUnit + 
               vSmoothCoord.Get(surfaceNode)*yNormalUnit + 
			   wSmoothCoord.Get(surfaceNode)*zNormalUnit;
  real uSmoothNormal = xNormalUnit*prod2;
  real vSmoothNormal = yNormalUnit*prod2;
  real wSmoothNormal = zNormalUnit*prod2;

  real uSmoothTangent = uSmoothCoord.Get(surfaceNode) - uSmoothNormal;
  real vSmoothTangent = vSmoothCoord.Get(surfaceNode) - vSmoothNormal;
  real wSmoothTangent = wSmoothCoord.Get(surfaceNode) - wSmoothNormal;

  //uALE.Set(surfaceNode,uSolNormal+uSmoothTangent);
  //vALE.Set(surfaceNode,vSolNormal+vSmoothTangent);
  //wALE.Set(surfaceNode,wSolNormal+wSmoothTangent);

  //uALE.Set(surfaceNode,uSmoothTangent);
  //vALE.Set(surfaceNode,vSmoothTangent);
  //wALE.Set(surfaceNode,wSmoothTangent);

  uALE.Set(surfaceNode,uSolNormal);
  vALE.Set(surfaceNode,vSolNormal);
  wALE.Set(surfaceNode,wSolNormal);
 }
} // fecha metodo setInterfaceVelNormal 

//setRHS eh o metodo que cria o vetor do lado direito (condicao de
//contorno + termo convectivo)
void Simulator3D::setRHS()
{
 // sem correcao na pressao
 va = ( (1.0/dt) * M + (1-alpha) * -(1.0/Re) * K ) * convUVW;

 // com correcao na pressao
 //va = ( (1.0/dt) * M - (1-alpha) * (1.0/Re) * K ) * convUVW - (G*pSol);
}

void Simulator3D::setCRHS()
{
 //vcc = ( (1.0/dt) * Mc - (1-alpha) * (1.0/(Re*Sc)) * Kc ) * convC;
 //vcc = ( (1.0/dt) * McLumped - (1-alpha) * (1.0/(Re*Sc)) * Kc ) * convC;
 vcc = ( (1.0/dt) * McLumped ) * convC;
}

void Simulator3D::setGravity()
{
 clVector uvwOne(2*numNodes);
 uvwOne.SetAll(0.0);
 clVector wOne(numNodes);
 wOne.SetAll(-1.0);
 uvwOne.Append(wOne);

 va = va + M * ( (1.0/(Fr*Fr)) * uvwOne );
}

void Simulator3D::setGravityBoussinesq()
{
 clVector zeroAux(numNodes-numVerts);
 clVector forceC(2*numNodes);
 forceC.Append(convC); 
 forceC.Append(zeroAux);

 va = va - beta*( M*forceC );
}

void Simulator3D::setInterface()
{
 matMountC();
 setUnCoupledCBC();
 Interface3D interface(*m);
 distance = interface.curvature1();

 // ----------- smoothing step ------------ //
  vcc = ( (1.0/dt) * McLumped ) * distance;
  //vcc = ( (1.0/dt) * Mc - (1-alpha) * (1.0/Sc) * Kc ) * distance;
  unCoupledC();
  //clVector kappaAux = invC*(Kc*distance);
  clVector kappaAux = invC*(Kc*cTilde);
  //clVector kappaAux = invMcLumped*(Kc*cTilde);
 // --------------------------------------- //
 
 //interface.plotKappa(kappaAux);
 kappa = interface.setKappaSurface(kappaAux);

 fint = (1.0/We) * sigma * ( kappa*(GTilde*cSol) );

 //va = va + invA*fint;
} // fecha metodo setInterface 

void Simulator3D::setInterfaceGeo()
{
 Interface3D interface(*m);
 clVector kappaAux = interface.computeKappa1();

 //interface.plotKappa(kappaAux);
 kappa = interface.setKappaSurface(kappaAux);
 // eu acho que eh necessario neste ponto aplicar a direcao do kappa

 fint = (1.0/We) * sigma * ( kappa*(GTilde*cSol) );
 
 //va = va + invA*fint;
} // fecha metodo setInterface 

void Simulator3D::matMount()
{
 mat = ( (1.0/dt) * M) + (alpha * (1.0/Re) * K );

 for( int i = 0; i < 3*numNodes; i++ )
  MLumped.Set(i, M.SumLine(i));

 for( int i = 0; i < 3*numNodes; i++ )
  invA.Set(i, mat.SumLine(i));

 invA = invA.Inverse();
 invMLumped = MLumped.Inverse();
}

void Simulator3D::matMountC()
{
 for( int i=0;i<numVerts;i++ )
  McLumped.Set(i, Mc.SumLine(i));

 //matc = ((1.0/dt) * Mc) + (alpha * (1.0/(Sc)) * Kc);
 //matc = ((1.0/dt) * Mc) + (alpha * (1.0/(Sc*Re)) * Kc);
 matc = ((1.0/dt) * McLumped) + (alpha * (1.0/(Sc)) * Kc);
 //matc = ((1.0/dt) * McLumped) + (alpha * (1.0/(Sc*Re)) * Kc);

 for( int i = 0; i < numVerts; i++ )
  invC.Set(i, matc.SumLine(i));

 invC = invC.Inverse();
 invMcLumped = McLumped.Inverse();
}

void Simulator3D::coupled()
{
 // A=[K G]=[matU  0 Gx][ ]=[us]  [us]=[ ]
 //   [D 0] [ 0 matV Gy][u]=[vs]  [vs]=[b]
 //         [Dx  Dy   0][ ]=[ps]  [ps]=[ ]

 A.CopyFrom(         0,         0,mat);
 A.CopyFrom(         0,3*numNodes,G  );
 A.CopyFrom(3*numNodes,         0,D  );

 clVector zeros(numVerts);

 b = va;
 b.Append(zeros);
 setCoupledBC();

 cout << " --> solving velocity and pressure -- " << endl;
 solverV->solve(1E-15,A,uAnt,b);
 cout << " ------------------------------------ " << endl;
 
 uAnt.CopyTo(0,uSol);
 uAnt.CopyTo(numNodes,vSol);
 uAnt.CopyTo(numNodes*2,wSol);
 uAnt.CopyTo(numNodes*3,pSol);
 
} // fecha metodo coupled 

void Simulator3D::unCoupled()
{
 clVector uvw(3*numNodes);
 clVector vaIp(3*numNodes);
 clVector b1Tilde;
 clVector b2Tilde;

 vaIp = va.MultVec(ip); // operacao vetor * vetor (elemento a elemento)

 b1Tilde = b1 + vaIp;

 // resolve sitema ATilde uTilde = b
 cout << " --------> solving velocity --------- " << endl;
 solverV->solve(1E-15,ATilde,uTilde,b1Tilde);
 cout << " ------------------------------------ " << endl;

 //uvw = uTilde + dt*invMLumped*fint;
 uvw = uTilde + invA*fint;
 //uvw = uTilde;

 b2Tilde = b2 - (DTilde * uvw); // D com c.c.
 b2Tilde = (-1) * b2Tilde;

 // resolve sistema E pTilde = b2
 cout << " --------> solving pressure --------- " << endl;
 solverP->solve(1E-15,ETilde,pTilde,b2Tilde);
 cout << " ------------------------------------ " << endl;
 
 //uvw = uTilde - dt*(invMLumped * GTilde * pTilde);
 uvw = uvw - (invA * GTilde * pTilde);

 uvw.CopyTo(         0,uSol);
 uvw.CopyTo(  numNodes,vSol);
 uvw.CopyTo(numNodes*2,wSol);

//--------------------------------------------------
//  /* TEST */
//  real bubbleZVelocity = getBubbleVelocity();
//  for( int i=0;i<surface->Dim();i++ )
//  {
//   int aux = surface->Get(i);
//   real vel = wSol.Get(aux)+bubbleZVelocity;
//   wSol.Set(aux,vel);
//  }
//  /********/
//-------------------------------------------------- 

 pSol = pTilde;       // sem correcao na pressao
 //pSol = pSol + pTilde;  // com correcao na pressao

 uAnt.CopyFrom(0,uvw);
 uAnt.CopyFrom(numNodes*3,pSol);

 time = time + dt;
} // fecha metodo unCoupled 

void Simulator3D::unCoupledC()
{
 clVector vcIp(numVerts);
 clVector b1cTilde;

 vcIp = vcc.MultVec(ipc); // operacao vetor * vetor (elemento a elemento)

 b1cTilde = b1c + vcIp;

 // resolve sitema ATilde uTilde = b
 cout << " --------> solving scalar ----------- " << endl;
 solverC->solve(1E-15,AcTilde,cTilde,b1cTilde);
 cout << " ------------------------------------ " << endl;

 // comentar em caso de utilizacao de setInterface()
 // pois cSol nao pode ser atualizado
 cSol = cTilde;
}

void Simulator3D::convergenceCriteria( real value )
{
 clVector Fnew = uAnt;

 real diffSum = ( (Fnew-Fold).Abs() ).Sum();
 real FnewSum = ( Fnew.Abs() ).Sum();
 real error = (diffSum/FnewSum)/dt;

 cout << "relative error: " << error << endl;

 Fold = Fnew;
} // fecha metodo convergenceCriteria

/**
 * @brief ESTRATEGIA PARA APLICACAO DA CONDICAO DE CONTORNO PARA SISTEMA
 * RESOLVIDO DIRETAMENTE
 * 1-localizar o vertice onde ha condicao de contorno
 * 2-somar a coluna onde o vertice se encontra no vetor do lado direto 
 * 3-zerar a coluna e a linha colocando 1 no vertice da matriz
 * 4-repetir a operacao para outros vertices condicao de contorno 
 *
 * @return VOID (VAZIO)
 **/
void Simulator3D::setCoupledBC()
{
 int nbc,i,j;
 clVector vetColSizeUVWP(3*numNodes+numVerts);
 clVector UVWPC = *uc;
 UVWPC.Append(*vc);
 UVWPC.Append(*wc);
 UVWPC.Append(*pc);
 
 nbc = idbcu->Dim();
 for( i=0;i<nbc;i++ )
 {
  j=(int) idbcu->Get(i);
  b.CopyMult(j,A,UVWPC);
  A.Set(j,j,1);
  b.Set(j,uc->Get(j));
 }
 cout << "imposta c.c. de U " << endl;

 nbc = idbcv->Dim();
 for( i=0;i<nbc;i++ )
 {
  j=(int) idbcv->Get(i);
  b.CopyMult(j+numNodes,A,UVWPC);
  A.Set( j+numNodes, j+numNodes, 1 );
  b.Set( j+numNodes, vc->Get(j) );
 }
 cout << "imposta c.c de V " << endl;

 nbc = idbcw->Dim();
 for( i=0;i<nbc;i++ )
 {
  j=(int) idbcw->Get(i);
  b.CopyMult(j+numNodes*2,A,UVWPC);
  A.Set( j+numNodes*2,j+numNodes*2, 1);
  b.Set( j+numNodes*2,wc->Get(j) );
 }
 cout << "imposta c.c. de W " << endl;

 nbc = idbcp->Dim();
 for( i=0;i<nbc;i++ )
 {
  j = (int) idbcp->Get(i);
  b.CopyMult(j+numNodes*3,A,UVWPC);
  A.Set( j+3*numNodes,j+3*numNodes, -1 );
  b.Set( j+3*numNodes, -pc->Get(j) ); // sem correcao na pressao
  //b.Set( j+3*numNodes,-pc->Get(j)*0 ); // com correcao na pressao
 }
 cout << "imposta c.c. de P " << endl;
 
} // fecha metodo setCoupledBC 

void Simulator3D::setUnCoupledBC()
{
 int nbc,i,j;
 // ----------- nova versao ------------ //
 clVector UVWC = *uc;
 UVWC.Append(*vc);
 UVWC.Append(*wc);
 // ------------------------------------ //
 
 ATilde = mat;
 DTilde = D;
 GTilde = G;

 b1.Dim(3*numNodes,0);  // zerando os vetores b1 e b2
 b2.Dim(numVerts,0);
 ip.Dim(3*numNodes,1); // inicializando vetor ip com 1

 nbc = idbcu->Dim();
 for( i=0;i<nbc;i++ )
 {
  j=(int) idbcu->Get(i);
  b1.CopyMult(j,ATilde,UVWC);
  b2.CopyMult(j,DTilde,GTilde,UVWC);
  ATilde.Set(j,j,1);
  b1.Set(j,uc->Get(j));
  ip.Set(j,0);
 }
 cout << "imposta c.c. de U " << endl;

 nbc = idbcv->Dim();
 for( i=0;i<nbc;i++ )
 {
  j=(int) idbcv->Get(i);
  b1.CopyMult(j+numNodes,ATilde,UVWC);
  b2.CopyMult(j+numNodes,DTilde,GTilde,UVWC);
  ATilde.Set(j+numNodes,j+numNodes,1);
  b1.Set(j+numNodes,vc->Get(j));
  ip.Set(j+numNodes,0);
 }
 cout << "imposta c.c. de V " << endl;

 nbc = idbcw->Dim();
 for( i=0;i<nbc;i++ )
 {
  j=(int) idbcw->Get(i);
  b1.CopyMult(j+numNodes*2,ATilde,UVWC);
  b2.CopyMult(j+numNodes*2,DTilde,GTilde,UVWC);
  ATilde.Set(j+numNodes*2,j+numNodes*2,1);
  b1.Set(j+numNodes*2,wc->Get(j));
  ip.Set(j+numNodes*2,0);
 }
 cout << "imposta c.c. de W " << endl;

 E.Dim(numVerts,numVerts);
 nbc = idbcp->Dim();
 for( i=0;i<nbc;i++ )
 {
  j=(int) idbcp->Get(i);
  b1.CopyMult(j,GTilde,DTilde,*pc);
  E.Set(j,j,-1);
  b2.Set(j,-pc->Get(j));  // sem correcao na pressao
  //b2.Set(j,-pc->Get(j)*0);  // com correcao na pressao
 }
 cout << "imposta c.c. de P " << endl;

 ETilde = E - ((DTilde * invA) * GTilde); 
 //ETilde = E - dt*((DTilde * invMLumped) * GTilde); 
} // fecha metodo setUnCoupledBC 

void Simulator3D::setUnCoupledCBC()
{
 int nbc,i,j;
 b1c.Dim(numVerts,0);  // zerando o vetore b1c
 ipc.Dim(numVerts,1); // inicializando vetor ipc com 1
 
 AcTilde = matc;

 nbc = idbcc->Dim();
 for( i=0;i<nbc;i++ )
 {
  j=(int) idbcc->Get(i);
  b1c.CopyMult(j,AcTilde,*cc);
  AcTilde.Set(j,j,1);
  b1c.Set(j,cc->Get(j));
  ipc.Set(j,0);
 }
 cout << "imposta c.c. de C " << endl;

} // fecha metodo setUnCoupledCBC 

void Simulator3D::setCfl(real _cfl)
{
 cfl = _cfl;
 real xMax = X->Max();
 real xMin = X->Min();
 real yMax = Y->Max();
 real yMin = Y->Min();
 real zMax = Z->Max();
 real zMin = Z->Min();

 dt = cfl*sqrt( (zMax-zMin)*(yMax-yMin)*(xMax-xMin)/numVerts);
}

void Simulator3D::setCflDisk(real _cfl)
{
 cfl = _cfl;

 real xMax = X->Max();
 real xMin = X->Min();
 real yMax = Y->Max();
 real yMin = Y->Min();
 //real zMax = Z->Max();
 //real zMin = Z->Min();
 real ucMax = uc->Max();

//--------------------------------------------------
//  dt = cfl/( (m->getMaxAbsUC()/m->getDeltaXMin() ) +
//             (m->getMaxAbsVC()/m->getDeltaYMin() ) +
//             (m->getMaxAbsWC()/m->getDeltaZMin() ) );
//-------------------------------------------------- 

 dt = cfl*sqrt((yMax-yMin)*(xMax-xMin)/numVerts)/fabs(ucMax);
}

void Simulator3D::setCflBubble(real _cfl)
{
 cfl = _cfl;
//--------------------------------------------------
//  real dx = m->getDeltaXMin();
//  real dy = m->getDeltaYMin();
//  real dz = m->getDeltaZMin();
// 
//  dt = cfl*sqrt( dx*dy*dz/(2*PI*sigma) );
//-------------------------------------------------- 

 real xMax = X->Max();
 real xMin = X->Min();
 real yMax = Y->Max();
 real yMin = Y->Min();
 real zMax = Z->Max();
 real zMin = Z->Min();
 real L = ( (xMax-xMin)*(yMax-yMin)*(zMax-zMin) )/numNodes;

 dt = cfl*sqrt( 1.0*L*L*L/(3.141592*sigma) );
}

void Simulator3D::setUAnt(clVector &_uAnt)
{
 uAnt = _uAnt;
 uAnt.CopyTo(0,uSol); 
 uAnt.CopyTo(numNodes,vSol); 
 uAnt.CopyTo(numNodes*2,wSol); 
 uAnt.CopyTo(numNodes*3,pSol); 
}

void Simulator3D::setHsmooth()
{
//--------------------------------------------------
//  Interface2D interface(*m);
//  distance = interface.distanceNodes();
//  // -- set para heaviside suavizada -- //
//  real aux;
//  real epsilon=0.1;
//  for( int i=0;i<numNodes;i++ )
//  {
//   if( distance.Get(i) < -epsilon )
//   {
//    Hsmooth.Set(i,0.0);
//   }
//   if( distance.Get(i) > epsilon )
//   {
//    Hsmooth.Set(i,1.0);
//   }
//   if( (distance.Get(i) >= -epsilon) && distance.Get(i) <= epsilon )
//   {
//    aux = (1.0/PI)*sin(PI*distance.Get(i)/epsilon);
//    aux = 0.5*(1.0 + distance.Get(i)/epsilon + aux);
//    Hsmooth.Set(i,aux);
//   }
//  }
//-------------------------------------------------- 
}

void Simulator3D::setNu(real nu0, real nu1)
{
//--------------------------------------------------
//  clVector ones(numNodes);ones.SetAll(1.0);
//  nu = nu1*Hsmooth + nu0*(ones-Hsmooth);
//-------------------------------------------------- 
}

void Simulator3D::setRho(real rho0, real rho1)
{
//--------------------------------------------------
//  clVector ones(numNodes);ones.SetAll(1.0);
//  rho = rho1*Hsmooth + rho0*(ones-Hsmooth);
//-------------------------------------------------- 
}

void Simulator3D::setCSol(clVector &_cSol)
{
 cSol = _cSol;
}

void Simulator3D::setNuZ()
{
 // -- Leitura do perfil de nu variavel em Z para os nos da malha -- //

 real aux;
 real dist1,dist2;
 clMatrix nuFile(1002,2); // vetor super dimensionado!

 const char* _filename = "../../db/baseState/nuZ/nuZ.dat";
 ifstream file( _filename,ios::in );

 if( !file )
 {
  cerr << "Esta faltando o arquivo de perfis!" << endl;
  exit(1);
 }

 // leitura do arquivo e transferencia para matriz
 if( !file.eof() )
 {
  for( int i=0;i<1001;i++ )
  {
   file >> aux;
   nuFile.Set(i,0,aux);
   file >> aux;
   nuFile.Set(i,1,aux);
  }
 }

 int j;
 for( int i=0;i<numNodes;i++ )
 {
  for( j=0;j<1001;j++ )
  {
   dist1 = fabs( Z->Get(i) - nuFile(j,0) );
   dist2 = fabs( Z->Get(i) - nuFile(j+1,0) );
   if( dist2 > dist1 ) break;
  }
  aux = nuFile(j,1);
  nu.Set(i,aux);
 }
}

void Simulator3D::setSolverVelocity(Solver *s){solverV = s;}
void Simulator3D::setSolverPressure(Solver *s){solverP = s;}
void Simulator3D::setSolverConcentration(Solver *s){solverC = s;}
void Simulator3D::setRe(real _Re){Re = _Re;}
real Simulator3D::getRe(){return Re;}
void Simulator3D::setSc(real _Sc){Sc = _Sc;}
real Simulator3D::getSc(){return Sc;}
void Simulator3D::setFr(real _Fr){Fr = _Fr;}
real Simulator3D::getFr(){return Fr;}
void Simulator3D::setWe(real _We){We = _We;}
real Simulator3D::getWe(){return We;}
void Simulator3D::setAlpha(real _alpha){alpha = _alpha;}
real Simulator3D::getAlpha(){return alpha;}
void Simulator3D::setBeta(real _beta){beta = _beta;}
real Simulator3D::getBeta(){return beta;}
void Simulator3D::setSigma(real _sigma){sigma = _sigma;}
real Simulator3D::getSigma(){return sigma;}
void Simulator3D::setDt(real _dt){dt = _dt;}
void Simulator3D::setTime(real _time){time = _time;}
real Simulator3D::getDt(){return dt;}
real Simulator3D::getTime2(){return time;}
real Simulator3D::getCfl(){return cfl;}
real* Simulator3D::getTime(){return &time;}
clVector* Simulator3D::getUSol(){return &uSol;} 
void Simulator3D::setUSol(clVector &_uSol){uSol = _uSol;}
clVector* Simulator3D::getVSol(){return &vSol;} 
void Simulator3D::setVSol(clVector &_vSol){vSol = _vSol;}
clVector* Simulator3D::getWSol(){return &wSol;}
void Simulator3D::setWSol(clVector &_wSol){wSol = _wSol;}
clVector* Simulator3D::getPSol(){return &pSol;}
clVector* Simulator3D::getCSol(){return &cSol;}
clVector* Simulator3D::getUALE(){return &uALE;} 
clVector* Simulator3D::getVALE(){return &vALE;} 
clVector* Simulator3D::getWALE(){return &wALE;} 
clVector* Simulator3D::getUAnt(){return &uAnt;}
clVector* Simulator3D::getCAnt(){return &cAnt;}
clVector* Simulator3D::getDistance(){return &distance;}
clVector* Simulator3D::getFint(){return &fint;}
clDMatrix* Simulator3D::getKappa(){return &kappa;}
clMatrix* Simulator3D::getK(){return &K;}
clMatrix* Simulator3D::getM(){return &M;}
clMatrix* Simulator3D::getGx(){return &gx;}
clMatrix* Simulator3D::getGy(){return &gy;}
clMatrix* Simulator3D::getGz(){return &gz;}
clMatrix* Simulator3D::getG(){return &G;}
clMatrix* Simulator3D::getD(){return &D;}
void Simulator3D::updateIEN(){IEN = m->getIEN();}

// set do centroide para o elemento mini apos a interpolacao linear
// (applyLinearInterpolation)
void Simulator3D::setCentroid()
{
 int v[5];
 real aux;

 for( int mele=0;mele<numElems;mele++ )
 {
  v[0] = (int) IEN->Get(mele,0);
  v[1] = (int) IEN->Get(mele,1);
  v[2] = (int) IEN->Get(mele,2);
  v[3] = (int) IEN->Get(mele,3);
  v[4] = (int) IEN->Get(mele,4);

  aux = ( uSol.Get(v[0])+
	      uSol.Get(v[1])+
		  uSol.Get(v[2])+
		  uSol.Get(v[3]) )*0.25;
  uSol.Set(v[4],aux);

  aux = ( vSol.Get(v[0])+
          vSol.Get(v[1])+
          vSol.Get(v[2])+
	 	  vSol.Get(v[3]) )*0.25;
  vSol.Set(v[4],aux);

  aux = ( wSol.Get(v[0])+
	      wSol.Get(v[1])+
          wSol.Get(v[2])+
		  wSol.Get(v[3]) )*0.25;
  wSol.Set(v[4],aux);

  aux = ( uALE.Get(v[0])+
	      uALE.Get(v[1])+
		  uALE.Get(v[2])+
		  uALE.Get(v[3]) )*0.25;
  uALE.Set(v[4],aux);

  aux = ( vALE.Get(v[0])+
	      vALE.Get(v[1])+
		  vALE.Get(v[2])+
		  vALE.Get(v[3]) )*0.25;
  vALE.Set(v[4],aux);

  aux = ( wALE.Get(v[0])+
	      wALE.Get(v[1])+
		  wALE.Get(v[2])+
		  wALE.Get(v[3]) )*0.25;
  wALE.Set(v[4],aux);
 }

}// fim do metodo compute -> setCentroid

// Atribui o Simulator3D do argumento no corrente
void Simulator3D::operator=(Simulator3D &_sRight) 
{
 m = _sRight.m;
 numVerts = _sRight.numVerts;
 numNodes = _sRight.numNodes;
 numElems = _sRight.numElems;
 numGLEU = _sRight.numGLEU;
 numGLEP = _sRight.numGLEP;
 numGLEC = _sRight.numGLEC;
 X = _sRight.X;
 Y = _sRight.Y;
 Z = _sRight.Z;
 uc = _sRight.uc;
 vc = _sRight.vc;
 wc = _sRight.wc;
 pc = _sRight.pc;
 cc = _sRight.cc;
 idbcu = _sRight.idbcu;
 idbcv = _sRight.idbcv;
 idbcw = _sRight.idbcw;
 idbcp = _sRight.idbcp;
 idbcc = _sRight.idbcc;
 outflow = _sRight.outflow;
 surface = _sRight.surface;
 IEN = _sRight.IEN;

 Re = _sRight.Re;
 Sc = _sRight.Sc;
 Fr = _sRight.Fr;
 We = _sRight.We;
 alpha = _sRight.alpha;
 beta = _sRight.beta;
 dt = _sRight.dt;
 cfl = _sRight.cfl;
 time = _sRight.time;
 sigma = _sRight.sigma;
 c1 = _sRight.c1;
 c2 = _sRight.c2;
 c3 = _sRight.c3;

 // assembly matrix
 K.Dim( 3*numNodes,3*numNodes );
 Kc.Dim( numVerts,numVerts );
 M.Dim( 3*numNodes,3*numNodes );
 Mc.Dim( numVerts,numVerts );
 MLumped.Dim( 3*numNodes );
 McLumped.Dim( numVerts );
 G.Dim( 3*numNodes,numVerts );
 D.Dim( numVerts,3*numNodes );
 gx.Dim( numNodes,numVerts );
 gy.Dim( numNodes,numVerts );
 gz.Dim( numNodes,numVerts );

 // COUPLED method matrix and vector
 A.Dim( 3*numNodes+numVerts,3*numNodes+numVerts );
 b.Dim( 3*numNodes+numVerts );

 // right hand side vectors
 va.Dim( 3*numNodes );
 vcc.Dim( numVerts );
 b1.Dim( 3*numNodes );
 b1c.Dim( numVerts );
 b2.Dim( numVerts );

 // boundary condiction configured matrix
 ATilde.Dim( 3*numNodes,3*numNodes );
 AcTilde.Dim( numVerts,numVerts );
 GTilde.Dim( 3*numNodes,numVerts );
 DTilde.Dim( numVerts,3*numNodes );
 ETilde.Dim( numVerts,numVerts );
 E.Dim( numVerts, numVerts );

 // K + M matrix set
 mat.Dim( 3*numNodes,3*numNodes );
 matc.Dim( numVerts,numVerts );
 invA.Dim( 3*numNodes );
 invMLumped.Dim( 3*numNodes );
 invC.Dim( numVerts );
 invMcLumped.Dim( numVerts );

 // vetores solucao
 uTilde.Dim( 3*numNodes );
 pTilde.Dim( numVerts );
 cTilde.Dim( numVerts );
 uSol.Dim( numNodes );
 vSol.Dim( numNodes );
 wSol.Dim( numNodes );
 pSol.Dim( numVerts );
 cSol.Dim( numVerts );

 // vetores do termo de conveccao
 convUVW.Dim( 3*numNodes );
 convC.Dim( numVerts );

 // auxiliar vectors
 Fold.Dim( 3*numNodes+numVerts );
 uAnt.Dim( 3*numNodes+numVerts );
 cAnt.Dim( numVerts );
 ip.Dim( 3*numNodes,1 );
 ipc.Dim( numVerts,1 );

 // convective term vectors (semi-lagrangian)
 uSL.Dim( numNodes );
 vSL.Dim( numNodes );
 wSL.Dim( numNodes );
 cSL.Dim( numVerts );

 // convective term vectors (ALE)
 uALE.Dim( numNodes );
 vALE.Dim( numNodes );
 wALE.Dim( numNodes );
 uSmooth.Dim( numNodes );
 vSmooth.Dim( numNodes );
 wSmooth.Dim( numNodes );

 // interface vectors (two-phase)
 distance.Dim( numVerts );
 kappa.Dim( 3*numNodes );
 fint.Dim ( 3*numNodes );
 Hsmooth.Dim( numNodes );
 nu.Dim( numNodes );
 rho.Dim( numNodes );

 K = _sRight.K;
 Kc = _sRight.Kc;
 M = _sRight.M;
 Mc = _sRight.Mc;
 G = _sRight.G;
 D = _sRight.D;
 mat = _sRight.mat;
 matc = _sRight.matc;
 MLumped = _sRight.MLumped;
 McLumped = _sRight.McLumped;
 velU = _sRight.velU;
 velV = _sRight.velV;
 velW = _sRight.velW;

 uSol = _sRight.uSol;
 vSol = _sRight.vSol;
 wSol = _sRight.wSol;
 pSol = _sRight.pSol;
 cSol = _sRight.cSol;
 uALE = _sRight.uALE;
 vALE = _sRight.vALE;
 wALE = _sRight.wALE;
 uSL = _sRight.uSL;
 vSL = _sRight.vSL;
 wSL = _sRight.wSL;
 cSL = _sRight.cSL;
 uSmooth = _sRight.uSmooth;
 vSmooth = _sRight.vSmooth;
 wSmooth = _sRight.wSmooth;

 gx = _sRight.gx;
 gy = _sRight.gy;
 gz = _sRight.gz;
 uAnt = _sRight.uAnt;
 cAnt = _sRight.cAnt;
 va = _sRight.va;
 vcc = _sRight.vcc;
 convUVW = _sRight.convUVW;
 convC = _sRight.convC;
 A = _sRight.A;
 b = _sRight.b;
 ATilde = _sRight.ATilde;
 AcTilde = _sRight.AcTilde;
 GTilde = _sRight.GTilde;
 DTilde = _sRight.DTilde;
 ETilde = _sRight.ETilde;
 E = _sRight.E;
 invA = _sRight.invA;
 invC = _sRight.invC;
 invMLumped = _sRight.invMLumped;
 invMcLumped = _sRight.invMcLumped;
 uTilde = _sRight.uTilde;
 cTilde = _sRight.cTilde;
 pTilde = _sRight.pTilde;
 b1 = _sRight.b1;
 b1c = _sRight.b1c;
 b2 = _sRight.b2;
 ip = _sRight.ip;
 ipc = _sRight.ipc;
 distance = _sRight.distance;
 kappa = _sRight.kappa;
 fint = _sRight.fint;
 Hsmooth = _sRight.Hsmooth;
 nu = _sRight.nu;
 rho = _sRight.rho;
 Fold = _sRight.Fold;

 solverV = _sRight.solverV;
 solverP = _sRight.solverP;
 solverC = _sRight.solverC;
}

void Simulator3D::loadSolution( const char* _dir,
                                const char* _filename )
{
 string fileUVWPC = (string) _dir + (string) _filename + ".bin";
 const char* filenameUVWPC = fileUVWPC.c_str();

 clVector aux2(6*numNodes+2*numVerts); // vetor tambem carrega a concentracao

 ifstream UVWPC_file( filenameUVWPC,ios::in | ios::binary ); 

 if( !UVWPC_file)
 {
  cerr << "Solution file is missing for reading!" << endl;
  exit(1);
 }

 UVWPC_file.read( (char*) aux2.GetVec(),aux2.Dim()*sizeof(real) );

 UVWPC_file.close();

 // setting vel+pressure+concentration
 aux2.CopyTo(0,uSol);
 aux2.CopyTo(numNodes,vSol);
 aux2.CopyTo(2*numNodes,wSol);
 aux2.CopyTo(3*numNodes,pSol);
 aux2.CopyTo(3*numNodes+numVerts,cSol);
 aux2.CopyTo(3*numNodes+2*numVerts,uALE);
 aux2.CopyTo(4*numNodes+2*numVerts,vALE);
 aux2.CopyTo(5*numNodes+2*numVerts,wALE);

 // setting uAnt
 aux2.CopyTo(0,uAnt);

 cout << "solucao <<last>> lida em binario" << endl;
 
} // fecha metodo loadSol 

void Simulator3D::loadSolution( const char* _dir,
                                const char* _filename, 
								int _iter )
{
 stringstream ss;  //convertendo int --> string
 string str;
 ss << _iter;
 ss >> str;

 cout << _iter << endl;

 string fileUVWPC = _dir;
 string aux = _filename;
 fileUVWPC += aux + "-" + str + ".bin";
 const char* filenameUVWPC = fileUVWPC.c_str();

 cout << endl;
 cout << filenameUVWPC << endl;
 cout << endl;

 clVector aux2(6*numNodes+2*numVerts); // vetor tambem carrega a concentracao

 ifstream UVWPC_file( filenameUVWPC,ios::in | ios::binary ); 

 if( !UVWPC_file)
 {
  cerr << "Solution file is missing for reading!" << endl;
  exit(1);
 }

 UVWPC_file.read( (char*) aux2.GetVec(),aux2.Dim()*sizeof(real) );

 UVWPC_file.close();

 aux2.CopyTo(0,uSol);
 aux2.CopyTo(numNodes,vSol);
 aux2.CopyTo(2*numNodes,wSol);
 aux2.CopyTo(3*numNodes,pSol);
 aux2.CopyTo(3*numNodes+numVerts,cSol);
 aux2.CopyTo(3*numNodes+2*numVerts,uALE);
 aux2.CopyTo(4*numNodes+2*numVerts,vALE);
 aux2.CopyTo(5*numNodes+2*numVerts,wALE);

 cout << "solucao no.  " << _iter << " lida em binario" << endl;
 
} // fecha metodo loadSol 

int Simulator3D::loadIteration()
{
 ifstream simTime( "./sim/simTime.dat",ios::in ); 

 real _time;
 int iterNumber;

 simTime >> iterNumber;
 simTime >> _time;
 time = _time;

 simTime.close();

 cout << "time = " << _time << " " << "itereracao: " << iterNumber << endl;
 return iterNumber+1;
} // fecha metodo loadIteration

int Simulator3D::loadIteration( const char* _dir, 
                                const char* _filename )
{
 string filename = (string) _dir + (string) _filename  + ".vtk";
 const char* File = filename.c_str();

 ifstream vtkFile( File,ios::in );

 char auxstr[255];
 real time;
 int iterNumber;

 if( !vtkFile )
 {
  cerr << "VTK Mesh file is missing for TIME and ITERATION reading!" << endl;
  exit(1);
 }

 while( ( !vtkFile.eof())&&(strcmp(auxstr,"TIME") != 0) )
  vtkFile >> auxstr;

 vtkFile >> auxstr;
 vtkFile >> auxstr;
 vtkFile >> auxstr;
 vtkFile >> time;

 while( ( !vtkFile.eof())&&(strcmp(auxstr,"ITERATION") != 0) )
  vtkFile >> auxstr;

 vtkFile >> auxstr;
 vtkFile >> auxstr;
 vtkFile >> auxstr;
 vtkFile >> iterNumber;

 vtkFile.close();

 setTime(time); 

 return iterNumber+1;
} // fecha metodo loadIteration

int Simulator3D::loadIteration( const char* _dir, 
                                const char* _filename, 
								int _iter)
{
 stringstream ss;  //convertendo int --> string
 string str;
 ss << _iter;
 ss >> str;

 string filename = (string) _dir + (string) _filename + "-" + str + ".vtk";
 const char* File = filename.c_str();

 cout << endl;
 cout << File << endl;
 cout << endl;


 ifstream vtkFile( File,ios::in );

 char auxstr[255];
 real time;
 int iterNumber;

 if( !vtkFile )
 {
  cerr << "VTK Mesh file is missing for TIME and ITERATION reading!" << endl;
  exit(1);
 }

 while( ( !vtkFile.eof())&&(strcmp(auxstr,"TIME") != 0) )
  vtkFile >> auxstr;

 vtkFile >> auxstr;
 vtkFile >> auxstr;
 vtkFile >> auxstr;
 vtkFile >> time;

 while( ( !vtkFile.eof())&&(strcmp(auxstr,"ITERATION") != 0) )
  vtkFile >> auxstr;

 vtkFile >> auxstr;
 vtkFile >> auxstr;
 vtkFile >> auxstr;
 vtkFile >> iterNumber;

 vtkFile.close();

 setTime(time); 

 return iterNumber+1;
} // fecha metodo loadIteration

// interpolacao linear dos vetores velocidade e pressao calculados na
// malha antiga (Model3D mold) para malha nova (this). Note que a
// interpolacao eh feita somente nos vertices da malha deixando para o
// metodo setCentroid a configuracao dos valores de cada centroide
// pertencente aos elementos
void Simulator3D::applyLinearInterpolation(Model3D &_mOld)
{
 SemiLagrangean semi(_mOld,uSol,vSol,wSol,velU,velV,velW,cSol);

 // xVert da malha nova
 clVector xVert(numVerts);
 clVector yVert(numVerts);
 clVector zVert(numVerts);
 X->CopyTo(0,xVert);
 Y->CopyTo(0,yVert);
 Z->CopyTo(0,zVert);
 semi.meshInterp(xVert,yVert,zVert);
 uSol.Dim( numVerts );
 vSol.Dim( numVerts );
 wSol.Dim( numVerts );
 pSol.Dim( numVerts );
 cSol.Dim( numVerts );
 uALE.Dim( numVerts );
 vALE.Dim( numVerts );
 wALE.Dim( numVerts );

 // interpolacao linear em numVerts
 clMatrix* interpLin = semi.getInterpLin();
 uSol = *interpLin*(uSolOld);
 vSol = *interpLin*(vSolOld);
 wSol = *interpLin*(wSolOld);
 pSol = *interpLin*(pSolOld);
 uALE = *interpLin*(uALEOld);
 vALE = *interpLin*(vALEOld);
 wALE = *interpLin*(wALEOld);

 cSol.CopyFrom( 0,*cc ); // copying new cc

 // set do centroid
 clVector zerosConv(numNodes-numVerts);
 uSol.Append(zerosConv);
 vSol.Append(zerosConv);
 wSol.Append(zerosConv);
 uALE.Append(zerosConv);
 vALE.Append(zerosConv);
 wALE.Append(zerosConv);
 setCentroid();

 // setting uAnt
 uAnt.Dim( 3*numNodes+numVerts );
 uAnt.CopyFrom(numNodes*0,uSol);
 uAnt.CopyFrom(numNodes*1,vSol);
 uAnt.CopyFrom(numNodes*2,wSol);
 uAnt.CopyFrom(numNodes*3,pSol);

} // fecha metodo applyLinearInterpolation

// calcula velocidade media da bolha tomando como referencia o centro de
// massa
real Simulator3D::getBubbleVelocity()
{
 real velX,velY,velZ;
 real sumVolume=0;
 real sumXVelVolume=0;
 real sumYVelVolume=0;
 real sumZVelVolume=0;
 int count = 0;

 list<int> *inElem;
 inElem = m->getInElem();
 for (list<int>::iterator it=inElem->begin(); it!=inElem->end(); ++it)
 {
  int v0 = IEN->Get(*it,0);
  int v1 = IEN->Get(*it,1);
  int v2 = IEN->Get(*it,2);
  int v3 = IEN->Get(*it,3);

  velX = ( uALE.Get(v0)+
	       uALE.Get(v1)+
		   uALE.Get(v2)+
		   uALE.Get(v3) )*0.25;

  velY = ( vALE.Get(v0)+
           vALE.Get(v1)+
           vALE.Get(v2)+
	 	   vALE.Get(v3) )*0.25;

  // modificado
  velZ = ( wSol.Get(v0)+
           wSol.Get(v1)+
           wSol.Get(v2)+
	 	   wSol.Get(v3) )*0.25;
  velZ = wSol.Get(5);

  sumXVelVolume += velX * m->getVolume(*it);
  sumYVelVolume += velY * m->getVolume(*it);
  sumZVelVolume += velZ * m->getVolume(*it);
  sumVolume += m->getVolume(*it);
  count++;
 }
 //real bubbleXVel = sumXVelVolume/sumVolume;
 //real bubbleYVel = sumYVelVolume/sumVolume;
 real bubbleZVel = sumZVelVolume/sumVolume;
 //cout << "numero de elmentos = " << count << endl;
 //cout << "bubbleYVel = " << bubbleYVel << endl;
 //cout << "bubbleZVel = " << bubbleZVel << endl;

 return bubbleZVel;
}

// impoe velocidade ALE = 0 no contorno
void Simulator3D::setALEVelBC()
{
 list<int> *outVert = m->getOutVert();

 for (list<int>::iterator it=outVert->begin(); it!=outVert->end(); ++it)
 {
  int vertice = *it;
  uALE.Set(vertice,0.0);
  vALE.Set(vertice,0.0);
  wALE.Set(vertice,0.0);
 }
}

