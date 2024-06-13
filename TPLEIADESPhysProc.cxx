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
// standard energy uses FPGA or TRAPEZ energy directly from TPLEIADESDetEvent, ie no pulse shape analysis.
// p-sides are handled by selection of active p-strip for SiPads and summation of L/R for DSSD
Int_t TPLEIADESPhysProc::pStripSelect(TString method, TPLEIADESDetector* theDetector, TPLEIADESDetPhysics* detPhysics)
// method is either FPGA or TRAPEZ, detName is detector name
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
                TGo4Log::Info("TPLEIADESPhysProc::pStripSelect interstrip event, composite energy used.");
            }
            else
            {
                TGo4Log::Warn("TPLEIADESPhysProc::pStripSelect multiple strips hit but not neighbours, skipping!");
                totEnergy = -99;
            }
        }
    }

    if(hitLoc == -99) { TGo4Log::Info("TPLEIADESPhysProc::pStripSelect no hit found on p-strip. Why did event trigger?"); }
    detPhysics->fpEnergy = totEnergy;   // assign selected p-strip energy to detector

    return 0;
}

//------------------------------------------------------------------------
Int_t TPLEIADESPhysProc::stdSinSideEnergy(TString method, TPLEIADESDetector* theDetector, TPLEIADESDetPhysics* detPhysics)     // fills n-side energy from scalars
{
    if(!theDetector) { throw std::invalid_argument("TPLEIADESPhysProc::pStripSelect no det event handed to pStripSelect!"); }
    else if(!detPhysics) { throw std::invalid_argument("TPLEIADESPhysProc::pStripSelect no phys event handed to pStripSelect!"); }

    TPLEIADESDetChan *nsideDetChan = theDetector->GetChannel(7);
    if(method == "FPGA") { detPhysics->fnEnergy = nsideDetChan->fDFPGAEnergy; }

    return 0;
}

//------------------------------------------------------------------------
Int_t TPLEIADESPhysProc::stdDSSDEnergy(TString method, TPLEIADESDetector* theDetector, TPLEIADESDetPhysics* detPhysics)   // fills DSSD energy from scalars
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

    return 0;
}

//------------------------------------------------------------------------
Int_t TPLEIADESPhysProc::stdDSSDPosition(TString method, TPLEIADESDetector* theDetector, TPLEIADESDetPhysics* detPhysics) // standard computation of normalised position
{
    return 0;
}

//------------------------------------------------------------------------
Int_t TPLEIADESPhysProc::stdCrystalEnergy(TString method, TPLEIADESDetector* theDetector, TPLEIADESDetPhysics* detPhysics)// fills Crystal energies from scalars
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

    return 0;
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

    try
    {
        // loop over detectors to fill physics properties
        for(const TString& dname : fPar->fDetNameVec)
        {
            TPLEIADESDetector *theDetector = detEvent->GetDetector(dname);
            TPLEIADESDetPhysics *detPhysics = fOutEvent->GetDetector(dname);

            if((theDetector->GetDetType()) == "SiPad")  // select SiPads for pStrip search
            {
                if((theDetector->getNElements()) != 8)
                {
                    TGo4Log::Error("Detector %s is a Si Pad but does not have 8 elements. Detector is setup incorrectly.", dname.Data());
                    return kFALSE;
                }

                // search for active p-strip for p-side energy
                TPLEIADESPhysProc::pStripSelect("FPGA", theDetector, detPhysics);

                //fill n-side energy
                TPLEIADESPhysProc::stdSinSideEnergy("FPGA", theDetector, detPhysics);
            }
            else if((theDetector->GetDetType()) == "DSSD")  // fill energies for DSSD
            {
                if((theDetector->getNElements()) != 6)
                {
                    TGo4Log::Warn("Detector %s is a DSSD but does not have 6 elements. Detector is setup incorrectly.", dname.Data());
                    return kFALSE;
                }

                //fill DSSD energies
                TPLEIADESPhysProc::stdDSSDEnergy("FPGA", theDetector, detPhysics);
            }
            else if((theDetector->GetDetType()) == "Crystal")  // fill energies for crystal
            {
                if((theDetector->getNElements()) != 2)
                {
                    TGo4Log::Warn("Detector %s is a Crystal but does not have 2 elements. Detector is setup incorrectly.", dname.Data());
                    throw std::runtime_error("TPLEIADESPhysProc::stdEnergy Crystal detector above not setup correctly");
                }

                // load the 2 crystal channels from the Raw Event input
                TPLEIADESPhysProc::stdCrystalEnergy("FPGA", theDetector, detPhysics);
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

    fOutEvent->SetValid(isValid);     // now event is filled, store event

    return isValid;
}

//----------------------------END OF GO4 SOURCE FILE ---------------------
