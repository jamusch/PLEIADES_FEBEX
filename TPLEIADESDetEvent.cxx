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
    fFPGAEnergy = 0;
    fFGPAHitTime = 0;
    fFPGATRAPEZ.clear();

    /** FEBEX trace properties **/
    #ifdef TPLEIADES_FILL_TRACES
    fTrapezEnergy = 0;
    fTrace.clear();
    fTraceBLR.clear();
    fTraceTRAPEZ.clear();
    #endif
}


//------------------------------------------------------------------------
// TPLEIADESNormPos is a dependent class on TPLEIADESDetector. It is a special class for the normalised position measurement of the DSSD.
//------------------------------------------------------------------------

TPLEIADESNormPos::TPLEIADESNormPos() :
    TGo4EventElement()
{
    TGo4Log::Info("TPLEIADESNormPos: Create instance");
}

TPLEIADESNormPos::TPLEIADESNormPos(const char* name, Short_t id) :
    TGo4EventElement(name, name, id)
{
    TGo4Log::Info("TPLEIADESNormPos: Create instance %s with composite ID %d", name, id);
}

TPLEIADESNormPos::~TPLEIADESNormPos()
{
    TGo4Log::Info("TPLEIADESNormPos: Delete instance");
}

void TPLEIADESNormPos::Clear(Option_t *opt)
{

}

//------------------------------------------------------------------------
// TPLEIADESDector is a dependent class on TPLEIADESDetEvent. It represents a physical detector.
//------------------------------------------------------------------------

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

void TPLEIADESDetector::SetupDetector()    // builds detector channels based on type of detector
{
    TString modname;

    if(fDetType == "SiPad")         // create Si Pad channels
    {
        // setup p-sides
        for (int j=0; j<7; ++j)
        {
            modname.Form("%s_pStrip%02d", fDetName, j);
            TPLEIADESDetChan *pStrip = TPLEIADESDetChan(modname.Data(), j);
            pStrip->SetChanMap(fPar->fpSideMap[fDetName] + j);
            pStrip->SetChanType("pStrip");
            addEventElement(pStrip);
        }
        // setup n-side
        modname.Form("%s_nSide", fDetName);
        TPLEIADESDetChan *nSide = TPLEIADESDetChan(modname.Data(), 7);
        nSide->SetChanMap(fPar->fnSideMap[fDetName]);
        nSide->SetChanType("nSide");
        addEventElement(nSide);
    }
    else if(fDetType == "DSSD")     // setup the DSSD
    {
        // setup raw DSSD channels
        TString *dssdNames[4] = {"FrntLft", "FrntRgt", "BackTop", "BackBot"};
        for(int j=0; j<4; ++j)
        {
            modname.Form("%s_%s", fDetName, dssdNames[j]);
            TPLEIADESDetChan *dssdChan = TPLEIADESDetChan(modname.Data(), j);
            dssdChan->SetChanMap(fPar->fDSSDPos[j]);
            dssdChan->SetChanType("dssdChan");
            addEventElement(dssdChan)
        }
        // setup secondary channels for Normalised Position calculation
        TString *posNames[2] = {"NormPosX", "NormPosY"};
        for(int j=0; j<2; ++j)
        {
            modname.Form("%s_%s", fDetName, posNames[j]);
            TPLEIADESNormPos *posChan = TPLEIADESNormPos(modname.Data(), j+4);
            addEventElement(posChan)
        }

    }
    else if(fDetType == "Crystal")  // setup the Crystal
    {
        // setup Crystal photodiode outputs
        TString *crysNames[2] = {"CrysFrnt", "CrysBack"}
        for(int j=0; j<2; ++j)
        {
            modname.Form("%s_%s", fDetName, crysNames[j]);
            TPLEIADESDetChan *crysChan = TPLEIADESDetChan(modname.Data(), j);
            crysChan->SetChanMap(fPar->fCrystalPos[j]);
            crysChan->SetChanType("crysChan");
            addEventElement(crysChan);
        }
    }
    else
    {
        TGo4Log::Warn("Detector %s does not have a recognised detector type, and thus can't be set up.", dname)
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

TPLEIADESDetEvent::TPLEIADESDetEvent() :
    TGo4CompositeEvent()
{
    TGo4Log::Info("TPLEIADESDetEvent: Create instance");
}

TPLEIADESDetEvent::TPLEIADESDetEvent(const char* name, Short_t id) :
    TGo4CompositeEvent(name, name, id)
{
    TGo4Log::Info("TPLEIADESDetEvent: Create instance %s with composite ID %d", name, id);

    BuildDetectors();
    SetupDetectors();
}

TPLEIADESDetEvent::~TPLEIADESDetEvent()
{
    TGo4Log::Info("TPLEIADESDetEvent: Delete instance");
}

void TPLEIADESDetEvent::BuildDetectors()      //construct detectors based on fDetNameVec list
{
    if(!fPar)
    {
        TGo4Log::Warn("fPar not set! Need parameter to access detector setup.")
        return;
    }

    int index = 0;
    for(const TString& dname : fPar->fDetNameVec)
    {
        TPLEIADESDetector* theDetector = TPLEIADESDetector(dname, index);
        theDetector->SetDetName(dname);
        theDetector->SetDetType(fPar->fDetTypeMap[dname]);
        theDetector->SetupDetector();
        addEventElement(theDetector);
        index++
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
