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
#include "TPLEIADESDetProc.h"

#include "TGo4Log.h"

#include "TH1.h"
#include "TH2.h"

#include <iostream>

//------------------------------------------------------------------------
// TPLEIADESDisplay is
//------------------------------------------------------------------------
// forward declaration of fParChDisp, which will point to fPar when used in TPLEIADESDetProc
TPLEIADESParam *TPLEIADESDisplay::fParDisplay = 0;

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
    TGo4Log::Info("TPLEIADESDetDisplay: Create instance for detector %s", theDetector->GetDetName().Data());
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
    SetMakeWithAutosave(kFALSE);    // recreate histograms

    TString modname, modhead, detname;

    // make channel hit pattern histogram
    detname = fDetector->GetDetName();

    std::cout << "This detector name is" << fDetector->GetDetName().Data() << std::endl;

    if(fDetector->GetDetType() == "SiPad")
    {
        std::cout << "This det is a SiPad!" << std::endl;
        modname.Form("TPLEIADESDetProc/%s/%s Hit Pattern", detname.Data(), detname.Data());
        modhead.Form("%s Hit Pattern", detname.Data());
        hDetHitPattern = MakeTH1('I', modname, modhead, 9, -1.5, 7.5);
        hDetHitPattern->GetXaxis()->SetBinLabel(1, "phys event trig");
        for(short i=0; i<7; ++i) { modname.Form("p-strip %d", i); hDetHitPattern->GetXaxis()->SetBinLabel(i+2, modname.Data()); }
        hDetHitPattern->GetXaxis()->SetBinLabel(9, "n-side");

        modname.Form("TPLEIADESDetProc/%s/%s Energy Pattern", detname.Data(), detname.Data());
        modhead.Form("%s Energy Pattern", detname.Data());
        hDetEnergyPattern = MakeTH2('D', modname, modhead, 8, -0.5, 7.5, 4e3, -2e5, 2e5);
        for(short i=0; i<7; ++i) { modname.Form("p-strip %d", i); hDetEnergyPattern->GetXaxis()->SetBinLabel(i+1, modname.Data()); }
        hDetEnergyPattern->GetXaxis()->SetBinLabel(8, "n-side");
    }
    else if(fDetector->GetDetType() == "DSSD")
    {
        modname.Form("TPLEIADESDetProc/%s/%s Hit Pattern", detname.Data(), detname.Data());
        modhead.Form("%s Hit Pattern", detname.Data());
        hDetHitPattern = MakeTH1('D', modname, modhead, 5, -1.5, 3.5);
    }
    else if(fDetector->GetDetType() == "Crystal")
    {
        modname.Form("TPLEIADESDetProc/%s/%s Hit Pattern", detname.Data(), detname.Data());
        modhead.Form("%s Hit Pattern", detname.Data());
        hDetHitPattern = MakeTH1('D', modname, modhead, 3, -1.5, 1.5);
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
    TGo4Log::Info("TPLEIADESDetDisplay: Create instance for detector %s", theChannel->GetName());
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
    SetMakeWithAutosave(kFALSE);    // recreate histograms

    TString modname, modhead, detname, chname;

    // make trace histograms
    detname = fChannel->GetDetName();
    chname = fChannel->GetName();

    if(fParDisplay == 0)
    {
        TGo4Log::Warn("TPLEIADESChanDisplay::InitDisplay: fPar not set! Need parameter to build histograms.");
        return;
    }

    modname.Form("TPLEIADESDetProc/%s/Channel Hit Multiplicity/%s Hit Multiplicity", detname.Data(), chname.Data());
    modhead.Form("%s Hit Multiplicity", detname.Data());
    hHitMultiplicity = MakeTH1('I', modname, modhead, 6, -0.5, 5.5);

    #ifdef TPLEIADES_FILL_TRACES
    UInt_t lTraceSize = fParDisplay->fTraceSize;
    modname.Form("TPLEIADESDetProc/%s/Channel Traces/%s Trace", detname.Data(), chname.Data());
    modhead.Form("%s Trace", detname.Data());
    hTraceChan = MakeTH1('D', modname, modhead, lTraceSize, 0, lTraceSize);

    modname.Form("TPLEIADESDetProc/%s/Channel Trace BLR/%s Trace BLR", detname.Data(), chname.Data());
    modhead.Form("%s Trace BLR", detname.Data());
    hTraceBLRChan = MakeTH1('D', modname, modhead, lTraceSize, 0, lTraceSize);

    modname.Form("TPLEIADESDetProc/%s/Channel Trace TRAPEZ/%s Trace TRAPEZ", detname.Data(), chname.Data());
    modhead.Form("%s Trace TRAPEZ", detname.Data());
    hTraceTRAPEZChan = MakeTH1('D', modname, modhead, lTraceSize, 0, lTraceSize);
    #endif // TPLEIADES_FILL_TRACES
}

void TPLEIADESChanDisplay::FillTraces()
{
    if(static_cast<Int_t>(fParDisplay->fTraceSize) != hTraceChan->GetNbinsX())
    {
        TGo4Log::Error("TPLEIADESChanDisplay::FillTraces - trace size from fPar does not match hTrace bin length. Histogram misconfigured!");
    }

    for(uint i=0; i<fChannel->fDTrace.size(); ++i)
    {
        hTraceChan->SetBinContent(i, fChannel->fDTrace[i]);
        hTraceBLRChan->SetBinContent(i, fChannel->fDTraceBLR[i]);
        hTraceTRAPEZChan->SetBinContent(i, fChannel->fDTraceTRAPEZ[i]);
    }
}

//----------------------------END OF GO4 SOURCE FILE ---------------------
