//------------------------------------------------------------------------
//************************ TPLEIADESDetProc.h ***************************
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
#include "TPLEIADESParam.h"

//------------------------------------------------------------------------
TPLEIADESDetProc::TPLEIADESDetProc() : TGo4EventProcessor("Proc")
{
    TGo4Log::Info("TPLEIADESDetProc: Create instance ");
}

//------------------------------------------------------------------------
TPLEIADESDetProc::~TPLEIADESDetProc()
{
    TGo4Log::Info("TPLEIADESDetProc: Delete instance ");
}

//------------------------------------------------------------------------
// this one is used in standard factory
TPLEIADESDetProc::TPLEIADESDetProc(const char* name) : TGo4EventProcessor(name)
{
    TGo4Log::Info("TPLEIADESDetProc: Create instance %s", name);
    fPar = dynamic_cast<TPLEIADESParam*>(MakeParameter("PLEIADESParam", "TPLEIADESParam", "set_PLEIADESParam.C"));
}

//------------------------------------------------------------------------
// Build event is the unpacker function, it reads the MBS event and does stuff with it.
// Histograms are erased with the next word, so are mostly used for online analysis.
Bool_t TPLEIADESDetProc::BuildEvent(TGo4EventElement* target)
{
    fOutEvent = (TPLEIADESDetEvent*) target;
    if(fOutEvent==0) { GO4_STOP_ANALYSIS_MESSAGE("NEVER COME HERE: output event is not configured, wrong class!") }

    fOutEvent->SetValid(kFALSE);    // initialize next output as not filled, i.e. it is only stored when something is in

    TPLEIADESRawEvent *RawEvent = (TPLEIADESRawEvent*) GetInputEvent();

    // loop over detectors to fill data from raw output using mapping
    for(const TString& dname : fPar->fDetNameVec)
    {
        TPLEIADESDetector *theDetector = fOutEvent->GetDetector(dname);
        UInt_t rawChPos;

        if((theDetector->GetDetType()) == "SiPad")
        {
            if( (theDetector->getNElements()) != 8)
            {
                TGo4Log::Warn("Detector %s is a Si Pad but does not have 8 elements. Detector is setup incorrectly.", dname)
                return;
            }

            UInt_t *boardID = (fPar->fpSideMap[dname] >> 4);            // bitwise shift to just select board location
            TPLEIADESFebBoard *detBoard = RawEvent->GetBoard(boardID);  // get board from input event with board location

            for(int j=0; j<nChan; ++j)
            {
                rawChPos = (theDetChan->fUniqChanMap & 0x00F);          // bitwise and to select last bits where channel location is
                TPLEIADESFebChannel *theRawChan = detBoard->GetChannel(rawChPos);

                TPLEIADESDetChan *theDetChan = theDetector->GetChannel(j);      // get detector channel
                theDetChan->fFPGAEnergy     = theRawChan->fFPGAEnergy;
                theDetChan->fFGPAHitTime;   = theRawChan->fFGPAHitTime;
                theDetChan->fFPGATRAPEZ;    = theRawChan->fFPGATRAPEZ;
                #ifdef TPLEIADES_FILL_TRACES
                theDetChan->fTrapezEnergy;  = theRawChan->fTrapezEnergy;
                theDetChan->fTrace;         = theRawChan->fTrace;
                theDetChan->fTraceBLR;      = theRawChan->fTraceBLR;
                theDetChan->fTraceTRAPEZ;   = theRawChan->fTraceTRAPEZ;
                #endif
            }

            //now do things with the n-side...
        }
    }



    fOutEvent->SetValid(kTRUE);     // now event is filled, store event
}

//----------------------------END OF GO4 SOURCE FILE ---------------------
