//------------------------------------------------------------------------
//************************ TPLEIADESDetProc.h ****************************
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

#ifndef TPLEIADESDETPROC_H
#define TPLEIADESDETPROC_H

#include "TGo4EventProcessor.h"
#include "TPLEIADESDetEvent.h"
#include "TPLEIADESParam.h"
#include "TPLEIADESDisplay.h"

#include <vector>

class TPLEIADESParam;
class TPLEIADESDetDisplay;

class TPLEIADESDetProc : public TGo4EventProcessor
{
    public:
        TPLEIADESDetProc();
        TPLEIADESDetProc(const char* name);
        virtual ~TPLEIADESDetProc() ;

        Bool_t BuildEvent(TGo4EventElement* target); // event processing function

        void InitDisplays(TPLEIADESDetEvent* out);  // init displays for event based on detectors in event

    private:
        /** parameter for runtime settings*/
        TPLEIADESParam* fPar;

        /** reference to output data*/
        TPLEIADESDetEvent* fOutEvent;  //!

        std::vector<TPLEIADESDetDisplay*> fDetDisplays;

    ClassDef(TPLEIADESDetProc, 1)
};

#endif // TPLEIADESDETPROC_H

//----------------------------END OF GO4 HEADER FILE ---------------------
