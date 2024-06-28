//------------------------------------------------------------------------
//************************ TPLEIADESDetProc.cxx **************************
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

#include "TPLEIADESDetProc.h"
#include "TPLEIADESRawEvent.h"
#include "TPLEIADESDisplay.h"

#include "TH1.h"
#include "TH2.h"

//------------------------------------------------------------------------
TPLEIADESDetProc::TPLEIADESDetProc() : TGo4EventProcessor("Proc")
{
    TGo4Log::Info("TPLEIADESDetProc: Create instance ");
}

//------------------------------------------------------------------------
// this one is used in standard factory
TPLEIADESDetProc::TPLEIADESDetProc(const char* name) :
    TGo4EventProcessor(name), fOutEvent(0)
{
    TGo4Log::Info("TPLEIADESDetProc: Create instance %s", name);
    // no need to set param macro as this was already done in first step
    fPar = dynamic_cast<TPLEIADESParam*>(MakeParameter("PLEIADESParam", "TPLEIADESParam"));
    if(fPar)
    {
        fPar->SetConfigDetEvent();
        fPar->SetConfigDisplay();
    }
}

//------------------------------------------------------------------------
TPLEIADESDetProc::~TPLEIADESDetProc()
{
    TGo4Log::Info("TPLEIADESDetProc: Delete instance ");
}

void TPLEIADESDetProc::InitDisplays(TPLEIADESDetEvent* out)
{
    // loop through detectors in DetEvent
    for(int id=0; id<(out->getNElements()); ++id)
    {
        const char* name = out->getEventElement(id)->GetName();
        TPLEIADESDetector *theDetector = out->GetDetector(name);
        TPLEIADESDetDisplay *detDisplay = new TPLEIADESDetDisplay(theDetector);
        detDisplay->InitDisplay();

        // loop through channels in each detector object
        for(int ch=0; ch<(theDetector->getNElements()); ++ch)
        {
            const char* name2 = theDetector->getEventElement(ch)->GetName();
            TPLEIADESDetChan *theChannnel = theDetector->GetChannel(name2);
            TPLEIADESChanDisplay *chanDisplay = new TPLEIADESChanDisplay(theChannnel);
            chanDisplay->InitDisplay();
            detDisplay->AddChanDisplay(chanDisplay);    // link channel to detector display
        }

        out->fDetDisplays.push_back(detDisplay);
    }
}

//------------------------------------------------------------------------
// Build event is the unpacker function, it reads the MBS event and does stuff with it.
// Histograms are erased with the next word, so are mostly used for online analysis.
Bool_t TPLEIADESDetProc::BuildEvent(TGo4EventElement* target)
{
    Bool_t isValid = kFALSE;    // validity of output event

    TPLEIADESRawEvent *RawEvent = (TPLEIADESRawEvent*) GetInputEvent();
    if(!RawEvent || !RawEvent->IsValid())
    {
        TGo4Log::Error("TPLEIADESDetProc: no input event !");
        return isValid;
    }

    if(fOutEvent == 0)
    {
        fOutEvent = (TPLEIADESDetEvent*) target;
        InitDisplays(fOutEvent);    // initialise display histograms
    }

    fOutEvent->fSequenceNumber = RawEvent->fSequenceNumber;
    fOutEvent->fPhysTrigger = RawEvent->fPhysTrigger;

    // initialize next output as not filled, i.e. it is only stored when something is in
    fOutEvent->SetValid(isValid);

    isValid = kTRUE;    // input/output events look good

    // loop over detectors to fill data from raw output using mapping
    for(const TString& dname : fPar->fDetNameVec)
    {
        TPLEIADESDetector *theDetector = fOutEvent->GetDetector(dname);
        TPLEIADESDetDisplay *detDisplay = fOutEvent->fDetDisplays[theDetector->getId()];
        //std::cout << "Sanity check!: detector name: " << theDetector->GetDetName() << " matches det display: " << detDisplay->fDetector->GetDetName() << std::endl;

        if(fOutEvent->fPhysTrigger) { detDisplay->hDetHitPattern->Fill(-1,1); } // fill physics trigger bin in hit pattern

        UInt_t rawChPos;

        if((theDetector->GetDetType()) == "SiPad")
        {
            if((theDetector->getNElements()) != 8)
            {
                TGo4Log::Warn("Detector %s is a Si Pad but does not have 8 elements. Detector is setup incorrectly.", dname.Data());
                return kFALSE;
            }

            // load the relevant p-side FEBEX Board from the Raw Event input
            UInt_t pBoardID = (fPar->fpSideMap[dname] >> 4);          // bitwise shift to just select board location
            TPLEIADESFebBoard *pBoard = RawEvent->GetBoard(pBoardID);  // get board from input event with board location

            // load the data for the p-strips and n-side
            for(int j=0; j<7; ++j)
            {
                TPLEIADESDetChan *theDetChan = theDetector->GetChannel(j); // get detector channel
                TPLEIADESChanDisplay *chanDisplay = detDisplay->GetChanDisplay(theDetChan->GetName()); // get chan display associated with channel

                rawChPos = (theDetChan->GetChanMap() & 0x00F);          // bitwise AND to select last bits where channel location is
                TPLEIADESFebChannel *theRawChan = pBoard->GetChannel(rawChPos);

                // load hit multiplicity information
                if(fOutEvent->fPhysTrigger) { chanDisplay->hHitMultiplicity->Fill(-1); }
                theDetChan->fDPolarity = theRawChan->fRPolarity;
                theDetChan->fDHitMultiplicity = theRawChan->fRHitMultiplicity;
                chanDisplay->hHitMultiplicity->Fill(theDetChan->fDHitMultiplicity);
                if(theDetChan->fDHitMultiplicity > 0) { detDisplay->hDetHitPattern->Fill(j); }
                if(theDetChan->fDHitMultiplicity == 1) { detDisplay->hDetEnergyPattern->Fill(j, theDetChan->fDTrapezEnergy); }

                // load energy information
                theDetChan->fDFPGAEnergy      = theRawChan->fRFPGAEnergy;
                theDetChan->fDFPGAHitTime     = theRawChan->fRFPGAHitTime;
                theDetChan->fDFPGATRAPEZ      = theRawChan->fRFPGATRAPEZ;

                // load trace information
                #ifdef TPLEIADES_FILL_TRACES
                theDetChan->fDTrapezEnergy    = theRawChan->fRTrapezEnergy;
                theDetChan->fDTrace           = theRawChan->fRTrace;
                theDetChan->fDTraceBLR        = theRawChan->fRTraceBLR;
                theDetChan->fDTraceTRAPEZ     = theRawChan->fRTraceTRAPEZ;
                chanDisplay->FillTraces();
                #endif
            }

            // load the data for the n-side
            UInt_t nBoardID = (fPar->fnSideMap[dname] >> 4);           // bitwise shift to just select board location
            TPLEIADESFebBoard *nBoard = RawEvent->GetBoard(nBoardID);   // get board from input event with board location

            TPLEIADESDetChan *theDetChan = theDetector->GetChannel(7);
            TPLEIADESChanDisplay *chanDisplay = detDisplay->GetChanDisplay(theDetChan->GetName());

            rawChPos = (theDetChan->GetChanMap() & 0x00F);              // bitwise AND to select last bits where channel location is
            TPLEIADESFebChannel *theRawChan = nBoard->GetChannel(rawChPos);

            // load hit multiplicity information
            if(fOutEvent->fPhysTrigger) { chanDisplay->hHitMultiplicity->Fill(-1); }
            theDetChan->fDPolarity = theRawChan->fRPolarity;
            theDetChan->fDHitMultiplicity = theRawChan->fRHitMultiplicity;
            chanDisplay->hHitMultiplicity->Fill(theDetChan->fDHitMultiplicity);
            if(theDetChan->fDHitMultiplicity > 0) { detDisplay->hDetHitPattern->Fill(7); }
            if(theDetChan->fDHitMultiplicity == 1) { detDisplay->hDetEnergyPattern->Fill(7, theDetChan->fDTrapezEnergy); }

            // load energy information
            theDetChan->fDFPGAEnergy      = theRawChan->fRFPGAEnergy;
            theDetChan->fDFPGAHitTime     = theRawChan->fRFPGAHitTime;
            theDetChan->fDFPGATRAPEZ      = theRawChan->fRFPGATRAPEZ;

            // load trace information
            #ifdef TPLEIADES_FILL_TRACES
            theDetChan->fDTrapezEnergy    = theRawChan->fRTrapezEnergy;
            theDetChan->fDTrace           = theRawChan->fRTrace;
            theDetChan->fDTraceBLR        = theRawChan->fRTraceBLR;
            theDetChan->fDTraceTRAPEZ     = theRawChan->fRTraceTRAPEZ;
            chanDisplay->FillTraces();
            #endif
        }
        else if((theDetector->GetDetType()) == "DSSD")
        {
            if((theDetector->getNElements()) != 4)
            {
                TGo4Log::Warn("Detector %s is a DSSD but does not have 6 elements. Detector is setup incorrectly.", dname.Data());
                return kFALSE;
            }

            // load the 4 DSSD channels from the Raw Event input
            for(int j=0; j<4; ++j)
            {
                // det board is called inside for loop in case different channels are plugged into differnet MSI-8s
                UInt_t dBoardID = (fPar->fDSSDMap[j] >> 4);               // bitwise shift to just select board location
                TPLEIADESFebBoard *dBoard = RawEvent->GetBoard(dBoardID);   // get board from input event with board location

                TPLEIADESDetChan *theDetChan = theDetector->GetChannel(j);
                TPLEIADESChanDisplay *chanDisplay = detDisplay->GetChanDisplay(theDetChan->GetName());

                rawChPos = (theDetChan->GetChanMap() & 0x00F);              // bitwise AND to select last bits where channel location is
                TPLEIADESFebChannel *theRawChan = dBoard->GetChannel(rawChPos);

                // load hit multiplicity information
                if(fOutEvent->fPhysTrigger) { chanDisplay->hHitMultiplicity->Fill(-1); }
                theDetChan->fDPolarity = theRawChan->fRPolarity;
                theDetChan->fDHitMultiplicity = theRawChan->fRHitMultiplicity;
                chanDisplay->hHitMultiplicity->Fill(theDetChan->fDHitMultiplicity);
                if(theDetChan->fDHitMultiplicity > 0) { detDisplay->hDetHitPattern->Fill(j); }

                // load energy information
                theDetChan->fDFPGAEnergy      = theRawChan->fRFPGAEnergy;
                theDetChan->fDFPGAHitTime     = theRawChan->fRFPGAHitTime;
                theDetChan->fDFPGATRAPEZ      = theRawChan->fRFPGATRAPEZ;

                // load trace information
                #ifdef TPLEIADES_FILL_TRACES
                theDetChan->fDTrapezEnergy    = theRawChan->fRTrapezEnergy;
                theDetChan->fDTrace           = theRawChan->fRTrace;
                theDetChan->fDTraceBLR        = theRawChan->fRTraceBLR;
                theDetChan->fDTraceTRAPEZ     = theRawChan->fRTraceTRAPEZ;
                chanDisplay->FillTraces();
                #endif
            }
        }
        else if((theDetector->GetDetType()) == "Crystal")
        {
            if((theDetector->getNElements()) != 2)
            {
                TGo4Log::Warn("Detector %s is a Crystal but does not have 2 elements. Detector is setup incorrectly.", dname.Data());
                return kFALSE;
            }

            // load the 2 crystal channels from the Raw Event input
            for(int j=0; j<2; ++j)
            {
                // det board is called inside for loop in case different channels are plugged into differnet MSI-8s
                UInt_t cBoardID = (fPar->fCrystalMap[j] >> 4);               // bitwise shift to just select board location
                TPLEIADESFebBoard *cBoard = RawEvent->GetBoard(cBoardID);   // get board from input event with board location

                TPLEIADESDetChan *theDetChan = theDetector->GetChannel(j);
                TPLEIADESChanDisplay *chanDisplay = detDisplay->GetChanDisplay(theDetChan->GetName());

                rawChPos = (theDetChan->GetChanMap() & 0x00F);              // bitwise AND to select last bits where channel location is
                TPLEIADESFebChannel *theRawChan = cBoard->GetChannel(rawChPos);

                // load hit multiplicity information
                if(fOutEvent->fPhysTrigger) { chanDisplay->hHitMultiplicity->Fill(-1); }
                theDetChan->fDPolarity = theRawChan->fRPolarity;
                theDetChan->fDHitMultiplicity = theRawChan->fRHitMultiplicity;
                chanDisplay->hHitMultiplicity->Fill(theDetChan->fDHitMultiplicity);
                if(theDetChan->fDHitMultiplicity > 0) { detDisplay->hDetHitPattern->Fill(j); }

                // load energy information
                theDetChan->fDFPGAEnergy      = theRawChan->fRFPGAEnergy;
                theDetChan->fDFPGAHitTime     = theRawChan->fRFPGAHitTime;
                theDetChan->fDFPGATRAPEZ      = theRawChan->fRFPGATRAPEZ;

                // load trace information
                #ifdef TPLEIADES_FILL_TRACES
                theDetChan->fDTrapezEnergy    = theRawChan->fRTrapezEnergy;
                theDetChan->fDTrace           = theRawChan->fRTrace;
                theDetChan->fDTraceBLR        = theRawChan->fRTraceBLR;
                theDetChan->fDTraceTRAPEZ     = theRawChan->fRTraceTRAPEZ;
                chanDisplay->FillTraces();
                #endif
            }
        }
        else
        {
            TGo4Log::Warn("Detector %s does not have a recognised detector type, and thus can't be filled.", dname.Data());
            return kFALSE;
        }
    }

    //std::cout << "What is validiaty of event: " << isValid << "???" << std::endl;
    fOutEvent->SetValid(isValid);     // now event is filled, store event

    return isValid;
}

//----------------------------END OF GO4 SOURCE FILE ---------------------
