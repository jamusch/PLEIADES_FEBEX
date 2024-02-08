//-------------------------------------------------------------
//******************** TPLEIADESParam.cxx *********************
//-------------------------------------------------------------
//        Go4 Release Package v3.03-05 (build 30305)
//                      05-June-2008
//---------------------------------------------------------------
//   The GSI Online Offline Object Oriented (Go4) Project
//   Experiment Data Processing at EE department, GSI
//---------------------------------------------------------------
//
//Copyright (C) 2000- Gesellschaft f. Schwerionenforschung, GSI
//                    Planckstr. 1, 64291 Darmstadt, Germany
//Contact:            http://go4.gsi.de
//----------------------------------------------------------------
//This software can be used under the license agreements as stated
//in Go4License.txt file which is part of the distribution.
//----------------------------------------------------------------
#include "TPLEIADESParam.h"

#include "Riostream.h"

using namespace std;

//***********************************************************
TPLEIADESParam::TPLEIADESParam() : TGo4Parameter("Parameter")
{
  fSlowMotion=kFALSE;
}
//***********************************************************
TPLEIADESParam::TPLEIADESParam(const char* name) : TGo4Parameter(name)
{
  fSlowMotion=kFALSE;
}
//***********************************************************
TPLEIADESParam::~TPLEIADESParam()
{
}
//***********************************************************


//-----------------------------------------------------------
Bool_t TPLEIADESParam::UpdateFrom(TGo4Parameter *pp){
  if(pp->InheritsFrom("TPLEIADESParam"))
  {
    TPLEIADESParam * from;
    from = (TPLEIADESParam *) pp;
    cout << "**** TPLEIADESParam " << GetName() << " updated from auto save file" << endl;
    fSlowMotion=from->fSlowMotion;
    // usually, elemental members up to 1d arrays are copied automatically correctly
    // this function is intended to pass more complex data structures, or to exectue code
    // whenever user applies the parameter from gui
  }
  else // NEVER COME HERE
     cout << "Wrong parameter object: " << pp->ClassName() << endl;
  return kTRUE;
}

//----------------------------END OF GO4 SOURCE FILE ---------------------
