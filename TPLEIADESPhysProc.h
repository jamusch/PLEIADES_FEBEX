//------------------------------------------------------------------------
//*********************** TPLEIADESPhysProc.h ****************************
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

#ifndef TPLEIADESPHYSPROC_H
#define TPLEIADESPHYSPROC_H

#include "TGo4EventProcessor.h"
#include "TPLEIADESDetEvent.h"
#include "TPLEIADESPhysEvent.h"
#include "TPLEIADESParam.h"
#include "TPLEIADESDisplay.h"

class TPLEIADESParam;
class TPLEIADESPhysDisplay;

class TPLEIADESPhysProc : public TGo4EventProcessor
{
    public:
        TPLEIADESPhysProc(const char* name);
        TPLEIADESPhysProc() ;
        virtual ~TPLEIADESPhysProc() ;

        Bool_t BuildEvent(TGo4EventElement* target); // event processing function
        std::vector<Short_t> pStripSelect(TPLEIADESDetector* theDetector);   // provides vector of p-side active strips

        void PulseShapeIntegration(TPLEIADESDetChan *theDetChan, TPLEIADESDetPhysics *detPhysics, TString side);

        void FillClipStatsHists();      // fills clipping statistics histograms
        void FillTOThreshHists();       // fills pulse height reconstruction histograms
        void ExpFitPHRecon();           // implements a pulse shape fitting method
        void ExpIntegPHRecon();         // implements exponential integral method

    private:
        /** parameter for runtime settings*/
        TPLEIADESParam* fPar;

        /** reference to output data*/
        TPLEIADESDetEvent*  fInEvent;   // input event for processing
        TPLEIADESPhysEvent* fOutEvent;  // output event with processed values

        TPLEIADESPhysDisplay* fPhysDisplay;

    ClassDef(TPLEIADESPhysProc, 1)
};

#endif // TPLEIADESPHYSPROC_H

//----------------------------END OF GO4 HEADER FILE ---------------------
