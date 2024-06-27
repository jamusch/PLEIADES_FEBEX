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
#include "TGo4Picture.h"

#include "TPLEIADESPhysProc.h"
#include "TPLEIADESDetEvent.h"
#include "TPLEIADESParam.h"

#include "TH1.h"
#include "TH2.h"
#include "TF1.h"
#include "TMath.h"

#include <vector>
#include <numeric>

//------------------------------------------------------------------------
TPLEIADESPhysProc::TPLEIADESPhysProc() : TGo4EventProcessor("Proc")
{
    TGo4Log::Info("TPLEIADESPhysProc: Create instance ");
}

//------------------------------------------------------------------------
// this one is used in standard factory
TPLEIADESPhysProc::TPLEIADESPhysProc(const char* name) :
    TGo4EventProcessor(name), fOutEvent(0)
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

    fInEvent = (TPLEIADESDetEvent*) GetInputEvent();
    if(!fInEvent || !fInEvent->IsValid())
    {
        TGo4Log::Error("TPLEIADESPhysProc: no input event!");
        return isValid;
    }

    if(fOutEvent == 0)
    {
        fOutEvent = (TPLEIADESPhysEvent*) target;

        fPhysDisplay = new TPLEIADESPhysDisplay();
        fPhysDisplay->InitClipStatsHists(fInEvent);
        fPhysDisplay->InitPHReconHists(fInEvent);
    }

    fOutEvent->fSequenceNumber = fInEvent->fSequenceNumber;
    fOutEvent->fPhysTrigger = fInEvent->fPhysTrigger;

    fOutEvent->SetValid(isValid);   // initialize next output as not filled, i.e. it is only stored when something is in

    isValid = kTRUE;                // input/output events look good

    //------------------------------------------------------------------------
    // Process all detectors to calculate final energy values.
    try
    {
        // loop over detectors to fill physics properties
        for(const TString& dname : fPar->fDetNameVec)
        {
            TPLEIADESDetector *theDetector = fInEvent->GetDetector(dname);
            TPLEIADESDetPhysics *detPhysics = fOutEvent->GetDetector(dname);

            if((theDetector->GetDetType()) == "SiPad")  // select SiPads for pStrip search
            {
                if((theDetector->getNElements()) != 8)
                {
                    TGo4Log::Error("Detector %s is a Si Pad but does not have 8 elements. Detector is setup incorrectly.", dname.Data());
                    return kFALSE;
                }

                // search for active p-strip for p-side energy
                pStripSelect("FPGA", theDetector, detPhysics);

                //fill n-side energy
                stdSinSideEnergy("FPGA", theDetector, detPhysics);
            }
            else if((theDetector->GetDetType()) == "DSSD")  // fill energies for DSSD
            {
                if((theDetector->getNElements()) != 4)
                {
                    TGo4Log::Warn("Detector %s is a DSSD but does not have 4 elements. Detector is setup incorrectly.", dname.Data());
                    return kFALSE;
                }

                //fill DSSD energies
                stdDSSDEnergy("FPGA", theDetector, detPhysics);
            }
            else if((theDetector->GetDetType()) == "Crystal")  // fill energies for crystal
            {
                if((theDetector->getNElements()) != 2)
                {
                    TGo4Log::Warn("Detector %s is a Crystal but does not have 2 elements. Detector is setup incorrectly.", dname.Data());
                    throw std::runtime_error("TPLEIADESPhysProc::stdEnergy Crystal detector above not setup correctly");
                }

                // load the 2 crystal channels from the Raw Event input
                stdCrystalEnergy("FPGA", theDetector, detPhysics);
            }
            else
            {
                TGo4Log::Warn("Detector %s does not have a recognised detector type, and thus can't be filled.", dname.Data());
                return kFALSE;
            }
        }
    }
    catch (const invalid_argument&  error) { TGo4Log::Error(error.what()); return isValid; }
    catch (const runtime_error&     error) { TGo4Log::Error(error.what()); return isValid; }

    //------------------------------------------------------------------------
    // Calculate statistics on signal clipping
    FillClipStatsHists();
    FillTOThreshHists();
    ExpIntegPHRecon();

    fOutEvent->SetValid(isValid);     // now event is filled, store event

    //------------------------------------------------------------------------
    // JAM 12-Dec-2023: Added slow motion (eventwise step mode) as example how to use parameter container flag
    //------------------------------------------------------------------------
    if(fPar->fSlowMotion)
    {
        GO4_STOP_ANALYSIS_MESSAGE("Stopped for slow motion mode after MBS event count %d. Click green arrow for next event. please.", fOutEvent->fSequenceNumber);
    }

    return isValid;
}

//------------------------------------------------------------------------
// standard energy uses FPGA or TRAPEZ energy directly from TPLEIADESDetEvent, ie no pulse shape analysis.
// p-sides are handled by selection of active p-strip for SiPads and summation of L/R for DSSD
void TPLEIADESPhysProc::pStripSelect(TString method, TPLEIADESDetector* theDetector, TPLEIADESDetPhysics* detPhysics)
// method is either FPGA or TRAPEZ
{
    if(!theDetector) { throw std::invalid_argument("TPLEIADESPhysProc::pStripSelect no det event handed to pStripSelect!"); }
    else if(!detPhysics) { throw std::invalid_argument("TPLEIADESPhysProc::pStripSelect no phys event handed to pStripSelect!"); }

    Short_t hitLoc = -99;      // location of first hit, initialised to unphysical -99
    Int_t stripEnergy = -99;  // strip energy placeholder
    Double_t totEnergy = 0;    // value for total energy deposited in p-side

    for(int j=0; j<7; ++j)  // loop over p-strips
    {
        TPLEIADESDetChan *theDetChan = theDetector->GetChannel(j);

        if(method == "FPGA") { stripEnergy = theDetChan->fDFPGAEnergy; }
        else { throw std::runtime_error("TPLEIADESPhysProc::pStripSelect only FPGA method currently implemented"); }

        if(stripEnergy == -99) { throw std::runtime_error("TPLEIADESPhysProc::pStripSelect NEVER COME HERE - strip energy wasn't assigned to fDFPGAEnergy"); }
        else if(stripEnergy != 0)   // hit detected
        {
            if(hitLoc == -99)   // if first hit
            {
                hitLoc = j;
                totEnergy = stripEnergy;
            }
            else if(j == hitLoc+1)  // is hit a neighbour of first hit?
            {
                totEnergy += stripEnergy;
                //TGo4Log::Info("TPLEIADESPhysProc::pStripSelect interstrip event, composite energy used.");
            }
            else
            {
                //TGo4Log::Warn("TPLEIADESPhysProc::pStripSelect multiple strips hit but not neighbours, skipping!");
                totEnergy = -99;
            }
        }
    }

    //if(hitLoc == -99) { TGo4Log::Info("TPLEIADESPhysProc::pStripSelect no hit found on p-strip. Pulser event?"); }
    detPhysics->fpEnergy = totEnergy;   // assign selected p-strip energy to detector
}

//------------------------------------------------------------------------
void TPLEIADESPhysProc::stdSinSideEnergy(TString method, TPLEIADESDetector* theDetector, TPLEIADESDetPhysics* detPhysics)     // fills n-side energy from scalars
{
    if(!theDetector) { throw std::invalid_argument("TPLEIADESPhysProc::pStripSelect no det event handed to pStripSelect!"); }
    else if(!detPhysics) { throw std::invalid_argument("TPLEIADESPhysProc::pStripSelect no phys event handed to pStripSelect!"); }

    TPLEIADESDetChan *nsideDetChan = theDetector->GetChannel(7);
    if(method == "FPGA") { detPhysics->fnEnergy = nsideDetChan->fDFPGAEnergy; }
}

//------------------------------------------------------------------------
void TPLEIADESPhysProc::stdDSSDEnergy(TString method, TPLEIADESDetector* theDetector, TPLEIADESDetPhysics* detPhysics)   // fills DSSD energy from scalars
{
    Int_t chanEnergy = -99; // channel energy placeholder
    Double_t pEnergy = 0;    // value for total energy deposited in p-side
    Double_t nEnergy = 0;    // value for total energy deposited in n-side

    for(int j=0; j<4; ++j)
    {
        TPLEIADESDetChan *theDetChan = theDetector->GetChannel(j);

        if(method == "FPGA") { chanEnergy = theDetChan->fDFPGAEnergy; }
        else { throw std::runtime_error("TPLEIADESPhysProc::stdEnergy only FPGA method currently implemented"); }

        if(chanEnergy == -99) { throw std::runtime_error("TPLEIADESPhysProc::stdEnergy NEVER COME HERE - strip energy wasn't assigned to fDFPGAEnergy"); }
        else
        {
            if(j<2) { pEnergy += chanEnergy; }          // add L/R to pEnergy
            else if(j>=2) { nEnergy += chanEnergy; }    // add T/B to nEnergy
        }
    }

    //fill energies
    detPhysics->fpEnergy = pEnergy;
    detPhysics->fnEnergy = nEnergy;
}

//------------------------------------------------------------------------
void TPLEIADESPhysProc::stdDSSDPosition(TString method, TPLEIADESDetector* theDetector, TPLEIADESDetPhysics* detPhysics) // standard computation of normalised position
{

}

//------------------------------------------------------------------------
void TPLEIADESPhysProc::stdCrystalEnergy(TString method, TPLEIADESDetector* theDetector, TPLEIADESDetPhysics* detPhysics)// fills Crystal energies from scalars
{
    for(int j=0; j<2; ++j)
    {
        TPLEIADESDetChan *theDetChan = theDetector->GetChannel(j);
        Int_t chanEnergy = -99; // channel energy placeholder

        if(method == "FPGA") { chanEnergy = theDetChan->fDFPGAEnergy; }
        else { throw std::runtime_error("TPLEIADESPhysProc::stdEnergy only FPGA method currently implemented"); }

        if(chanEnergy == -99) { throw std::runtime_error("TPLEIADESPhysProc::stdEnergy NEVER COME HERE - strip energy wasn't assigned to fDFPGAEnergy"); }
        else
        {
            if(j==0) { detPhysics->fpEnergy = chanEnergy; }
            else if(j==1) { detPhysics->fnEnergy = chanEnergy; }
        }
    }
}

//------------------------------------------------------------------------
// the following functions are for the pulse height analysis of saturated traces
void TPLEIADESPhysProc::FillClipStatsHists()
{
    std::vector<Double_t> trace, traceBLR;
    Int_t   startRise,  satTime,    satReentry;
    Bool_t  startRec,   satRec,     reentRec;
    Int_t nsideCnt = 0;

    for(const TString& dname : fPar->fDetNameVec)
    {
        TPLEIADESDetector *theDetector = fInEvent->GetDetector(dname);
        startRise = -99; satTime = -999; satReentry = -999;
        startRec = satRec = reentRec = kFALSE;

        if(theDetector->GetDetType() == "SiPad")
        {
            trace = theDetector->GetChannel(7)->fDTrace;
            traceBLR = theDetector->GetChannel(7)->fDTraceBLR;

            for(uint i=0; i < (traceBLR.size()-5); ++i)
            {
                if(!startRec && trace[i+1]-trace[i] > 300) { startRec = kTRUE; startRise = i; }
                if(!satRec   && trace[i] == 16383) { satRec = kTRUE; satTime = i; }
                if(!reentRec && trace[i] == 16383 && trace[i+1]<trace[i] && trace[i+2] != 16383 && trace[i+3] != 16383 && trace[i+4] != 16383 && trace[i+5] != 16383) { reentRec = kTRUE; satReentry = i; }
            }
            //std::cout << "n Side " << dname << ": Signal start time was " << startRise << ", saturation time was " << satTime << ", reentry time was " << satReentry << std::endl;
            fPhysDisplay->hRiseTimeNSides[nsideCnt]->Fill(satTime-startRise);
            fPhysDisplay->hReentryTimeNSides[nsideCnt]->Fill(satReentry-startRise);
            fPhysDisplay->hPulseTimeNSides[nsideCnt]->Fill(2997-startRise);
            fPhysDisplay->hClipHeightNSides[nsideCnt]->Fill(traceBLR[satTime]);
            if(satTime != -999) { fPhysDisplay->hEndHeightNSides[nsideCnt]->Fill(traceBLR[2997]); }
            fPhysDisplay->hClipLenVHghtNSides[nsideCnt]->Fill(satReentry-startRise, traceBLR[satTime]);

            nsideCnt += 1;
        }
        else if(theDetector->GetDetType() == "Crystal")
        {
            trace = theDetector->GetChannel(0)->fDTrace;
            traceBLR = theDetector->GetChannel(0)->fDTraceBLR;

            for(uint i=0; i < (traceBLR.size()-5); ++i)
            {
                if(!startRec && trace[i+1]-trace[i] > 300) { startRec = kTRUE; startRise = i; }
                if(!satRec   && trace[i] == 16383) { satRec = kTRUE; satTime = i; }
                if(!reentRec && trace[i] == 16383 && trace[i+1]<trace[i] && trace[i+2] != 16383 && trace[i+3] != 16383 && trace[i+4] != 16383 && trace[i+5] != 16383) { reentRec = kTRUE; satReentry = i; }
            }
            //std::cout << "Crys Front: Signal start time was " << startRise << ", saturation time was " << satTime << ", reentry time was " << satReentry << std::endl;
            fPhysDisplay->hRiseTimeCrysFr->Fill(satTime-startRise);
            fPhysDisplay->hReentryTimeCrysFr->Fill(satReentry-startRise);
            fPhysDisplay->hPulseTimeCrysFr->Fill(2997-startRise);
            fPhysDisplay->hClipHeightCrysFr->Fill(traceBLR[satTime]);
            if(satTime != -999) { fPhysDisplay->hEndHeightCrysFr->Fill(traceBLR[2997]); }
            fPhysDisplay->hClipLenVHghtCrysFr->Fill(satReentry-startRise, traceBLR[satTime]);

            // repeat for back side
            startRise = -99; satTime = -999;
            startRec = satRec = reentRec = kFALSE;

            trace = theDetector->GetChannel(1)->fDTrace;
            traceBLR = theDetector->GetChannel(1)->fDTraceBLR;

            for(uint i=0; i < (traceBLR.size()-5); ++i)
            {
                if(!startRec && trace[i+1]-trace[i] > 300) { startRec = kTRUE; startRise = i; }
                if(!satRec   && trace[i] == 16383) { satRec = kTRUE; satTime = i; }
                if(!reentRec && trace[i] == 16383 && trace[i+1]<trace[i] && trace[i+2] != 16383 && trace[i+3] != 16383 && trace[i+4] != 16383 && trace[i+5] != 16383) { reentRec = kTRUE; satReentry = i; }
            }
            //std::cout << "Crys Back: Signal start time was " << startRise << ", saturation time was " << satTime << ", reentry time was " << satReentry << std::endl;
            fPhysDisplay->hRiseTimeCrysBk->Fill(satTime-startRise);
            fPhysDisplay->hReentryTimeCrysBk->Fill(satReentry-startRise);
            fPhysDisplay->hPulseTimeCrysBk->Fill(2997-startRise);
            fPhysDisplay->hClipHeightCrysBk->Fill(traceBLR[satTime]);
            if(satTime != -999) { fPhysDisplay->hEndHeightCrysBk->Fill(traceBLR[2997]); }
            fPhysDisplay->hClipLenVHghtCrysBk->Fill(satReentry-startRise, traceBLR[satTime]);
        }
    }
}

//------------------------------------------------------------------------
void TPLEIADESPhysProc::FillTOThreshHists()
{
    std::vector<Double_t> trace, traceBLR;
    std::vector<Int_t>   startThresh(3, -99), stopThresh(3, -999);
    std::vector<Bool_t>  startRec(3, kFALSE), stopRec(3, kFALSE);
    Bool_t satTrace;
    Int_t   nsideCnt = 0;

    for(const TString& dname : fPar->fDetNameVec)
    {
        TPLEIADESDetector *theDetector = fInEvent->GetDetector(dname);

        if(theDetector->GetDetType() == "SiPad")
        {
            trace = theDetector->GetChannel(7)->fDTrace;
            traceBLR = theDetector->GetChannel(7)->fDTraceBLR;
            std::fill(startThresh.begin(), startThresh.end(), -99);     std::fill(stopThresh.begin(), stopThresh.end(), -999);
            std::fill(startRec.begin(), startRec.end(), kFALSE);        std::fill(stopRec.begin(), stopRec.end(), kFALSE);
            satTrace = kFALSE;

            for(uint i=0; i<traceBLR.size()-5; ++i)
            {
                if(!satTrace && trace[i] == 16383)  { satTrace = kTRUE; }

                if(!startRec[0] && traceBLR[i]>500  && traceBLR[i+5]>traceBLR[i]) { startRec[0] = kTRUE; startThresh[0] = i; }
                if(!startRec[1] && traceBLR[i]>1000 && traceBLR[i+5]>traceBLR[i]) { startRec[1] = kTRUE; startThresh[1] = i; }
                if(!startRec[2] && traceBLR[i]>1500 && traceBLR[i+5]>traceBLR[i]) { startRec[2] = kTRUE; startThresh[2] = i; }

                if(startRec[0] && !stopRec[0] && traceBLR[i]<500  && traceBLR[i+5]<traceBLR[i]) { stopRec[0] = kTRUE; stopThresh[0] = i; }
                if(startRec[1] && !stopRec[1] && traceBLR[i]<1000 && traceBLR[i+5]<traceBLR[i]) { stopRec[1] = kTRUE; stopThresh[1] = i; }
                if(startRec[2] && !stopRec[2] && traceBLR[i]<1500 && traceBLR[i+5]<traceBLR[i]) { stopRec[2] = kTRUE; stopThresh[2] = i; }
            }
            if(satTrace)
            {
                fPhysDisplay->hTOThreshNSides[nsideCnt][0]->Fill(stopThresh[0] - startThresh[0]);
                fPhysDisplay->hTOThreshNSides[nsideCnt][1]->Fill(stopThresh[1] - startThresh[1]);
                fPhysDisplay->hTOThreshNSides[nsideCnt][2]->Fill(stopThresh[2] - startThresh[2]);
                //std::cout << "Event " << fInEvent->fSequenceNumber <<", detector " << theDetector->GetDetName() << ". Threshold 300: start " << startThresh[0] << ", stop " << stopThresh[0] << ". 1200: start " << startThresh[1] << ", stop " << stopThresh[1] << ". 2000: start " << startThresh[2] << ", stop " << stopThresh[2] << std::endl;
            }

            nsideCnt += 1;
        }
        else if(theDetector->GetDetType() == "Crystal")
        {
            trace = theDetector->GetChannel(0)->fDTrace;
            traceBLR = theDetector->GetChannel(0)->fDTraceBLR;
            std::fill(startThresh.begin(), startThresh.end(), -99);     std::fill(stopThresh.begin(), stopThresh.end(), -999);
            std::fill(startRec.begin(), startRec.end(), kFALSE);        std::fill(stopRec.begin(), stopRec.end(), kFALSE);
            satTrace = kFALSE;

            for(uint i=0; i<traceBLR.size(); ++i)
            {
                if(!satTrace && trace[i] == 16383)  { satTrace = kTRUE; }

                if(!startRec[0] && traceBLR[i]>1000 && traceBLR[i+5]>traceBLR[i]) { startRec[0] = kTRUE; startThresh[0] = i; }
                if(!startRec[1] && traceBLR[i]>1400 && traceBLR[i+5]>traceBLR[i]) { startRec[1] = kTRUE; startThresh[1] = i; }
                if(!startRec[2] && traceBLR[i]>1800 && traceBLR[i+5]>traceBLR[i]) { startRec[2] = kTRUE; startThresh[2] = i; }

                if(startRec[0] && !stopRec[0] && traceBLR[i]<1000 && traceBLR[i+5]<traceBLR[i]) { stopRec[0] = kTRUE; stopThresh[0] = i; }
                if(startRec[1] && !stopRec[1] && traceBLR[i]<1400 && traceBLR[i+5]<traceBLR[i]) { stopRec[1] = kTRUE; stopThresh[1] = i; }
                if(startRec[2] && !stopRec[2] && traceBLR[i]<1800 && traceBLR[i+5]<traceBLR[i]) { stopRec[2] = kTRUE; stopThresh[2] = i; }
            }
            if(satTrace)
            {
                fPhysDisplay->hTOThreshCrysFr[0]->Fill(stopThresh[0] - startThresh[0]);
                fPhysDisplay->hTOThreshCrysFr[1]->Fill(stopThresh[1] - startThresh[1]);
                fPhysDisplay->hTOThreshCrysFr[2]->Fill(stopThresh[2] - startThresh[2]);
                //std::cout << "Event " << fInEvent->fSequenceNumber <<", detector " << theDetector->GetDetName() << ". Threshold 300: start " << startThresh[0] << ", stop " << stopThresh[0] << ". 1200: start " << startThresh[1] << ", stop " << stopThresh[1] << ". 2000: start " << startThresh[2] << ", stop " << stopThresh[2] << std::endl;
            }

            // repeat for back side
            std::fill(startThresh.begin(), startThresh.end(), -99);     std::fill(stopThresh.begin(), stopThresh.end(), -999);
            std::fill(startRec.begin(), startRec.end(), kFALSE);        std::fill(stopRec.begin(), stopRec.end(), kFALSE);
            satTrace = kFALSE;

            trace = theDetector->GetChannel(1)->fDTrace;
            traceBLR = theDetector->GetChannel(1)->fDTraceBLR;

            for(uint i=0; i<traceBLR.size(); ++i)
            {
                if(!satTrace && trace[i] == 16383)  { satTrace = kTRUE; }

                if(!startRec[0] && traceBLR[i]>1000 && traceBLR[i+5]>traceBLR[i]) { startRec[0] = kTRUE; startThresh[0] = i; }
                if(!startRec[1] && traceBLR[i]>1400 && traceBLR[i+5]>traceBLR[i]) { startRec[1] = kTRUE; startThresh[1] = i; }
                if(!startRec[2] && traceBLR[i]>1800 && traceBLR[i+5]>traceBLR[i]) { startRec[2] = kTRUE; startThresh[2] = i; }

                if(startRec[0] && !stopRec[0] && traceBLR[i]<1000 && traceBLR[i+5]<traceBLR[i]) { stopRec[0] = kTRUE; stopThresh[0] = i; }
                if(startRec[1] && !stopRec[1] && traceBLR[i]<1400 && traceBLR[i+5]<traceBLR[i]) { stopRec[1] = kTRUE; stopThresh[1] = i; }
                if(startRec[2] && !stopRec[2] && traceBLR[i]<1800 && traceBLR[i+5]<traceBLR[i]) { stopRec[2] = kTRUE; stopThresh[2] = i; }
            }
            if(satTrace)
            {
                fPhysDisplay->hTOThreshCrysBk[0]->Fill(stopThresh[0] - startThresh[0]);
                fPhysDisplay->hTOThreshCrysBk[1]->Fill(stopThresh[1] - startThresh[1]);
                fPhysDisplay->hTOThreshCrysBk[2]->Fill(stopThresh[2] - startThresh[2]);
                //std::cout << "Event " << fInEvent->fSequenceNumber <<", detector " << theDetector->GetDetName() << ". Threshold 300: start " << startThresh[0] << ", stop " << stopThresh[0] << ". 1200: start " << startThresh[1] << ", stop " << stopThresh[1] << ". 2000: start " << startThresh[2] << ", stop " << stopThresh[2] << std::endl;
            }
        }
    }
}

//------------------------------------------------------------------------
void TPLEIADESPhysProc::ExpFitPHRecon() //std::vector<TF1*> linFunc, std::vector<TF1*> expFunc)
{
    /********************************************************************************
     * This function is currently unstable and crashes Go4 if a stop is requested.  *
     * I suspect there is a memory leak somewhere. Proceed with caution!            *
     ********************************************************************************/

    std::vector<Double_t> trace, traceBLR;
    Int_t   startRise;      Bool_t  startRec, satEv;
    TString chanName;
    Int_t nsideCnt = 0;

    TGo4Picture *fitPic = new TGo4Picture("PH Recon Fits", "PH Recon Fits", 3, 3);
    std::vector<std::vector<short>> coords = { {0,0}, {0,1}, {0,2}, {1,0}, {1,1}, {1,2}, {2,0}, {2,1}, {2,2} };
    TF1 *linFunc = new TF1("linf","[0]+[1]*x", 0, 3e3);         //gROOT->GetListOfFunctions()->Add(linFunc);
    TF1 *expFunc = new TF1("expf","[0]*exp(-[1]*x)", 0, 3e3);   //gROOT->GetListOfFunctions()->Add(expFunc);

    //TF1 intersection functions
    auto finter = [linFunc, expFunc](double *x, double *par) -> Double_t { return TMath::Abs(linFunc->Eval(x[0]) - expFunc->Eval(x[0])); };
    TF1 *fint = new TF1("fint", finter, 0, 3e3, 0);       //Form("abs(%s-%s)", linFunc->GetName(), expFunc->GetName())

    for(const TString& dname : fPar->fDetNameVec)
    {
        TPLEIADESDetector *theDetector = fInEvent->GetDetector(dname);
        startRise = -99;    startRec = satEv = kFALSE;

        if(theDetector->GetDetType() == "SiPad")
        {
            trace = theDetector->GetChannel(7)->fDTrace;
            traceBLR = theDetector->GetChannel(7)->fDTraceBLR;

            for(uint i=0; i < (traceBLR.size()-5); ++i)
            {
                if(!startRec && trace[i+1]-trace[i] > 300) { startRec = kTRUE; startRise = i; }
                if(!satEv && trace[i] == 16383) { satEv = kTRUE; }
            }

            if(satEv)
            {
                chanName.Form("%s_nSide", theDetector->GetDetName().Data());
                TH1 *traceBLRHist = fInEvent->fDetDisplays[theDetector->getId()]->GetChanDisplay(chanName)->hTraceBLRChan;
                TList* lis = traceBLRHist->GetListOfFunctions();
                if(lis) lis->Delete(); // get rid of previous fit results
                traceBLRHist->Fit(linFunc, "WCQF+", "", startRise-1, startRise+5);
                traceBLRHist->Fit(expFunc, "WCQF+", "", 800, 2995);
                fitPic->Pic(coords[nsideCnt][0], coords[nsideCnt][1])->AddH1(traceBLRHist);
                fitPic->Pic(coords[nsideCnt][0], coords[nsideCnt][1])->SetDrawOption("L");
                AddPicture(fitPic);

                Double_t xint = fint->GetMinimumX();
                std::cout << "Intersection point is " << xint << ", int value is " << linFunc->Eval(xint) << std::endl;
                fPhysDisplay->hExpFitNSides[nsideCnt]->Fill(linFunc->Eval(xint));
            }

            nsideCnt += 1;
        }
        else if(theDetector->GetDetType() == "Crystal")
        {
            trace = theDetector->GetChannel(0)->fDTrace;
            traceBLR = theDetector->GetChannel(0)->fDTraceBLR;

            for(uint i=0; i < (traceBLR.size()-5); ++i)
            {
                if(!startRec && trace[i+1]-trace[i] > 300) { startRec = kTRUE; startRise = i; }
                if(!satEv && trace[i] == 16383) { satEv = kTRUE; }
            }

            if(satEv)
            {
                chanName.Form("%s_CrysFrnt", theDetector->GetDetName().Data());
                TH1 *traceBLRHist = fInEvent->fDetDisplays[theDetector->getId()]->GetChanDisplay(chanName)->hTraceBLRChan;
                TList* lis = traceBLRHist->GetListOfFunctions();
                if(lis) lis->Delete(); // get rid of previous fit results
                traceBLRHist->Fit(linFunc, "WCQF+", "", startRise-1, startRise+5);
                traceBLRHist->Fit(expFunc, "WCQF+", "", 1200, 2995);
                fitPic->Pic(coords[7][0], coords[7][1])->AddH1(traceBLRHist);
                fitPic->Pic(coords[7][0], coords[7][1])->SetDrawOption("L");
                AddPicture(fitPic);

                Double_t xint = fint->GetMinimumX();
                std::cout << "Intersection point is " << xint << ", int value is " << linFunc->Eval(xint) << std::endl;
                fPhysDisplay->hExpFitCrysFr->Fill(linFunc->Eval(xint));
            }

            // repeat for back side
            startRise = -99;    startRec = satEv = kFALSE;
            trace = theDetector->GetChannel(1)->fDTrace;
            traceBLR = theDetector->GetChannel(1)->fDTraceBLR;

            for(uint i=0; i < (traceBLR.size()-5); ++i)
            {
                if(!startRec && trace[i+1]-trace[i] > 300) { startRec = kTRUE; startRise = i; }
                if(!satEv && trace[i] == 16383) { satEv = kTRUE; }
            }

            if(satEv)
            {
                chanName.Form("%s_CrysBack", theDetector->GetDetName().Data());
                TH1 *traceBLRHist = fInEvent->fDetDisplays[theDetector->getId()]->GetChanDisplay(chanName)->hTraceBLRChan;
                TList* lis = traceBLRHist->GetListOfFunctions();
                if(lis) lis->Delete(); // get rid of previous fit results
                traceBLRHist->Fit(linFunc, "WCQF+", "", startRise-1, startRise+5);
                traceBLRHist->Fit(expFunc, "WCQF+", "", 1500, 2995);
                fitPic->Pic(coords[8][0], coords[8][1])->AddH1(traceBLRHist);
                fitPic->Pic(coords[8][0], coords[8][1])->SetDrawOption("L");
                AddPicture(fitPic);

                Double_t xint = fint->GetMinimumX();
                std::cout << "Intersection point is " << xint << ", int value is " << linFunc->Eval(xint) << std::endl;
                fPhysDisplay->hExpFitCrysBk->Fill(linFunc->Eval(xint));
            }
        }
    }
}

//------------------------------------------------------------------------
void TPLEIADESPhysProc::ExpIntegPHRecon()
{
    std::vector<Double_t> trace, traceBLR;
    Short_t startRise, consStart, indivStart, stopInt;
    std::vector<Short_t> indivClipLen = { 250, 300, 300, 350, 400, 450, 500, 900 };
    Bool_t  startRec, satTrace;
    Int_t   nsideCnt = 0;
    UInt_t  consInteg, indivInteg;

    for(const TString& dname : fPar->fDetNameVec)
    {
        TPLEIADESDetector *theDetector = fInEvent->GetDetector(dname);

        if(theDetector->GetDetType() == "SiPad")
        {
            trace = theDetector->GetChannel(7)->fDTrace;
            traceBLR = theDetector->GetChannel(7)->fDTraceBLR;
            startRise = -9999;  startRec = satTrace = kFALSE;

            for(uint i=0; i<traceBLR.size(); ++i)
            {
                if(!satTrace && trace[i] == 16383)  { satTrace = kTRUE; }
                if(!startRec && trace[i+1]-trace[i] > 300) { startRec = kTRUE; startRise = i; }

            }
            if(satTrace)
            {
                consStart = startRise+900;  stopInt = startRise+2800;
                indivStart = startRise + indivClipLen[nsideCnt];
                if(stopInt>3000) { TGo4Log::Info("TPLEIADESPhysProc::ExpIntegPHRecon - stopInt > trace length!"); continue; }
                consInteg = std::accumulate(traceBLR.begin()+consStart, traceBLR.begin()+stopInt, 0);
                indivInteg = std::accumulate(traceBLR.begin()+indivStart, traceBLR.begin()+stopInt, 0);
                fPhysDisplay->hExpIntegNSides[nsideCnt][0]->Fill(consInteg);
                fPhysDisplay->hExpIntegNSides[nsideCnt][1]->Fill(indivInteg);
                //std::cout << "Event " << fInEvent->fSequenceNumber <<", detector " << theDetector->GetDetName() << ". Threshold 300: start " << startThresh[0] << ", stop " << stopThresh[0] << ". 1200: start " << startThresh[1] << ", stop " << stopThresh[1] << ". 2000: start " << startThresh[2] << ", stop " << stopThresh[2] << std::endl;
            }

            nsideCnt += 1;
        }
        else if(theDetector->GetDetType() == "Crystal")
        {
            trace = theDetector->GetChannel(0)->fDTrace;
            traceBLR = theDetector->GetChannel(0)->fDTraceBLR;
            startRise = -9999;  startRec = satTrace = kFALSE;

            for(uint i=0; i<traceBLR.size(); ++i)
            {
                if(!satTrace && trace[i] == 16383)  { satTrace = kTRUE; }
                if(!startRec && trace[i+1]-trace[i] > 300) { startRec = kTRUE; startRise = i; }

            }
            if(satTrace)
            {
                consStart = startRise+900;  stopInt = startRise+2800;
                indivStart = startRise + indivClipLen[6];
                if(stopInt>3000) { TGo4Log::Info("TPLEIADESPhysProc::ExpIntegPHRecon - stopInt > trace length!"); continue; }
                consInteg = std::accumulate(traceBLR.begin()+consStart, traceBLR.begin()+stopInt, 0);
                indivInteg = std::accumulate(traceBLR.begin()+indivStart, traceBLR.begin()+stopInt, 0);
                fPhysDisplay->hExpIntegCrysFr[0]->Fill(consInteg);
                fPhysDisplay->hExpIntegCrysFr[1]->Fill(indivInteg);
                //std::cout << "Event " << fInEvent->fSequenceNumber <<", detector " << theDetector->GetDetName() << ". Threshold 300: start " << startThresh[0] << ", stop " << stopThresh[0] << ". 1200: start " << startThresh[1] << ", stop " << stopThresh[1] << ". 2000: start " << startThresh[2] << ", stop " << stopThresh[2] << std::endl;
            }

            // repeat for back side
            trace = theDetector->GetChannel(1)->fDTrace;
            traceBLR = theDetector->GetChannel(1)->fDTraceBLR;
            startRise = -9999;  startRec = satTrace = kFALSE;

            for(uint i=0; i<traceBLR.size(); ++i)
            {
                if(!satTrace && trace[i] == 16383)  { satTrace = kTRUE; }
                if(!startRec && trace[i+1]-trace[i] > 300) { startRec = kTRUE; startRise = i; }

            }
            if(satTrace)
            {
                consStart = startRise+900;  stopInt = startRise+2800;
                indivStart = startRise + indivClipLen[7];
                if(stopInt>3000) { TGo4Log::Info("TPLEIADESPhysProc::ExpIntegPHRecon - stopInt > trace length!"); continue; }
                consInteg = std::accumulate(traceBLR.begin()+consStart, traceBLR.begin()+stopInt, 0);
                indivInteg = std::accumulate(traceBLR.begin()+indivStart, traceBLR.begin()+stopInt, 0);
                fPhysDisplay->hExpIntegCrysBk[0]->Fill(consInteg);
                fPhysDisplay->hExpIntegCrysBk[1]->Fill(indivInteg);
                //std::cout << "Event " << fInEvent->fSequenceNumber <<", detector " << theDetector->GetDetName() << ". Threshold 300: start " << startThresh[0] << ", stop " << stopThresh[0] << ". 1200: start " << startThresh[1] << ", stop " << stopThresh[1] << ". 2000: start " << startThresh[2] << ", stop " << stopThresh[2] << std::endl;
            }
        }
    }
}


//----------------------------END OF GO4 SOURCE FILE ---------------------
