#include "sionstep.h"
#include "areatime.h"
#include "errorhandler.h"
#include "readfunc.h"
#include "readword.h"
#include "loglinearregression.h"
#include "gadget.h"

extern ErrorHandler handle;

SIOnStep::~SIOnStep() {
}

SIOnStep::SIOnStep(CommentStream& infile, const char* datafilename, const CharPtrVector& areaindex,
  const TimeClass* const TimeInfo, int numcols, const IntMatrix& areas,
  const CharPtrVector& index1, const CharPtrVector& index2)
  : Areas(areas) {

  char text[MaxStrLength];
  strncpy(text, "", MaxStrLength);

  error = 0;
  NumberOfSums = 0;
  slope = 0.0;
  intercept = 0.0;

  //if numcols is 1 then this is a sibyalengthandageonstep
  //else we have a pionstep - these use different fittypes

  if (numcols == 1) {
    infile >> text;
    if (strcasecmp(text, "linearfit") == 0)
      fittype = LINEARFIT;
    else if (strcasecmp(text, "powerfit") == 0)
      fittype = POWERFIT;
    else
      handle.Message("Error in surveyindex - unrecognised fittype", text);

  } else {
    infile >> text;
    if (strcasecmp(text, "linearfit") == 0)
      fittype = LINEARFIT;
    else if (strcasecmp(text, "loglinearfit") == 0)
      fittype = LOGLINEARFIT;
    else if (strcasecmp(text, "fixedslopelinearfit") == 0)
      fittype = FIXEDSLOPELINEARFIT;
    else if (strcasecmp(text, "fixedslopeloglinearfit") == 0)
      fittype = FIXEDSLOPELOGLINEARFIT;
    else if (strcasecmp(text, "fixedinterceptlinearfit") == 0)
      fittype = FIXEDINTERCEPTLINEARFIT;
    else if (strcasecmp(text, "fixedinterceptloglinearfit") == 0)
      fittype = FIXEDINTERCEPTLOGLINEARFIT;
    else if (strcasecmp(text, "fixedlinearfit") == 0)
      fittype = FIXEDLINEARFIT;
    else if (strcasecmp(text, "fixedloglinearfit") == 0)
      fittype = FIXEDLOGLINEARFIT;
    else
      handle.Message("Error in surveyindex - unrecognised fittype", text);

    switch(fittype) {
      case LINEARFIT:
      case LOGLINEARFIT:
        break;
      case FIXEDSLOPELINEARFIT:
      case FIXEDSLOPELOGLINEARFIT:
        readWordAndVariable(infile, "slope", slope);
        break;
      case FIXEDINTERCEPTLINEARFIT:
      case FIXEDINTERCEPTLOGLINEARFIT:
        readWordAndVariable(infile, "intercept", intercept);
        break;
      case FIXEDLINEARFIT:
      case FIXEDLOGLINEARFIT:
        readWordAndVariable(infile, "slope", slope);
        readWordAndVariable(infile, "intercept", intercept);
        break;
      default:
        handle.Message("Error in surveyindex - unrecognised fittype");
        break;
    }
  }

  //read the year and step data from the datafile
  ifstream datafile;
  CommentStream subdata(datafile);
  datafile.open(datafilename, ios::in);
  handle.checkIfFailure(datafile, datafilename);
  handle.Open(datafilename);
  readSIData(subdata, areaindex, index1, index2, TimeInfo);
  handle.Close();
  datafile.close();
  datafile.clear();

  Indices.AddRows(Years.Size(), numcols, 0.0);
  abundance.AddRows(Years.Size(), numcols, 0.0);
  if (numcols != 1) {
    slopes.resize(numcols);
    intercepts.resize(numcols);
    sse.resize(numcols);
  }
}

SIOnStep::SIOnStep(CommentStream& infile, const char* datafilename, const CharPtrVector& areaindex,
  const TimeClass* const TimeInfo, const IntMatrix& areas, const CharPtrVector& colindex,
  const char* name) : Areas(areas) {

  char text[MaxStrLength];
  strncpy(text, "", MaxStrLength);

  error = 0;
  NumberOfSums = 0;
  slope = 0.0;
  intercept = 0.0;

  //read the fittype
  infile >> text;
  if (strcasecmp(text, "linearfit") == 0)
    fittype = LINEARFIT;
  else if (strcasecmp(text, "loglinearfit") == 0)
    fittype = LOGLINEARFIT;
  else if (strcasecmp(text, "fixedslopelinearfit") == 0)
    fittype = FIXEDSLOPELINEARFIT;
  else if (strcasecmp(text, "fixedslopeloglinearfit") == 0)
    fittype = FIXEDSLOPELOGLINEARFIT;
  else if (strcasecmp(text, "fixedinterceptlinearfit") == 0)
    fittype = FIXEDINTERCEPTLINEARFIT;
  else if (strcasecmp(text, "fixedinterceptloglinearfit") == 0)
    fittype = FIXEDINTERCEPTLOGLINEARFIT;
  else if (strcasecmp(text, "fixedlinearfit") == 0)
    fittype = FIXEDLINEARFIT;
  else if (strcasecmp(text, "fixedloglinearfit") == 0)
    fittype = FIXEDLOGLINEARFIT;
  else
    handle.Message("Error in surveyindex - unrecognised fittype", text);

  switch(fittype) {
    case LINEARFIT:
    case LOGLINEARFIT:
      break;
    case FIXEDSLOPELINEARFIT:
    case FIXEDSLOPELOGLINEARFIT:
      readWordAndVariable(infile, "slope", slope);
      break;
    case FIXEDINTERCEPTLINEARFIT:
    case FIXEDINTERCEPTLOGLINEARFIT:
      readWordAndVariable(infile, "intercept", intercept);
      break;
    case FIXEDLINEARFIT:
    case FIXEDLOGLINEARFIT:
      readWordAndVariable(infile, "slope", slope);
      readWordAndVariable(infile, "intercept", intercept);
      break;
    default:
      handle.Message("Error in surveyindex - unrecognised fittype");
      break;
  }

  //read the survey indices data from the datafile
  ifstream datafile;
  CommentStream subdata(datafile);
  datafile.open(datafilename, ios::in);
  handle.checkIfFailure(datafile, datafilename);
  handle.Open(datafilename);
  readSIData(subdata, areaindex, colindex, TimeInfo, name);
  handle.Close();
  datafile.close();
  datafile.clear();

  if ((Indices.Nrow() > 0) && (Indices.Ncol() > 0)) {
    abundance.AddRows(Indices.Nrow(), Indices.Ncol(), 0.0);
    slopes.resize(Indices.Ncol());
    intercepts.resize(Indices.Ncol());
    sse.resize(Indices.Ncol());
  }
}

void SIOnStep::readSIData(CommentStream& infile, const CharPtrVector& areaindex,
  const CharPtrVector& colindex, const TimeClass* const TimeInfo, const char* name) {

  int i;
  int year, step;
  double tmpnumber;
  char tmparea[MaxStrLength], tmplabel[MaxStrLength];
  strncpy(tmparea, "", MaxStrLength);
  strncpy(tmplabel, "", MaxStrLength);
  int keepdata, timeid, colid, areaid;
  int count = 0;

  //Check the number of columns in the inputfile
  if (countColumns(infile) != 5)
    handle.Message("Wrong number of columns in inputfile - should be 5");

  while (!infile.eof()) {
    keepdata = 0;
    infile >> year >> step >> tmparea >> tmplabel >> tmpnumber >> ws;

    //check if the year and step are in the simulation
    timeid = -1;
    if (TimeInfo->IsWithinPeriod(year, step)) {
      for (i = 0; i < Years.Size(); i++)
        if ((Years[i] == year) && (Steps[i] == step))
          timeid = i;

    } else
      keepdata = 1;

    //if tmparea is in areaindex keep data, else dont keep the data
    areaid = -1;
    for (i = 0; i < areaindex.Size(); i++)
      if (strcasecmp(areaindex[i], tmparea) == 0)
        areaid = i;

    if (areaid == -1)
      keepdata = 1;

    //if tmplabel is in colindex find colid, else dont keep the data
    colid = -1;
    for (i = 0; i < colindex.Size(); i++)
      if (strcasecmp(colindex[i], tmplabel) == 0)
        colid = i;

    if (colid == -1)
      keepdata = 1;

    if (keepdata == 0) {
      //survey indices data is required, so store it
      count++;
      if (timeid == -1) {
        Years.resize(1, year);
        Steps.resize(1, step);
        timeid = (Years.Size() - 1);
        Indices.AddRows(1, colindex.Size(), 0.0);
      }
      Indices[timeid][colid] = tmpnumber;
    }

    //Keep all years from the input file in a vector [mnaa 25.04.00]
    if (YearsInFile.Size() == 0 || year != YearsInFile[YearsInFile.Size() - 1])
      YearsInFile.resize(1, year);
  }
  AAT.AddActions(Years, Steps, TimeInfo);
  if (count == 0)
    handle.LogWarning("Warning in surveyindex - found no data in the data file for", name);
  handle.LogMessage("Read surveyindex data file - number of entries", count);
}

void SIOnStep::readSIData(CommentStream& infile, const CharPtrVector& areaindex,
  const CharPtrVector& index1, const CharPtrVector& index2, const TimeClass* const TimeInfo) {

  int i;
  int year, step;
  double tmpnumber;
  char tmparea[MaxStrLength], tmpstr1[MaxStrLength], tmpstr2[MaxStrLength];
  strncpy(tmparea, "", MaxStrLength);
  strncpy(tmpstr1, "", MaxStrLength);
  strncpy(tmpstr2, "", MaxStrLength);
  int keepdata, timeid, areaid, id;

  //for sibylengthandageonstep tmpstr1 is age, tmpstr2 is length
  //for pionstep tmpstr1 is predlength, tmpstr2 is preylength
  //check the number of columns in the inputfile
  if (countColumns(infile) != 6)
    handle.Message("Wrong number of columns in inputfile - should be 6");

  while (!infile.eof()) {
    keepdata = 0;
    infile >> year >> step >> tmparea >> tmpstr1 >> tmpstr2 >> tmpnumber >> ws;

    //check if the year and step are in the simulation
    timeid = -1;
    if (TimeInfo->IsWithinPeriod(year, step)) {
      //if this is a new timestep, resize to store the data
      for (i = 0; i < Years.Size(); i++)
        if ((Years[i] == year) && (Steps[i] == step))
          timeid = i;

    } else
      keepdata = 1;

    //if tmparea is in areaindex keep data, else dont keep the data
    areaid = -1;
    for (i = 0; i < areaindex.Size(); i++)
      if (strcasecmp(areaindex[i], tmparea) == 0)
        areaid = i;

    if (areaid == -1)
      keepdata = 1;

    //if tmpstr1 is in index1 keep data, else dont keep the data
    id = -1;
    for (i = 0; i < index1.Size(); i++)
      if (strcasecmp(index1[i], tmpstr1) == 0)
        id = i;

    if (id == -1)
      keepdata = 1;

    //if tmpstr2 is in index2 keep data, else dont keep the data
    id = -1;
    for (i = 0; i < index2.Size(); i++)
      if (strcasecmp(index2[i], tmpstr2) == 0)
        id = i;

    if (id == -1)
      keepdata = 1;

    if (keepdata == 0) {
      //survey indices data is required, so store it
      if (timeid == -1) {
        Years.resize(1, year);
        Steps.resize(1, step);
      }
    }

    //Keep all years from the input file in a vector [mnaa 25.04.00]
    if (YearsInFile.Size() == 0 || year != YearsInFile[YearsInFile.Size() - 1])
      YearsInFile.resize(1, year);
  }
  AAT.AddActions(Years, Steps, TimeInfo);
}

void SIOnStep::Reset(const Keeper* const keeper) {
  error = 0;
  NumberOfSums = 0;
  int i, j;
  for (i = 0; i < abundance.Nrow(); i++)
    for (j = 0; j < abundance.Ncol(i); j++)
      abundance[i][j] = 0.0;
}

void SIOnStep::Print(ofstream& outfile) const {
  int i, j;
  outfile << "\tSurvey indices\n";
  for (i = 0; i < Years.Size(); i++) {
    outfile << TAB << Years[i] << sep << Steps[i] << sep;
    for (j = 0; j < Indices.Ncol(i); j++) {
      outfile.width(smallwidth);
      outfile.precision(smallprecision);
      outfile << Indices[i][j] << sep;
    }
    outfile << endl;
  }
  outfile.flush();
}

void SIOnStep::LikelihoodPrint(ofstream& outfile) {
  int i, j;
  outfile << "Number of sums " << NumberOfSums << "\nError " << error << "\nSurvey indices\n";
  for (i = 0; i < Years.Size(); i++) {
    outfile << TAB << Years[i] << sep << Steps[i] << sep;
    for (j = 0; j < Indices.Ncol(i); j++) {
      outfile.width(smallwidth);
      outfile.precision(smallprecision);
      outfile << Indices[i][j] << sep;
    }
    outfile << endl;
  }
  outfile << "Abundance numbers\n";
  for (i = 0; i < Years.Size(); i++) {
    outfile << TAB << Years[i] << sep << Steps[i] << sep;
    for (j = 0; j < abundance.Ncol(i); j++) {
      outfile.width(smallwidth);
      outfile.precision(smallprecision);
      outfile << abundance[i][j] << sep;
    }
    outfile << endl;
  }

  outfile << "Regression information\nintercept ";
  for (j = 0; j < intercepts.Size(); j++) {
    outfile.width(smallwidth);
    outfile.precision(smallprecision);
    outfile << intercepts[j] << sep;
  }
  outfile << "\nslope ";
  for (j = 0; j < slopes.Size(); j++) {
    outfile.width(smallwidth);
    outfile.precision(smallprecision);
    outfile << slopes[j] << sep;
  }
  outfile << "\nsse   ";
  for (j = 0; j < sse.Size(); j++) {
    outfile.width(smallwidth);
    outfile.precision(smallprecision);
    outfile << sse[j] << sep;
  }
  outfile.flush();
}

int SIOnStep::IsToSum(const TimeClass* const TimeInfo) const {
  return AAT.AtCurrentTime(TimeInfo);
}

double SIOnStep::Regression() {
  if (NumberOfSums < 2)
    return 0.0;

  double likelihood = 0.0;
  int col, index;
  for (col = 0; col < Indices.Ncol(); col++) {
    //Let LLR figure out what to do in the case of zero stock size.
    DoubleVector indices(NumberOfSums);
    DoubleVector stocksize(NumberOfSums);
    for (index = 0; index < NumberOfSums; index++) {
      indices[index] = Indices[index][col];
      stocksize[index] = abundance[index][col];
    }
    //Now fit the log of the abundance indices as a function of stock size.
    likelihood += this->Fit(stocksize, indices, col);
  }
  return likelihood;
}

void SIOnStep::KeepNumbers(const DoubleVector& numbers) {
  assert(numbers.Size() == abundance.Ncol());
  int i;
  for (i = 0; i < numbers.Size(); i++)
    abundance[NumberOfSums][i] = numbers[i];
  NumberOfSums++;
}

double SIOnStep::Fit(const DoubleVector& stocksize, const DoubleVector& indices, int col) {

  LogLinearRegression LLR;
  LinearRegression LR;

  //fit the data to the (log) linear regression curve
  switch(fittype) {
    case LOGLINEARFIT:
      LLR.Fit(stocksize, indices);
      break;
    case FIXEDSLOPELOGLINEARFIT:
      LLR.Fit(stocksize, indices, slope);
      break;
    case FIXEDINTERCEPTLOGLINEARFIT:
      LLR.Fit(intercept, stocksize, indices);
      break;
    case FIXEDLOGLINEARFIT:
      LLR.Fit(stocksize, indices, slope, intercept);
      break;
    case LINEARFIT:
      LR.Fit(stocksize, indices);
      break;
    case FIXEDSLOPELINEARFIT:
      LR.Fit(stocksize, indices, slope);
      break;
    case FIXEDINTERCEPTLINEARFIT:
      LR.Fit(intercept, stocksize, indices);
      break;
    case FIXEDLINEARFIT:
      LR.Fit(stocksize, indices, slope, intercept);
      break;
    default:
      handle.LogWarning("Warning in surveyindex - unknown fittype", fittype);
      break;
  }

  //and then store the results
  switch(fittype) {
    case LOGLINEARFIT:
    case FIXEDSLOPELOGLINEARFIT:
    case FIXEDINTERCEPTLOGLINEARFIT:
    case FIXEDLOGLINEARFIT:
      slopes[col] = LLR.slope();
      intercepts[col] = LLR.intersection();
      sse[col] = LLR.SSE();
      if (LLR.Error())
        this->setError();
      return LLR.SSE();
      break;
    case LINEARFIT:
    case FIXEDSLOPELINEARFIT:
    case FIXEDINTERCEPTLINEARFIT:
    case FIXEDLINEARFIT:
      slopes[col] = LR.slope();
      intercepts[col] = LR.intersection();
      sse[col] = LR.SSE();
      if (LR.Error())
        this->setError();
      return LR.SSE();
      break;
    default:
      handle.LogWarning("Warning in surveyindex - unknown fittype", fittype);
      break;
  }
  return 0.0;
}
