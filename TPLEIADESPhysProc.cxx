//------------------------------------------------------------------------
//*********************** TPLEIADESPhysProc.cxx **************************
//------------------------------------------------------------------------
//       The GSI Online Offline Object Oriented (Go4) Project
//         Experiment Data Processing at EE department, GSI
//------------------------------------------------------------------------
// Copyright (C) 2000- GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
//                     Planckstr. 1, 64291 Darmstadt, Germany
// Contact:            http://go4.gsi.de
//------------------------------------------------------------------------
// This software can be used under the license agreements as stated
// in Go4License.txt file which is part of the distribution.
//------------------------------------------------------------------------

#include "TGo4Analysis.h"
#include "TGo4UserException.h"
#include "TGo4Log.h"

#include "TPLEIADESPhysProc.h"
#include "TPLEIADESDetEvent.h"
#include "TPLEIADESParam.h"

//------------------------------------------------------------------------
TPLEIADESPhysProc::TPLEIADESPhysProc() : TGo4EventProcessor("Proc")
{
    TGo4Log::Info("TPLEIADESPhysProc: Create instance ");
}

//------------------------------------------------------------------------
// this one is used in standard factory
TPLEIADESPhysProc::TPLEIADESPhysProc(const char* name) : TGo4EventProcessor(name)
{
    TGo4Log::Info("TPLEIADESPhysProc: Create instance %s", name);
    // no need to set param macro as this was already done in first step
    fPar = dynamic_cast<TPLEIADESParam*>(MakeParameter("PLEIADESParam", "TPLEIADESParam"));
    if(fPar) { fPar->SetConfigPhysEvent(); }
}

//------------------------------------------------------------------------
TPLEIADESPhysProc::~TPLEIADESPhysProc()
{
    TGo4Log::Info("TPLEIADESPhysProc: Delete instance ");
}

//------------------------------------------------------------------------
// Build event is the unpacker function, it reads the MBS event and does stuff with it.
// Histograms are erased with the next word, so are mostly used for online analysis.
Bool_t TPLEIADESPhysProc::BuildEvent(TGo4EventElement* target)
{
    Bool_t isValid = kFALSE;    // validity of output event

    TPLEIADESDetEvent *detEvent = (TPLEIADESDetEvent*) GetInputEvent();
    if(!detEvent || !detEvent->IsValid())
    {
        TGo4Log::Error("TPLEIADESPhysProc: no input event!");
        return isValid;
    }

    fOutEvent = (TPLEIADESPhysEvent*) target;
    if(fOutEvent == 0)
    {
        GO4_STOP_ANALYSIS_MESSAGE("NEVER COME HERE: output event is not configured, wrong class!");
    }

    fOutEvent->SetValid(isValid);   // initialize next output as not filled, i.e. it is only stored when something is in

    isValid = kTRUE;                // input/output events look good

    // loop over detectors to fill data from raw output using mapping
    std::cout << "Analysis loop goes here!" << std::endl;

    fOutEvent->SetValid(isValid);     // now event is filled, store event

    return isValid;
}

//----------------------------END OF GO4 SOURCE FILE ---------------------
