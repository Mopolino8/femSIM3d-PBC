/**=========================================================================
 *\file Id:InOut.h, GESAR
 *\author Gustavo Rabello dos Anjos
 *\date   23-ago-2007
 *\comment Classe InOut para o modulo HYDRO
 *\references
 *\version 1.0
 *\updates
 	version    date         author             comment
	1.0        23/08/2007   Gustavo            implementacao da classe
 =========================================================================**/

#ifndef INOUT_H
#define INOUT_H

#include "Model3D.h"
#include "Simulator3D.h"
#include "clMatrix.h"
#include "clDMatrix.h"
#include "clVector.h"
#include "PCGSolver.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cmath>

/**
 * @brief classe responsavel pela manipulacao de entrada e saida de
 * dados em arquivos ASCII e binario.
 *
 * @return 
 **/
class InOut
{
 public:
  InOut( Model3D &_m );
  InOut( Model3D &_m,Simulator3D &_s );
  virtual ~InOut();

  /**
   * @brief grava arquivo ASCII da solucao do sistema, ou seja, grava
   * os valores de todas matrizes e vetores criadas na inicializacao do
   * simuldador (metodo init) e vetores solucao
   *
   * @param _s objeto do tipo Simulador3D para resgatar os valores
   * nodais dos vetores e matrizes 
   * @param _dir diretorio onde serao gravados os arquivos
   *
   * @return 
   **/
  void saveTXT( const char* _dir,const char* _filename, int _iter );

  /**
   * @brief grava arquivo ASCII da solucao do sistema, ou seja, grava
   * apenas os valores que sao modificados com os passos de tempo. Os
   * vetores sao: velocidade e pressao
   *
   * @param _s objeto do tipo Simulador3D para resgatar os valores
   * nodais dos vetores solucao do problema
   * @param _dir diretorio onde serao gravados os arquivos
   * @param iter numero da iteracao  
   *
   * @return 
   **/
  void saveSolTXT( const char* _dir,const char* _filename, int _iter );

  void saveSol( const char* _dir,const char* _filename,int _iter );
  void loadSol( const char* _dir,const char* _filename,int _iter );

  /**
   * @brief grava arquivo do tipo ASCII da matriz passada como argumento 
   *
   * @param _matrix matriz a ser gravada em arquivo
   *
   * @param _filename campo para inclusao do diretorio + raiz do nome
   *
   * @return 
   **/
  void saveMatrix( clMatrix &_matrix,const char* _filename,string& _filename2 );

  /**
   * @brief grava arquivo do tipo VTK para visualizacao da solucao do
   * problema. Este arquivo inclui a malha utilizada, um campo para a
   * solucao do sistema (U,V) e um campo para a inclusao de um escalar,
   * neste caso o valores nodais da pressao
   *
   * @param _s objeto do tipo Simulador3D para resgatar os valores
   * nodais dos vetores solucao do problema
   * @param _filename campo para inclusao do diretorio + raiz do nome
   *
   * @return 
   **/
  void saveVTK( const char* _dir,const char* _filename );

  /**
   * @brief grava arquivo do tipo VTK para visualizacao da solucao do
   * problema. Este arquivo inclui a malha utilizada, um campo para a
   * solucao do sistema (U,V) e um campo para a inclusao de um escalar,
   * neste caso o valores nodais da pressao
   *
   * @param _m objeto do tipo Model3D para resgatar os valores
   * nodais dos vetores solucao do problema
   * @param _s objeto do tipo Simulador3D para resgatar os valores das
   * solucoes
   * @param _filename campo para inclusao do diretorio + raiz do nome
   *
   * @return 
   **/
  void saveVTK( const char* _dir,const char* _filename, int _iter );
  void saveVTKSurface( const char* _dir,const char* _filename, int _iter );
  void saveVTKTest( const char* _dir,const char* _filename, int _iter );

  /**
   * @brief imprime em arquivo ASCII a visualizacao de uma matriz
   * qualquer do tipo clMatrix. Este metodo imprimi '-' em valores
   * iguais a 0 e 'X' em valores diferentes de zero. Sua melhor
   * utilizacao eh direcionada a malhas bem pequenas.
   *
   * @param _m matriz do tipo clMatriz para impressao
   * @param _filename no do arquivo a ser criado com a matriz impressa.
   *
   * @return 
   **/
  void matrixPrint( clMatrix &_m,const char* _filename );

  /**
   * @brief imprime em arquivo ASCII a visualizacao de uma matriz
   * qualquer do tipo clDMatrix. Este metodo imprimi '-' em valores
   * iguais a 0 e 'X' em valores diferentes de zero. Sua melhor
   * utilizacao eh direcionada a malhas bem pequenas.
   *
   * @param _m matriz do tipo clDMatriz para impressao
   * @param _filename no do arquivo a ser criado com a matriz impressa.
   *
   * @return 
   **/
  void matrixPrint( clDMatrix &_m,const char* _filename );

  void saveVonKarman( const char* _dir,const char* _filename,int _iter, 
	                  int vertice );
  void savePert( const char* _dir,const char* _filename,int _iter, int vertice);

  void saveVortX(const char* _dir,const char* _filename,int _iter);
  void saveVortY(const char* _dir,const char* _filename,int _iter);
  void saveVortZ(const char* _dir,const char* _filename,int _iter);
  void saveTime(const char* _comment);
  void saveSimTime(int _iter);
  void saveSimTime( const char* _dir,const char* _filename, int _iter );
  int loadIter();
  int loadIter( const char* filename );
  void saveInfo(const char* _dir,const char* _filename,const char* _mesh);
  void printInfo(const char* _mesh);
  void oscillating(int point1,int point2,int point3,const char* _filename);
  void oscillatingD(int point1,int point2,int point3,int point4,
	                int point5,int point6,const char* _filename);
  void oscillating(const char* _dir,const char* _filename, int _iter);
  void oscillatingD(const char* _dir,const char* _filename, int _iter);
  void oscillatingKappa(const char* _dir,const char* _filename, int _iter);
  void saveDistance(const char* _dir,const char* _filename,real time );
  void saveMeshInfo(const char* _dir,const char* _filename );

private:
  Model3D *m;
  int numVerts,numNodes,numElems;
  int numGLEP,numGLEU,numGLEC;
  clVector *X,*Y,*Z;
  clVector *uc,*vc,*wc,*pc,*cc;
  clVector *idbcu,*idbcv,*idbcw,*idbcp,*idbcc;
  clVector *surface,*outflow;
  clMatrix *IEN;

  Simulator3D *s;
  real Re,Sc,We,Fr,dt,cfl,alpha,beta,*simTime;
  clVector *uAnt,*cAnt;
  clMatrix *M,*K,*G,*D,*gx,*gy,*gz;
  clVector *uSol,*vSol,*wSol,*pSol,*cSol;
  clVector *uALE,*vALE,*wALE;
  clVector *distance,*fint;
  clDMatrix *kappa;
};



#endif /* ifndef INOUT_H */

