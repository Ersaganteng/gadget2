##########################################################################
# Common for all architecture and compiler options
##########################################################################
GCCWARNINGS = -Wimplicit -Wreturn-type -Wswitch -Wcomment -Wformat \
              -Wparentheses -Wpointer-arith -Wcast-qual -Wconversion \
              -Wreorder -Wwrite-strings -Wsynth -Wchar-subscripts \
              -Wuninitialized -W -pedantic


ifeq ($(OS),Windows_NT)
	DEFINE_FLAGS = -D NDEBUG -D INTERRUPT_HANDLER -O3 -I headers/ 
else
	DEFINE_FLAGS = -D NDEBUG -D INTERRUPT_HANDLER -O3 -I headers/ -D NOT_WINDOWS
endif
#-s

INC_DIR = ./headers
SRC_DIR = ./src
OBJ_DIR = .

##########################################################################
# The name of the final executable (eg gadget-paramin or gadget.exe)
#GADGET = gadget-paramin
#GADGET = gadget
GADGET = gadget
#GADGET = gadget-ompSpec
##########################################################################
# Pick the appropriate compiler from the following switches
##########################################################################
# 1. Linux, or Cygwin, or Solaris, with MPI, mpic++ compiler
#CXX = mpic++
#LIBDIRS = -L. -L/usr/local/lib
#LIBRARIES = -lm
#CXXFLAGS = $(GCCWARNINGS) $(DEFINE_FLAGS) -D GADGET_NETWORK
#OBJECTS = $(GADGETINPUT) $(GADGETOBJECTS) $(SLAVEOBJECTS)
_LIBOBJ = $(GADGETINPUT) $(EXTRAINPUT)
LIBOBJ = $(patsubst %,$(SRC_DIR)/%,$(_LIBOBJ))
##########################################################################
# 2. Linux, Mac, Cgwin or Solaris, without MPI, using g++ compiler - sequential
#CXX = g++
#LIBDIRS = -L. -L/usr/local/lib
#LIBRARIES = -lm
#CXXFLAGS = $(DEFINE_FLAGS)
#OBJECTS = $(GADGETINPUT) $(GADGETOBJECTS)
##########################################################################
# 2,5. Linux, Mac, Cgwin or Solaris, without MPI, using g++ compiler and OpenMP Reproducible
CXX = g++
LIBDIRS = -L. -L/usr/local/lib
LIBRARIES = -lm
CXXFLAGS = $(DEFINE_FLAGS) -fopenmp
_OBJECTS = $(GADGETINPUT) $(GADGETOBJECTS) 
OBJECTS = $(patsubst %,$(SRC_DIR)/%,$(_OBJECTS))
##########################################################################
# 2,6. Linux, Mac, Cgwin or Solaris, without MPI, using g++ compiler and OpenMP Speculative
#CXX = g++
#LIBDIRS = -L. -L/usr/local/lib
#LIBRARIES = -lm
#CXXFLAGS = $(DEFINE_FLAGS) -fopenmp -DSPECULATIVE
#OBJECTS = $(GADGETINPUT) $(GADGETOBJECTS)
##########################################################################
# 3. Solaris, without pvm3, using CC compiler
#CXX = CC
#LIBDIRS = -L. -L/usr/local/lib
#LIBRARIES = -lm
#CXXFLAGS = $(DEFINE_FLAGS)
#OBJECTS = $(GADGETINPUT) $(GADGETOBJECTS)
##########################################################################
# 4. Linux or Solaris, without pvm3, g++ compiler running CONDOR
#CXX = condor_compile g++
#LIBDIRS = -L. -L/usr/local/lib
#LIBRARIES = -lm
#CXXFLAGS = $(GCCWARNINGS) $(DEFINE_FLAGS)
#OBJECTS = $(GADGETINPUT) $(GADGETOBJECTS)
##########################################################################

GADGETOBJECTS = gadget.o ecosystem.o initialize.o simulation.o fleet.o otherfood.o \
    area.o time.o keeper.o maininfo.o printinfo.o runid.o global.o stochasticdata.o \
    timevariable.o formula.o readaggregation.o readfunc.o readmain.o readword.o \
    actionattimes.o livesonareas.o lengthgroup.o conversionindex.o selectfunc.o \
    suitfunc.o suits.o popinfomemberfunctions.o popratio.o popstatistics.o \
    interruptinterface.o agebandmatrix.o agebandmatrixmemberfunctions.o tags.o \
    interrupthandler.o agebandmatrixratio.o agebandmatrixratiomemberfunctions.o \
    migrationarea.o rectangle.o ludecomposition.o multinomial.o regressionline.o \
    stock.o stockmemberfunctions.o renewal.o spawner.o stray.o transition.o \
    grow.o grower.o growermemberfunctions.o growthcalc.o taggrow.o maturity.o \
    initialcond.o migration.o naturalm.o lengthprey.o prey.o stockprey.o \
    effortpredator.o lengthpredator.o linearpredator.o numberpredator.o \
    quotapredator.o predator.o poppredator.o stockpredator.o totalpredator.o \
    recaggregator.o predatorpreyaggregator.o stockpreyaggregator.o \
    predatoraggregator.o predatoroveraggregator.o preyoveraggregator.o \
    fleetpreyaggregator.o stockaggregator.o fleeteffortaggregator.o stockstdprinter.o \
    predatorprinter.o predatoroverprinter.o predatorpreyprinter.o preyoverprinter.o \
    stockfullprinter.o stockpreyfullprinter.o stockpreyprinter.o stockprinter.o \
    likelihoodprinter.o summaryprinter.o boundlikelihood.o migrationpenalty.o \
    catchdistribution.o catchinkilos.o catchstatistics.o understocking.o \
    recapture.o recstatistics.o stockdistribution.o stomachcontent.o \
    surveydistribution.o surveyindices.o sionstep.o sibyacousticonstep.o \
    sibyageonstep.o sibyfleetonstep.o sibylengthonstep.o sibyeffortonstep.o \
    optinfobfgs.o optinfohooke.o optinfosimann.o bfgs.o hooke.o simann.o \
    addresskeepervector.o addresskeepermatrix.o intmatrix.o doublematrix.o \
    agebandmatrixptrvector.o agebandmatrixptrmatrix.o agebandmatrixratioptrvector.o \
    doublematrixptrvector.o doublematrixptrmatrix.o timevariablevector.o \
    formulavector.o formulamatrix.o formulamatrixptrvector.o charptrmatrix.o \
    popinfovector.o popinfomatrix.o popinfoindexvector.o suitfuncptrvector.o \
    popratiovector.o popratiomatrix.o popratioindexvector.o fleetptrvector.o \
    baseclassptrvector.o conversionindexptrvector.o likelihoodptrvector.o \
    predatorptrvector.o preyptrvector.o printerptrvector.o stockptrvector.o \
    migrationareaptrvector.o rectangleptrvector.o otherfoodptrvector.o \
    tagptrvector.o optinfoptrvector.o errorhandler.o modelvariablevector.o \
    stockvariable.o modelvariable.o migrationproportion.o matrix.o
    
    GADGETCLASSES = src/agebandmatrixmemberfunctions.cc src/grow.cc src/growermemberfunctions.cc

SLAVEOBJECTS = netdata.o slavecommunication.o pvmconstants.o

GADGETINPUT = intvector.o doublevector.o charptrvector.o initialinputfile.o \
    commentstream.o parameter.o parametervector.o strstack.o 

LDFLAGS = $(CXXFLAGS) $(LIBDIRS) $(LIBRARIES)

partial :	$(OBJECTS)
		$(CXX) -c $(LDFLAGS) $(GADGETCLASSES)

gadget	:	$(OBJECTS)
		$(CXX) -o $(GADGET) $(OBJECTS) $(LDFLAGS)

## you need root permission to be able to do this ...
install	:	$(GADGET)
		strip $(GADGET)
		cp $(GADGET) /usr/local/bin/
		cp gadget.1 /usr/local/man/man1/

##########################################################################
# The following line is needed to create the gadget input library
# for newer versions of paramin.  To create this library, you need
# to type "make libgadgetinput.a" *before* you compile paramin
##########################################################################
EXTRAINPUT = optinfoptrvector.o doublematrix.o runid.o global.o errorhandler.o
libgadgetinput.a	:	$(GADGETINPUT) $(EXTRAINPUT)
		ar rs libgadgetinput.a $?

clean	:
		rm -f $(OBJECTS) libgadgetinput.a

depend	:
		$(CXX) -M -MM $(CXXFLAGS) *.cc

# DO NOT DELETE THIS LINE -- make depend depends on it.
