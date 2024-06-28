//------------------------------------------------------------------------
//*********************** TPLEIADESDetEvent.cxx **************************
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

#include "TPLEIADESDetEvent.h"
#include "TPLEIADESDetProc.h"
#include "TGo4Log.h"


//------------------------------------------------------------------------
// TPLEIADESDetChan is a dependent class on TPLEIADESDetector. It represents an output channel on the detector.
//------------------------------------------------------------------------

TPLEIADESDetChan::TPLEIADESDetChan() :
    TGo4EventElement()
{
    TGo4Log::Info("TPLEIADESDetChan: Create instance");
}

TPLEIADESDetChan::TPLEIADESDetChan(const char* name, Short_t id) :
    TGo4EventElement(name, name, id)
{
    TGo4Log::Info("TPLEIADESDetChan: Create instance %s with composite ID %d", name, id);
}

TPLEIADESDetChan::~TPLEIADESDetChan()
{
    TGo4Log::Info("TPLEIADESDetChan: Delete instance");
}

void TPLEIADESDetChan::Clear(Option_t *opt)
{
    // all members should be cleared, i.e. assigned to a "not filled" value
    /** FEBEX special channel properties **/
    fDHitMultiplicity = 0;
    fDFPGAEnergy = 0;
    fDFPGAHitTime = 0;
    fDFPGATRAPEZ.clear();

    /** FEBEX trace properties **/
    #ifdef TPLEIADES_FILL_TRACES
    fDTrapezEnergy = 0;
    fDTrace.clear();
    fDTraceBLR.clear();
    fDTraceTRAPEZ.clear();
    #endif
}

//------------------------------------------------------------------------
// TPLEIADESDector is a dependent class on TPLEIADESDetEvent. It represents a physical detector.
//------------------------------------------------------------------------

// forward declaration of fParDet, which will point to fPar when used in TPLEIADESDetProc
TPLEIADESParam *TPLEIADESDetector::fParDet = 0;

TPLEIADESDetector::TPLEIADESDetector() :
    TGo4CompositeEvent()
{
    TGo4Log::Info("TPLEIADESDetector: Create instance");
}

TPLEIADESDetector::TPLEIADESDetector(const char* name, Short_t id) :
    TGo4CompositeEvent(name, name, id)
{
    TGo4Log::Info("TPLEIADESDetector: Create instance %s with composite ID %d", name, id);
}

TPLEIADESDetector::~TPLEIADESDetector()
{
    TGo4Log::Info("TPLEIADESDetector: Delete instance");
}

TPLEIADESDetChan* TPLEIADESDetector::GetChannel(TString chname)
{
    TPLEIADESDetChan* theChannel = 0;
    Short_t numChans = getNElements();
    for(int i=0; i<numChans; ++i)
    {
        theChannel = (TPLEIADESDetChan*) getEventElement(i);
        if(theChannel->GetName() == chname)
        {
            return theChannel;
        }
    }
    return 0;
}

TPLEIADESDetChan* TPLEIADESDetector::GetChannel(Short_t id)
{
    if(id<0 || id>=getNElements()) { TGo4Log::Error("TPLEIADESDetector::GetChannel - id outside of NElements."); return 0; }
    return (TPLEIADESDetChan*) getEventElement(id);
}

void TPLEIADESDetector::SetupDetector()    // builds detector channels based on type of detector
{
    if(fParDet == 0)
    {
        TGo4Log::Warn("TPLEIADESDetEvent::BuildDetectors: fPar not set! Need parameter to access detector setup to build detectors.");
        return;
    }

    TString modname;

    if(fDetType == "SiPad")         // create Si Pad channels
    {
        // setup p-sides
        for (int j=0; j<7; ++j)
        {
            modname.Form("%s_pStrip%d", fDetName.Data(), j);
            TPLEIADESDetChan *pStrip = new TPLEIADESDetChan(modname.Data(), j);
            pStrip->SetDetName(fDetName.Data());
            pStrip->SetDetId(getId());
            pStrip->SetDetType(fDetType.Data());
            pStrip->SetChanMap(fParDet->fpSideMap[fDetName] + j);
            pStrip->SetChanType("pStrip");
            addEventElement(pStrip);
        }
        // setup n-side
        modname.Form("%s_nSide", fDetName.Data());
        TPLEIADESDetChan *nSide = new TPLEIADESDetChan(modname.Data(), 7);
        nSide->SetDetName(fDetName.Data());
        nSide->SetDetId(getId());
        nSide->SetDetType(fDetType.Data());
        nSide->SetChanMap(fParDet->fnSideMap[fDetName]);
        nSide->SetChanType("nSide");
        addEventElement(nSide);
    }
    else if(fDetType == "DSSD")     // setup the DSSD
    {
        // setup raw DSSD channels
        TString dssdNames[4] = {"FrntLft", "FrntRgt", "BackTop", "BackBot"};
        for(int j=0; j<4; ++j)
        {
            modname.Form("%s_%s", fDetName.Data(), dssdNames[j].Data());
            TPLEIADESDetChan *dssdChan = new TPLEIADESDetChan(modname.Data(), j);
            dssdChan->SetDetName(fDetName.Data());
            dssdChan->SetDetId(getId());
            dssdChan->SetDetType(fDetType.Data());
            dssdChan->SetChanMap(fParDet->fDSSDMap[j]);
            dssdChan->SetChanType("dssdChan");
            addEventElement(dssdChan);
        }
    }
    else if(fDetType == "Crystal")  // setup the Crystal
    {
        // setup Crystal photodiode outputs
        TString crysNames[2] = {"CrysFrnt", "CrysBack"};
        for(int j=0; j<2; ++j)
        {
            modname.Form("%s_%s", fDetName.Data(), crysNames[j].Data());
            TPLEIADESDetChan *crysChan = new TPLEIADESDetChan(modname.Data(), j);
            crysChan->SetDetName(fDetName.Data());
            crysChan->SetDetId(getId());
            crysChan->SetDetType(fDetType.Data());
            crysChan->SetChanMap(fParDet->fCrystalMap[j]);
            crysChan->SetChanType("crysChan");
            addEventElement(crysChan);
        }
    }
    else
    {
        TGo4Log::Warn("Detector %s does not have a recognised detector type, and thus can't be set up.", fDetName.Data());
        return;
    }
}

void TPLEIADESDetector::Clear(Option_t *opt)
{
    TGo4CompositeEvent::Clear();
}


//------------------------------------------------------------------------
// TPLEIADESDetEvent is the base class for detector grouping
//------------------------------------------------------------------------

// forward declaration of fParDEv, which will point to fPar when used in TPLEIADESDetProc
TPLEIADESParam *TPLEIADESDetEvent::fParDEv = 0;

TPLEIADESDetEvent::TPLEIADESDetEvent() :
    TGo4CompositeEvent(), fSequenceNumber(0)
{
    TGo4Log::Info("TPLEIADESDetEvent: Create instance");
}

TPLEIADESDetEvent::TPLEIADESDetEvent(const char* name, Short_t id) :
    TGo4CompositeEvent(name, name, id), fSequenceNumber(0)
{
    TGo4Log::Info("TPLEIADESDetEvent: Create instance %s with composite ID %d", name, id);

    BuildDetectors();
}

TPLEIADESDetEvent::~TPLEIADESDetEvent()
{
    TGo4Log::Info("TPLEIADESDetEvent: Delete instance");
}

void TPLEIADESDetEvent::BuildDetectors()      //construct detectors based on fDetNameVec list
{
    if(fParDEv == 0)
    {
        TGo4Log::Warn("TPLEIADESDetEvent::BuildDetectors: fPar not set! Need parameter to access detector setup to build detectors.");
        return;
    }

    int index = 0;
    for(const TString& dname : fParDEv->fDetNameVec)
    {
        TPLEIADESDetector *theDetector = new TPLEIADESDetector(dname, index);
        theDetector->SetDetName(dname);
        theDetector->SetDetType(fParDEv->fDetTypeMap[dname]);
        theDetector->SetupDetector();
        addEventElement(theDetector);
        index++;
        TGo4Log::Info("TPLEIADESDetector builds detector %s", dname.Data());
    }
}

TPLEIADESDetector* TPLEIADESDetEvent::GetDetector(TString dname)
{
    TPLEIADESDetector* theDetector = 0;
    Short_t numDets = getNElements();
    for(int i=0; i<numDets; ++i)
    {
        theDetector = (TPLEIADESDetector*) getEventElement(i);
        if(theDetector->GetDetName() == dname)
        {
            return theDetector;
        }
    }
    return 0;
}

void TPLEIADESDetEvent::Clear(Option_t *opt)
{
    TGo4CompositeEvent::Clear();
}

//----------------------------END OF GO4 SOURCE FILE ---------------------
