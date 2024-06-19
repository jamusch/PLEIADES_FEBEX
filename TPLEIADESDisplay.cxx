//------------------------------------------------------------------------
//************************ TPLEIADESDisplay.cxx **************************
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

#include "TPLEIADESDisplay.h"

#include "TGo4Log.h"

//------------------------------------------------------------------------
// TPLEIADESDisplay is
//------------------------------------------------------------------------

TPLEIADESDisplay::TPLEIADESDisplay() :
    TGo4EventProcessor()
{
    TGo4Log::Info("TPLEIADESDisplay: Create instance");
}

TPLEIADESDisplay::~TPLEIADESDisplay()
{
    TGo4Log::Info("TPLEIADESDisplay: Delete instance");
}

//------------------------------------------------------------------------
// TPLEIADESDetDisplay is
//------------------------------------------------------------------------

TPLEIADESDetDisplay::TPLEIADESDetDisplay() :
    TPLEIADESDisplay()
{
    TGo4Log::Info("TPLEIADESDetDisplay: Create instance");
}

TPLEIADESDetDisplay::TPLEIADESDetDisplay(TPLEIADESDetector* theDetector) :
    TPLEIADESDisplay()
{
    fDetector = theDetector;  // link display to detector
}

TPLEIADESDetDisplay::~TPLEIADESDetDisplay()
{
    TGo4Log::Info("TPLEIADESDetDisplay: Delete instance");
}

void TPLEIADESDetDisplay::AddChanDisplay(TPLEIADESChanDisplay* chanDisplay)
{
    fChannels.push_back(chanDisplay);
}

void TPLEIADESDetDisplay::ClearChanDisplay()
{
    fChannels.clear();
}

Int_t TPLEIADESDetDisplay::GetNumChannels()
{
    return fChannels.size();
}

TPLEIADESChanDisplay* TPLEIADESDetDisplay::GetChanDisplay(TString name)
{
    for(TPLEIADESChanDisplay* chanDisplay : fChannels)
    {
        if(chanDisplay->GetChanName() == name)
        {
            return chanDisplay;
        }
    }
    return 0;
}

void TPLEIADESDetDisplay::InitDisplay()
{
    TString modname, modhead, detname;

    // make channel hit pattern histogram
    detname = fDetector->GetName();
    modname.Form("TPLEIADESDetProc/%s/Hit Pattern", detname.Data());
    modhead.Form("%s Hit Pattern", detname.Data());
    if(fDetector->GetDetType() == "SiPad")
    {
        hDetHitPattern = MakeTH1('D', modname, modhead, 8, 0, 8);
    }
    else if(fDetector->GetDetType() == "DSSD")
    {
        hDetHitPattern = MakeTH1('D', modname, modhead, 4, 0, 4);
    }
    else if(fDetector->GetDetType() == "Crystal")
    {
        hDetHitPattern = MakeTH1('D', modname, modhead, 2, 0, 2);
    }
    else
    {
        TGo4Log::Warn("TPLEIADESChanDisplay::InitChanDisplay - Detector %s does not have a recognised detector type, histos can't be set up.", detname.Data());
        return;
    }

}

//------------------------------------------------------------------------
// TPLEIADESChanDisplay is
//------------------------------------------------------------------------

TPLEIADESChanDisplay::TPLEIADESChanDisplay() :
    TPLEIADESDisplay()
{
    TGo4Log::Info("TPLEIADESChanDisplay: Create instance");
}

TPLEIADESChanDisplay::TPLEIADESChanDisplay(TPLEIADESDetChan* theChannel) :
    TPLEIADESDisplay()
{
    fChannel = theChannel;
}

TPLEIADESChanDisplay::~TPLEIADESChanDisplay()
{
    TGo4Log::Info("TPLEIADESChanDisplay: Delete instance");
}

TString TPLEIADESChanDisplay::GetChanName()
{
    TString tstr(fChannel->GetName());
    return tstr;
}

void TPLEIADESChanDisplay::InitDisplay()
{

}

//----------------------------END OF GO4 SOURCE FILE ---------------------
