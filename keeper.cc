#include "keeper.h"
#include "errorhandler.h"
#include "runid.h"
#include "ecosystem.h"
#include "gadget.h"

extern RunID RUNID;
extern Ecosystem* EcoSystem;
extern ErrorHandler handle;

Keeper::Keeper() {
  stack = new StrStack();
  likcompnames = new StrStack();
}

void Keeper::KeepVariable(double& value, const Parameter& attr) {

  //Try to find the value attr in the vector switches.
  int i, index = -1;
  for (i = 0; i < switches.Size(); i++)
    if (switches[i] == attr)
      index = i;

  if (index == -1) {
    //attr was not found -- add it to switches and values.
    index = switches.Size();
    switches.resize(1);
    values.resize(1);
    lowerbds.resize(1, -9999);  // default lower bound
    upperbds.resize(1, 9999);   // default upper bound
    opt.resize(1, 1);
    scaledvalues.resize(1, 1.0);
    initialvalues.resize(1, 1.0);
    address.AddRows(1, 1);
    switches[index] = attr;
    values[index] = value;
    address[index][0] = &value;
    address[index][0] = stack->SendAll();

  } else {
    if (value != values[index]) {
      handle.Message("Error in keeper - received a switch with a repeated name but different value");

    } else {
      //Everything is ok, just add the address &value to address.
      int secindex = address[index].Size();
      address[index].resize(1);
      address[index][secindex] = &value;
      address[index][secindex] = stack->SendAll();
    }
  }
}

Keeper::~Keeper() {
  ClearAll();
  delete stack;
  ClearComponents();
  delete likcompnames;
}

void Keeper::DeleteParam(const double& var) {
  int found = 0;
  int i, j;
  for (i = 0; i < address.Nrow() && !found; i++) {
    for (j = 0; j < address[i].Size() && !found; j++) {
      if (address[i][j] == &var) {
        address[i].Delete(j);
        found = 1;
        if (address[i].Size() == 0) {
          //The variable we deleted was the only one with this switch.
          address.DeleteRow(i);
          switches.Delete(i);
          values.Delete(i);
          opt.Delete(i);
          lowerbds.Delete(i);
          upperbds.Delete(i);
          i--;
        }
      }
    }
  }
}

void Keeper::ChangeVariable(const double& pre, double& post) {
  int found = 0;
  int i, j = 0;
  for (i = 0; i < address.Nrow() && !found; i++)
    for (j = 0; j < address[i].Size() && !found; j++)
      if (address[i][j] == &pre) {
        address[i][j] = &post;
        found = 1;
      }

  if (found == 1 && pre != post) {
    handle.LogWarning("Error in keeper - failed to change variables");
    exit(EXIT_FAILURE);
  }
}

void Keeper::ClearLast() {
  stack->OutOfStack();
}

void Keeper::ClearLastAddString(const char* str) {
  stack->OutOfStack();
  stack->PutInStack(str);
}

void Keeper::ClearAll() {
  stack->ClearStack();
}

void Keeper::setString(const char* str) {
  stack->ClearStack();
  stack->PutInStack(str);
}

void Keeper::AddString(const char* str) {
  stack->PutInStack(str);
}

void Keeper::AddComponent(const char* name) {
  likcompnames->PutInStack("\t");
  likcompnames->PutInStack(name);
}

char* Keeper::SendComponents() const {
  return likcompnames->SendAll();
}

void Keeper::ClearComponents() {
  likcompnames->ClearStack();
}

int Keeper::NoVariables() const {
  return switches.Size();
}

int Keeper::NoOptVariables() const {
  int i, nr;
  nr = 0;
  if (opt.Size() == 0)
    return switches.Size();
  else
    for (i = 0; i < opt.Size(); i++)
      if (opt[i] == 1)
        nr++;

  return nr;
}

void Keeper::ValuesOfVariables(DoubleVector& val) const {
  int i;
  for (i = 0; i < values.Size(); i++)
    val[i] = values[i];
}

void Keeper::InitialValues(DoubleVector& val) const {
  int i;
  for (i = 0; i < values.Size(); i++)
    val[i] = initialvalues[i];
}

void Keeper::ScaledValues(DoubleVector& val) const {
  int i;
  for (i = 0; i < values.Size(); i++)
    val[i] = scaledvalues[i];
}

void Keeper::ScaledOptValues(DoubleVector& val) const {
  int i, k;
  if (val.Size() != this->NoOptVariables()) {
    handle.LogWarning("Warning in keeper - illegal number of optimising variables");
    return;
  }
  if (opt.Size() == 0)
    this->ScaledValues(val);
  else {
    k = 0;
    for (i = 0; i < scaledvalues.Size(); i++)
      if (opt[i] == 1) {
        val[k] = scaledvalues[i];
        k++;
      }
  }
}

void Keeper::OptValues(DoubleVector& val) const {
  int i, k;
  if (val.Size() != this->NoOptVariables()) {
    handle.LogWarning("Warning in keeper - illegal number of optimising variables");
    return;
  }
  if (opt.Size() == 0)
    this->ValuesOfVariables(val);
  else {
    k = 0;
    for (i = 0; i < values.Size(); i++)
      if (opt[i] == 1) {
        val[k] = values[i];
        k++;
      }
  }
}

void Keeper::InitialOptValues(DoubleVector& val) const {
  int i, k;
  if (val.Size() != this->NoOptVariables()) {
    handle.LogWarning("Warning in keeper - illegal number of optimising variables");
    return;
  }
  if (opt.Size() == 0)
    this->InitialValues(val);
  else {
    k = 0;
    for (i = 0; i < initialvalues.Size(); i++)
      if (opt[i] == 1) {
        val[k] = initialvalues[i];
        k++;
      }
  }
}

void Keeper::OptSwitches(ParameterVector& sw) const {
  int i, k;
  if (sw.Size() != this->NoOptVariables()) {
    handle.LogWarning("Warning in keeper - illegal number of optimising variables");
    return;
  }
  if (opt.Size() == 0)
    this->Switches(sw);
  else {
    k = 0;
    for (i = 0; i < switches.Size(); i++)
      if (opt[i] == 1) {
        sw[k] = switches[i];
        k++;
      }
  }
}

void Keeper::ScaleVariables() {
  int i;
  for (i = 0; i < values.Size(); i++) {
    if (isZero(values[i])) {
      handle.LogWarning("Warning in keeper - cannot scale switch with initial value zero", switches[i].getValue());
      initialvalues[i] = 1.0;
      scaledvalues[i] = values[i];
    } else {
      initialvalues[i] = values[i];
      scaledvalues[i] = 1.0;
    }
  }
}

void Keeper::Update(const DoubleVector& val) {
  int i, j;
  if (val.Size() != values.Size()) {
    error = 1;
    return;
  }
  for (i = 0; i < address.Nrow(); i++) {
    for (j = 0; j < address.Ncol(i); j++)
      *address[i][j].addr = val[i];

    values[i] = val[i];
    if (isZero(initialvalues[i])) {
      handle.LogWarning("Warning in keeper - cannot scale switch with initial value zero", switches[i].getValue());
      scaledvalues[i] = val[i];
    } else
      scaledvalues[i] = val[i] / initialvalues[i];
  }
}

//Memberfunction added so boundlikelihood could change variables
void Keeper::Update(int pos, double& value) {
  int i;
  if (pos <= 0 && pos >= address.Nrow()) {
    error = 1;
    return;
  }
  for (i = 0; i < address.Ncol(pos); i++)
    *address[pos][i].addr = value;

  values[pos] = value;
  if (isZero(initialvalues[pos])) {
    handle.LogWarning("Warning in keeper - cannot scale switch with initial value zero", switches[pos].getValue());
    scaledvalues[pos] = value;
  } else
    scaledvalues[pos] = value / initialvalues[pos];
}

void Keeper::writeOptValues(double functionValue, const LikelihoodPtrVector& Likely) const {
  int i;
  for (i = 0; i < values.Size(); i++)
    if (opt[i] == 1 || opt.Size() == 0)
      cout << values[i] << sep;

  cout << "\n\nThe likelihood components are\n";
  for (i = 0; i < Likely.Size(); i++)
    cout << Likely[i]->returnUnweightedLikelihood() << sep;

  cout << "\n\nThe overall likelihood score is " << functionValue << endl;
}

void Keeper::writeInitialInformation(const char* const filename, const LikelihoodPtrVector& Likely) {
  ofstream outfile;
  outfile.open(filename, ios::out);
  handle.checkIfFailure(outfile, filename);
  int i, j, k;

  outfile << "; ";
  RUNID.print(outfile);
  outfile << "; Listing of the switches used in the current Gadget run\n";
  for (i = 0; i < address.Nrow(); i++) {
    outfile << switches[i] << TAB;
    for (j = 0; j < address.Ncol(i); j++)
      outfile << address[i][j].name << TAB;
    outfile << endl;
  }

  CharPtrVector tmpLikely;
  tmpLikely.resize(Likely.Size());

  char* strLikely = SendComponents();
  char* strTemp = new char[MaxStrLength];
  strncpy(strTemp, "", MaxStrLength);

  j = k = 0;
  for (i = 1; i < strlen(strLikely); i++) {
    if (strLikely[i] == TAB) {
      strTemp[j] = '\0';
      tmpLikely[k] = new char[strlen(strTemp) + 1];
      strcpy(tmpLikely[k], strTemp);
      strncpy(strTemp, "", MaxStrLength);
      j = 0;
      k++;
    } else {
      strTemp[j] = strLikely[i];
      j++;
    }
  }
  //copy the last component into tmpLikely
  strTemp[j] = '\0';
  tmpLikely[k] = new char[strlen(strTemp) + 1];
  strcpy(tmpLikely[k], strTemp);

  outfile << ";\n; Listing of the likelihood components used in the current Gadget run\n;\n";
  outfile << "; Component\tType\tWeight\n";
  for (i = 0; i < tmpLikely.Size(); i++)
    outfile << tmpLikely[i] << TAB << Likely[i]->Type() << TAB << Likely[i]->returnWeight() << endl;
  outfile << ";\n; Listing of the output from the likelihood components for the current Gadget run\n;\n";
  outfile.close();
  outfile.clear();

  //tmpLikely is not required - free up memory
  for (i = 0; i < tmpLikely.Size(); i++)
    delete[] tmpLikely[i];
  delete[] strLikely;
  delete[] strTemp;
}

void Keeper::writeValues(const char* const filename,
  double functionValue, const LikelihoodPtrVector& Likely, int prec) const {

  int i, p, w;
  ofstream outfile;
  outfile.open(filename, ios::app);
  handle.checkIfFailure(outfile, filename);

  //JMB - print the number of function evaluations at the start of the line
  outfile << EcoSystem->getFuncEval() << TAB;

  p = prec;
  w = p + 4;
  if (prec == 0) {
    p = printprecision;
    w = printwidth;
  }
  for (i = 0; i < values.Size(); i++)
    outfile << setw(w) << setprecision(p) << values[i] << sep;

  if (prec == 0) {
    p = smallprecision;
    w = smallwidth;
  }
  outfile << TAB << TAB;
  for (i = 0; i < Likely.Size(); i++)
    outfile << setw(w) << setprecision(p) << Likely[i]->returnUnweightedLikelihood() << sep;

  if (prec == 0) {
    p = fullprecision;
    w = fullwidth;
  }
  outfile << TAB << TAB << setw(w) << setprecision(p) << functionValue << endl;
  outfile.close();
  outfile.clear();
}

void Keeper::writeInitialInformationInColumns(const char* const filename) const {
  ofstream outfile;
  outfile.open(filename, ios::out);
  handle.checkIfFailure(outfile, filename);

  outfile << "; ";
  RUNID.print(outfile);
  outfile.close();
  outfile.clear();
}

void Keeper::writeValuesInColumns(const char* const filename,
  double functionValue, const LikelihoodPtrVector& Likely, int prec) const {

  int i, p, w;
  ofstream outfile;
  outfile.open(filename, ios::app);
  handle.checkIfFailure(outfile, filename);

  p = prec;
  if (prec == 0)
    p = largeprecision;
  w = p + 4;

  outfile << ";\n; the optimisation has run for " << EcoSystem->getFuncEval()
    << " function evaluations\n; the current likelihood value is "
    << setprecision(p) << functionValue << "\nswitch\tvalue\t\tlower\tupper\toptimize\n";

  if (opt.Size() == 0) {
    for (i = 0; i < values.Size(); i++) {
      outfile << switches[i] << TAB << setw(w) << setprecision(p) << values[i];
      outfile << setw(smallwidth) << setprecision(smallprecision) << lowerbds[i] << TAB
        << setw(smallwidth) << setprecision(smallprecision) << upperbds[i] << "\t1\n";
   }

  } else {
    for (i = 0; i < values.Size(); i++) {
      outfile << switches[i] << TAB << setw(w) << setprecision(p) << values[i];
      outfile << setw(smallwidth) << setprecision(smallprecision) << lowerbds[i] << TAB
        << setw(smallwidth) << setprecision(smallprecision) << upperbds[i] << TAB << opt[i] << endl;
    }
  }
  outfile.close();
  outfile.clear();
}

void Keeper::Update(const StochasticData* const Stoch) {

  int i, j;
  if (Stoch->SwitchesGiven()) {
    IntVector match(Stoch->NoVariables(), 0);
    IntVector found(switches.Size(), 0);

    for (i = 0; i < Stoch->NoVariables(); i++)
      for (j = 0; j < switches.Size(); j++) {
        if (Stoch->Switches(i) == switches[j]) {
          values[j] = Stoch->Values(i);

          if (isZero(initialvalues[j])) {
            handle.LogWarning("Warning in keeper - cannot scale switch with initial value zero", switches[j].getValue());
            scaledvalues[j] = values[j];
          } else
            scaledvalues[j] = values[j] / initialvalues[j];

          lowerbds[j] = Stoch->Lower(i);
          upperbds[j] = Stoch->Upper(i);
          if (Stoch->OptGiven())
            opt[j] = Stoch->Optimize(i);

          match[i]++;
          found[j]++;
        }
      }

    for (i = 0; i < Stoch->NoVariables(); i++)
      if (match[i] == 0)
        handle.LogWarning("Warning in keeper - failed to match switch", Stoch->Switches(i).getValue());

    for (i = 0; i < switches.Size(); i++)
      if (found[i] == 0)
        handle.LogWarning("Warning in keeper - using default values for switch", switches[i].getValue());

  } else {
    if (this->NoVariables() != Stoch->NoVariables()) {
      handle.LogWarning("Error in keeper - failed to read values");
      exit(EXIT_FAILURE);
    }
    for (i = 0; i < Stoch->NoVariables(); i++) {
      if (Stoch->OptGiven())
        opt[i] = Stoch->Optimize(i);

      values[i] = Stoch->Values(i);
      if (isZero(initialvalues[i])) {
        handle.LogWarning("Warning in keeper - cannot scale switch with initial value zero", switches[i].getValue());
        scaledvalues[i] = values[i];
      } else
        scaledvalues[i] = Stoch->Values(i) / initialvalues[i];
    }
  }

  for (i = 0; i < address.Nrow(); i++)
    for (j = 0; j < address.Ncol(i); j++)
      *address[i][j].addr = values[i];
}

void Keeper::Opt(IntVector& optimize) const {
  int i;
  for (i = 0; i < optimize.Size(); i++)
    optimize[i] = opt[i];
}

void Keeper::Switches(ParameterVector& sw) const {
  int i;
  for (i = 0; i < sw.Size(); i++)
    sw[i] = switches[i];
}

void Keeper::writeParamsInColumns(const char* const filename,
  double functionValue, const LikelihoodPtrVector& Likely, int prec) const {

  int i, p, w;
  ofstream outfile;
  outfile.open(filename, ios::out);
  handle.checkIfFailure(outfile, filename);

  p = prec;
  if (prec == 0)
    p = largeprecision;
  w = p + 4;

  outfile << "; ";
  RUNID.print(outfile);

  if (EcoSystem->getFuncEval() == 0) {
    outfile << "; a stochastic run was performed giving a likelihood value of "
      << setprecision(p) << functionValue;

  } else {
    outfile << "; the optimisation ran for " << EcoSystem->getFuncEval()
      << " function evaluations\n; the final likelihood value was "
      << setprecision(p) << functionValue;

    if (EcoSystem->getConverge() == 1)
      outfile << "\n; the optimisation stopped because the convergence criteria were met";
    else
      outfile << "\n; the optimisation stopped because the maximum number of iterations was reached";
  }

  outfile << "\nswitch\tvalue\t\tlower\tupper\toptimise\n";
  for (i = 0; i < values.Size(); i++) {
    outfile << switches[i] << TAB << setw(w) << setprecision(p) << values[i] << TAB;
    outfile << setw(smallwidth) << setprecision(smallprecision) << lowerbds[i]
      << setw(smallwidth) << setprecision(smallprecision) << upperbds[i]
      << setw(smallwidth) << opt[i] << endl;
  }
  outfile.close();
  outfile.clear();
}

void Keeper::LowerBds(DoubleVector& lbs) const {
  int i;
  for (i = 0; i < lbs.Size(); i++)
    lbs[i] = lowerbds[i];
}

void Keeper::UpperBds(DoubleVector& ubs) const {
  int i;
  for (i = 0; i < ubs.Size(); i++)
    ubs[i] = upperbds[i];
}

void Keeper::LowerOptBds(DoubleVector& lbs) const {
  int i, j;
  if (lbs.Size() != this->NoOptVariables()) {
    handle.LogWarning("Warning in keeper - illegal number of optimising variables");
    return;
  }
  if (lbs.Size() == 0)
    this->LowerBds(lbs);
  else {
    j = 0;
    for (i = 0; i < lowerbds.Size(); i++)
      if (opt[i] == 1) {
        lbs[j] = lowerbds[i];
        j++;
      }
  }
}

void Keeper::UpperOptBds(DoubleVector& ubs) const {
  int i, j;
  if (ubs.Size() != this->NoOptVariables()) {
    handle.LogWarning("Warning in keeper - illegal number of optimising variables");
    return;
  }
  if (ubs.Size() == 0)
    this->UpperBds(ubs);
  else {
    j = 0;
    for (i = 0; i < upperbds.Size(); i++)
      if (opt[i] == 1) {
        ubs[j] = upperbds[i];
        j++;
      }
  }
}

void Keeper::CheckBounds() const {
  int i, count = 0;
  for (i = 0; i < values.Size(); i++) {
    if (lowerbds[i] > values[i]) {
      count++;
      handle.LogWarning("Error in keeper - failed to read lowerbound", lowerbds[i], values[i]);
    }
    if (upperbds[i] < values[i]) {
      count++;
      handle.LogWarning("Error in keeper - failed to read upperbound", values[i], upperbds[i]);
    }
    if (upperbds[i] < lowerbds[i]) {
      count++;
      handle.LogWarning("Error in keeper - failed to read bounds", lowerbds[i], upperbds[i]);
    }
  }

  if (count > 0) {
    handle.LogWarning("Error in keeper - failed to read bounds");
    exit(EXIT_FAILURE);
  }
}

void Keeper::AddString(const string str) {
  AddString(str.c_str());
}
