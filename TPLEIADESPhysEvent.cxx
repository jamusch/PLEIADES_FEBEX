//------------------------------------------------------------------------
//*********************** TPLEIADESPhysEvent.cxx *************************
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

#include "TPLEIADESPhysEvent.h"
#include "TPLEIADESPhysProc.h"
#include "TGo4Log.h"


//------------------------------------------------------------------------
// TPLEIADESDetPhysics is a dependent class on TPLEIADESPhysEvent. It represents a detector with physics-processed values.
//------------------------------------------------------------------------

// forward declaration of fParDet, which will point to fPar when used in TPLEIADESDetProc
TPLEIADESDetPhysics::TPLEIADESDetPhysics() :
    TGo4EventElement()
{
    TGo4Log::Info("TPLEIADESDetPhysics: Create instance");
}

TPLEIADESDetPhysics::TPLEIADESDetPhysics(const char* name, Short_t id) :
    TGo4EventElement(name, name, id)
{
    TGo4Log::Info("TPLEIADESDetPhysics: Create instance %s with composite ID %d", name, id);
}

TPLEIADESDetPhysics::~TPLEIADESDetPhysics()
{
    TGo4Log::Info("TPLEIADESDetPhysics: Delete instance");
}

void TPLEIADESDetPhysics::Clear(Option_t *opt)
{
    // all members should be cleared, i.e. assigned to a "not filled" value
    fpEnergy  = -99;
    fnEnergy  = -99;
    fNormPosX = -99;
    fNormPosY = -99;
}


//------------------------------------------------------------------------
// TPLEIADESPhysEvent is the base class for detector grouping
//------------------------------------------------------------------------

// forward declaration of fParDEv, which will point to fPar when used in TPLEIADESDetProc
TPLEIADESParam *TPLEIADESPhysEvent::fParPEv = 0;

TPLEIADESPhysEvent::TPLEIADESPhysEvent() :
    TGo4CompositeEvent()
{
    TGo4Log::Info("TPLEIADESPhysEvent: Create instance");
}

TPLEIADESPhysEvent::TPLEIADESPhysEvent(const char* name, Short_t id) :
    TGo4CompositeEvent(name, name, id)
{
    TGo4Log::Info("TPLEIADESPhysEvent: Create instance %s with composite ID %d", name, id);

    BuildDetectors();
}

TPLEIADESPhysEvent::~TPLEIADESPhysEvent()
{
    TGo4Log::Info("TPLEIADESPhysEvent: Delete instance");
}

void TPLEIADESPhysEvent::BuildDetectors()      //construct detectors based on fDetNameVec list
{
    if(fParPEv == 0)
    {
        TGo4Log::Warn("TPLEIADESPhysEvent::BuildDetectors: fPar not set! Need parameter to access detector setup to build detectors.");
        return;
    }

    int index = 0;
    for(const TString& dname : fParPEv->fDetNameVec)
    {
        TPLEIADESDetPhysics *theDetector = new TPLEIADESDetPhysics(dname, index);
        theDetector->SetDetName(dname);
        theDetector->SetDetType(fParPEv->fDetTypeMap[dname]);
        addEventElement(theDetector);
        index++;
        TGo4Log::Info("TPLEIADESDetPhysics builds detector %s", dname.Data());
    }
}

TPLEIADESDetPhysics* TPLEIADESPhysEvent::GetDetector(TString dname)
{
    TPLEIADESDetPhysics* theDetector = 0;
    Short_t numDets = getNElements();
    for(int i=0; i<numDets; ++i)
    {
        theDetector = (TPLEIADESDetPhysics*) getEventElement(i);
        if(theDetector->GetDetName() == dname)
        {
            return theDetector;
        }
    }
    return 0;
}

void TPLEIADESPhysEvent::Clear(Option_t *opt)
{
    TGo4CompositeEvent::Clear();
}

//----------------------------END OF GO4 SOURCE FILE ---------------------
