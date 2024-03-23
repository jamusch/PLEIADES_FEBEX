//------------------------------------------------------------------------
//************************** TPLEIADESParam.h ****************************
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

#ifndef TPLEIADESPARAM_H
#define TPLEIADESPARAM_H

#include <map>
#include <vector>
#include "TGo4Parameter.h"
#include "TPLEIADESRawEvent.h"

class TPLEIADESParam : public TGo4Parameter
{
    public:
        TPLEIADESParam();
        TPLEIADESParam(const char* name=0);
        virtual ~TPLEIADESParam();

        virtual Bool_t UpdateFrom(TGo4Parameter *);

        Bool_t fSlowMotion; // if true than only process one MBS event and stop.

        // parameters for step1 - unpacking to FEBEX board format
        Int_t fBoardID[MAX_SFP][MAX_SLAVE];     // boardID to handle multiple SFP chains
        void InitBoardMapping();                // initialise to empty mapping
        Bool_t SetConfigBoards();               // copy board IDs to event members

        // functions for connecting fPar pointers
        Bool_t SetConfigDetEvent();

        // parameters for step2 - grouping by Detectors
        std::vector<TString> fDetNameVec;       // list of detector names
        std::map<TString, TString> fDetTypeMap; // a map of names to detector types
        std::map<TString, UInt_t> fpSideMap;    // a map of names to Si Pad p-side positions
        std::map<TString, UInt_t> fnSideMap;    // a map of names to Si Pad n-side positions
        UInt_t fDSSDMap[4];                     // a list of DSSD channel positions
        UInt_t fCrystalMap[2];                  // a list of Crystal channel positions

    ClassDef(TPLEIADESParam,1)
};

#endif //TPLEIADESPARAM_H

//----------------------------END OF GO4 SOURCE FILE ---------------------
