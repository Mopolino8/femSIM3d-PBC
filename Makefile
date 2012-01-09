## =================================================================== ##
## this is file Makefile, created at 10-Jun-2009                       ##
## maintained by Gustavo Rabello dos Anjos                             ##
## e-mail: gustavo.rabello@gmail.com                                   ##
## =================================================================== ##

# Compilers
CXX = g++ 
CXX = clang

# Flags
CXXFLAGS = -g -fPIC

# Libraries and includes
LIBS += -lgsl -lgslcblas -lm
LIBS += -L. -L${TETGEN_DIR} -ltet
INCLUDES += -I. -I${FEMLIB_DIR}
INCLUDES += -I${PETSC_DIR}/include
INCLUDES += -I${TETGEN_DIR}

# Petsc new config
include ${PETSC_DIR}/conf/variables
include ${PETSC_DIR}/conf/rules


# Sorces
src += ${FEMLIB_DIR}/clVector.cpp
src += ${FEMLIB_DIR}/clMatrix.cpp
src += ${FEMLIB_DIR}/clDMatrix.cpp
src += ${FEMLIB_DIR}/PCGSolver.cpp
src += ${FEMLIB_DIR}/GMRes.cpp
src += ${FEMLIB_DIR}/PetscSolver.cpp
src += ${FEMLIB_DIR}/FEMLinElement3D.cpp
src += ${FEMLIB_DIR}/FEMMiniElement3D.cpp
#src += ${FEMLIB_DIR}/FEMQuadElement3D.cpp
src += $(wildcard ${FEM3D_DIR}/*.cpp)

# Rules
obj = $(src:%.cpp=%.o)

all: single-phase two-phase two-phaseHT

single-phase: diskNuC diskNuZ diskNuCte diskSurf step stepALE 

two-phase: sphere cylinder torus curvatureSphere curvatureCylinder \
           curvatureTorus curvatureAndPressureSphere \
	       curvatureAndPressureCylinder curvatureAndPressureTorus\
		   sessileDrop oscillatingDrop fallingDrop risingBubble \
		   2bubbles micro

two-phaseHT: risingBubbleHT


# --------------------<< backward step (single-phase) >>-------------------- #
#                                                                            #
step: ${FEM3D_DIR}/script/mainStep.o $(obj)
	-${CLINKER} $(obj) $(LIBS) ${PETSC_KSP_LIB} $< -o $@

stepALE: ${FEM3D_DIR}/script/mainStepALE.o $(obj)
	-${CLINKER} $(obj) $(LIBS) ${PETSC_KSP_LIB} $< -o $@
#                                                                            #
# -------------------------------------------------------------------------- #


# ------------------<< rotating disk (single-phase) >>---------------------- #
#                                                                            #
diskNuC: ${FEM3D_DIR}/script/mainDiskNuC.o $(obj)
	 -${CLINKER} $(obj) $(LIBS) ${PETSC_KSP_LIB} $< -o $@

diskNuCte: ${FEM3D_DIR}/script/mainDiskNuCte.o $(obj)
	-${CLINKER} $(obj) $(LIBS) ${PETSC_KSP_LIB} $< -o $@

diskNuZ: ${FEM3D_DIR}/script/mainDiskNuZ.o $(obj)
	-${CLINKER} $(obj) $(LIBS) ${PETSC_KSP_LIB} $< -o $@
#                                                                            #
# -------------------------------------------------------------------------- #


# -----------------<< free surface disk (single-phase) >>------------------- #
#                                                                            #
# free surface (single-phase)
diskSurf: ${FEM3D_DIR}/script/mainDiskSurf.o $(obj)
	-${CLINKER} $(obj) $(LIBS) ${PETSC_KSP_LIB} $< -o $@
#                                                                            #
# -------------------------------------------------------------------------- #


# --------<< benchmarks for sphere,cylinder and torus (two-phase) >>-------- #
#                                                                            #
sphere: ${FEM3D_DIR}/script/mainSphere.o $(obj)
	-${CLINKER} $(obj) $(LIBS) ${PETSC_KSP_LIB} $< -o $@

cylinder: ${FEM3D_DIR}/script/mainCylinder.o $(obj)
	-${CLINKER} $(obj) $(LIBS) ${PETSC_KSP_LIB} $< -o $@

torus: ${FEM3D_DIR}/script/mainTorus.o $(obj)
	-${CLINKER} $(obj) $(LIBS) ${PETSC_KSP_LIB} $< -o $@

curvatureSphere: ${FEM3D_DIR}/script/mainCurvatureSphere.o $(obj)
	-${CLINKER} $(obj) $(LIBS) ${PETSC_KSP_LIB} $< -o $@

curvatureCylinder: ${FEM3D_DIR}/script/mainCurvatureCylinder.o $(obj)
	-${CLINKER} $(obj) $(LIBS) ${PETSC_KSP_LIB} $< -o $@

curvatureTorus: ${FEM3D_DIR}/script/mainCurvatureTorus.o $(obj)
	-${CLINKER} $(obj) $(LIBS) ${PETSC_KSP_LIB} $< -o $@

curvatureAndPressureSphere: ${FEM3D_DIR}/script/mainCurvatureAndPressureSphere.o $(obj)
	-${CLINKER} $(obj) $(LIBS) ${PETSC_KSP_LIB} $< -o $@

curvatureAndPressureCylinder: ${FEM3D_DIR}/script/mainCurvatureAndPressureCylinder.o $(obj)
	-${CLINKER} $(obj) $(LIBS) ${PETSC_KSP_LIB} $< -o $@

curvatureAndPressureTorus: ${FEM3D_DIR}/script/mainCurvatureAndPressureTorus.o $(obj)
	-${CLINKER} $(obj) $(LIBS) ${PETSC_KSP_LIB} $< -o $@

sessileDrop: ${FEM3D_DIR}/script/mainSessileDrop.o $(obj)
	-${CLINKER} $(obj) $(LIBS) ${PETSC_KSP_LIB} $< -o $@

oscillatingDrop: ${FEM3D_DIR}/script/mainOscillatingDrop.o $(obj)
	-${CLINKER} $(obj) $(LIBS) ${PETSC_KSP_LIB} $< -o $@

fallingDrop: ${FEM3D_DIR}/script/mainFallingDrop.o $(obj)
	-${CLINKER} $(obj) $(LIBS) ${PETSC_KSP_LIB} $< -o $@

risingBubble: ${FEM3D_DIR}/script/mainRisingBubble.o $(obj)
	-${CLINKER} $(obj) $(LIBS) ${PETSC_KSP_LIB} $< -o $@
#                                                                            #
# -------------------------------------------------------------------------- #


# --------------------------<< misc (two-phase) >>-------------------------- #
#                                                                            #
2bubbles: ${FEM3D_DIR}/script/main2Bubbles.o $(obj)
	-${CLINKER} $(obj) $(LIBS) ${PETSC_KSP_LIB} $< -o $@
#                                                                            #
# -------------------------------------------------------------------------- #


# -----------------------<< microchannels (two-phase) >>-------------------- #
#                                                                            #
micro: ${FEM3D_DIR}/script/mainMicro.o $(obj)
	-${CLINKER} $(obj) $(LIBS) ${PETSC_KSP_LIB} $< -o $@
#                                                                            #
# -------------------------------------------------------------------------- #


# ---------------------<< heat-transfer (two-phase) >>---------------------- #
#                                                                            #
risingBubbleHT: ${FEM3D_DIR}/script/mainRisingBubbleHT.o $(obj)
	-${CLINKER} $(obj) $(LIBS) ${PETSC_KSP_LIB} $< -o $@
#                                                                            #
# -------------------------------------------------------------------------- #

#--------------------------------------------------
# step: ./script/mainStep.o libtest.so
# 	$(CXX) -L. -ltest -o $@ $<
#-------------------------------------------------- 

libtest.so: $(obj)
	$(CXX) -shared $(LIBS) ${PETSC_KSP_LIB} $(INCLUDES) $(obj) -o $@

#--------------------------------------------------
# libtest: $(obj)
# 	$(CXX) -dynamiclib $(LIBS) ${PETSC_KSP_LIB} $(INCLUDES) $(obj) -o $@
# 	ln -s libtest libtest.dylib
#-------------------------------------------------- 

%.o: %.cpp $(wildcard *.h)
	$(CXX) $(INCLUDES) -c $< $(CXXFLAGS) -o $@
	
erase:
	@rm -f core
	@find . -name "*~" -exec rm {} \;
	@rm -f ./vtk/*.vtk ./vtk/*.vtu
	@rm -f ./msh/*.msh
	@rm -f ./sim/vk*
	@rm -f ./sim/sim*.dat
	@rm -f ./bin/*.bin
	@rm -f ./dat/*.dat
	@rm -f ./dat/vk*
	@rm -f ./dat/edge.*

deepclean: 
	@find . -type f -executable -exec rm {} \;
	@rm -f libtest*
	@rm -f core
	@find ${FEMLIB_DIR} -name "*.o" -exec rm {} \;
	@find . -name "*.o" -exec rm {} \;
	@find . -name "*~" -exec rm {} \;
	@rm -f ./vtk/*.vtk
	@rm -f ./msh/*.msh
	@rm -f ./sim/vk*.dat
	@rm -f ./sim/sim*.dat
	@rm -f ./bin/*.bin
	@rm -f ./*.dat
	@rm -f ./dat/*.dat
	@rm -f ./dat/vk*
	@rm -f ./dat/edge.*

