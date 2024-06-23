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
// TPLEIADESDisplay is the base display class
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
// TPLEIADESDetDisplay is the container holding detector wide histograms
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
// TPLEIADESChanDisplay is the container holding channel specific histograms
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

//------------------------------------------------------------------------
// TPLEIADESPhysDisplay is the container for holding Physics Processing histograms
//------------------------------------------------------------------------

TPLEIADESPhysDisplay::TPLEIADESPhysDisplay() :
    TPLEIADESDisplay()
{
    TGo4Log::Info("TPLEIADESPhysDisplay: Create instance");
}

TPLEIADESPhysDisplay::~TPLEIADESPhysDisplay()
{
    TGo4Log::Info("TPLEIADESPhysDisplay: Delete instance");
}

void TPLEIADESPhysDisplay::InitDisplay(TPLEIADESDetEvent* fInEvent)
{
    SetMakeWithAutosave(kFALSE);    // recreate histograms

    if(fParDisplay == 0)
    {
        TGo4Log::Warn("TPLEIADESChanDisplay::InitDisplay: fPar not set! Need parameter to build histograms.");
        return;
    }

    TString modname, modhead;

    for(const TString& dname : fParDisplay->fDetNameVec)
    {
        TPLEIADESDetector *theDetector = fInEvent->GetDetector(dname);
        if(theDetector->GetDetType() == "SiPad")
        {
            modname.Form("TPLEIADESPhysProc/Clipping Statistics/Rise Time to Clipping/%s n side RTtC", dname.Data());
            modhead.Form("Rise Time to Clipping - %s n side", dname.Data());
            hRiseTimeNSides.push_back(MakeTH1('I', modname, modhead, 300, 0, 300));

            modname.Form("TPLEIADESPhysProc/Clipping Statistics/Rise Time to Reentry/%s n side RTtR", dname.Data());
            modhead.Form("Rise Time to Reentry - %s n side", dname.Data());
            hReentryTimeNSides.push_back(MakeTH1('I', modname, modhead, 1000, 0, 1000));

            modname.Form("TPLEIADESPhysProc/Clipping Statistics/Pulse Length/%s n side PL", dname.Data());
            modhead.Form("Pulse Length - %s n side", dname.Data());
            hPulseTimeNSides.push_back(MakeTH1('I', modname, modhead, 1e3, 2e3, 3e3));

            modname.Form("TPLEIADESPhysProc/Clipping Statistics/Clip Height/%s n side CH", dname.Data());
            modhead.Form("Clip Height - %s n side", dname.Data());
            hClipHeightNSides.push_back(MakeTH1('I', modname, modhead, 2e3, 1e3, 3e3));

            modname.Form("TPLEIADESPhysProc/Clipping Statistics/End Height/%s n side EH", dname.Data());
            modhead.Form("End Height - %s n side", dname.Data());
            hEndHeightNSides.push_back(MakeTH1('I', modname, modhead, 1e3, 0, 1e3));
        }
    }

    modname.Form("TPLEIADESPhysProc/Clipping Statistics/Rise Time to Clipping/Crystal Front RTtC");
    modhead.Form("Rise Time to Clipping - Crys Front");
    hRiseTimeCrysFr = MakeTH1('I', modname, modhead, 300, 0, 300);
    modname.Form("TPLEIADESPhysProc/Clipping Statistics/Rise Time to Clipping/Crystal Back RTtC");
    modhead.Form("Rise Time to Clipping - Crys Back");
    hRiseTimeCrysBk = MakeTH1('I', modname, modhead, 300, 0, 300);

    modname.Form("TPLEIADESPhysProc/Clipping Statistics/Rise Time to Reentry/Crystal Front RTtR");
    modhead.Form("Rise Time to Reentry - Crys Front");
    hReentryTimeCrysFr = MakeTH1('I', modname, modhead, 1000, 0, 1000);
    modname.Form("TPLEIADESPhysProc/Clipping Statistics/Rise Time to Reentry/Crystal Back RTtR");
    modhead.Form("Rise Time to Reentry - Crys Back");
    hReentryTimeCrysBk = MakeTH1('I', modname, modhead, 1000, 0, 1000);

    modname.Form("TPLEIADESPhysProc/Clipping Statistics/Pulse Length/Crystal Front PL");
    modhead.Form("Pulse Length - Crys Front");
    hPulseTimeCrysFr = MakeTH1('I', modname, modhead, 1e3, 2e3, 3e3);
    modname.Form("TPLEIADESPhysProc/Clipping Statistics/Pulse Length/Crystal Back PL");
    modhead.Form("Pulse Length - Crys Back");
    hPulseTimeCrysBk = MakeTH1('I', modname, modhead, 1e3, 2e3, 3e3);

    modname.Form("TPLEIADESPhysProc/Clipping Statistics/Clip Height/Crystal Front CH");
    modhead.Form("Clip Height - Crys Front");
    hClipHeightCrysFr = MakeTH1('I', modname, modhead, 2e3, 1e3, 3e3);
    modname.Form("TPLEIADESPhysProc/Clipping Statistics/Clip Height/Crystal Back CH");
    modhead.Form("Clip Height - Crys Back");
    hClipHeightCrysBk = MakeTH1('I', modname, modhead, 2e3, 1e3, 3e3);

    modname.Form("TPLEIADESPhysProc/Clipping Statistics/End Height/Crystal Front EH");
    modhead.Form("End Height - Crys Front");
    hEndHeightCrysFr = MakeTH1('I', modname, modhead, 1e3, 0, 1e3);
    modname.Form("TPLEIADESPhysProc/Clipping Statistics/End Height/Crystal Back EH");
    modhead.Form("End Height - Crys Back");
    hEndHeightCrysBk = MakeTH1('I', modname, modhead, 1e3, 0, 1e3);
}

//----------------------------END OF GO4 SOURCE FILE ---------------------
