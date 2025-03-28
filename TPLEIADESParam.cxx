//------------------------------------------------------------------------
//************************* TPLEIADESParam.cxx ***************************
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

#include "TPLEIADESParam.h"
#include "TPLEIADESRawEvent.h"
#include "TPLEIADESDetEvent.h"
#include "TPLEIADESPhysEvent.h"
#include "TPLEIADESDisplay.h"

#include "Riostream.h"
#include "TGo4Log.h"

using namespace std;

TPLEIADESParam::TPLEIADESParam() : TGo4Parameter("Parameter")
{
    fSlowMotion = kFALSE;

    InitBoardMapping();
}

TPLEIADESParam::TPLEIADESParam(const char* name) : TGo4Parameter(name)
{
    fSlowMotion = kFALSE;

    InitBoardMapping();
}

TPLEIADESParam::~TPLEIADESParam()
{

}

// these functions govern the board id mapping. this functionality is so boards in multiple SFPs can be written out, if necessary.

void TPLEIADESParam::InitBoardMapping()
{
  for (int kin = 0; kin < MAX_CRATES; ++kin)
  {
    for (int sfp = 0; sfp < MAX_SFP; ++sfp)
    {
      for (int brd = 0; brd < MAX_SLAVE; ++brd)
      {
        fBoardID[kin][sfp][brd] = -1;
      }
    }
  }
}

Bool_t TPLEIADESParam::SetConfigBoards()
{
    TPLEIADESRawEvent::fgConfigBoards.clear();
    for (int kin = 0; kin < MAX_CRATES; ++kin)
     {
    for(int sfp=0; sfp<MAX_SFP; ++sfp)
    {
        for(int brd=0; brd<MAX_SLAVE; ++brd)
        {
            Int_t bid = fBoardID[kin][sfp][brd];
            if(bid < 0) continue;
            TPLEIADESRawEvent::fgConfigBoards.push_back(bid);
            TGo4Log::Info("TPLEIADESParam::SetConfigBoards registers board unique ID 0x%x configured at kinpex %d, SFP %d Board %d", bid, kin, sfp, brd);
        }
    }
     }
    return kTRUE;
}

// this function connects the fParDet and fParDEv of TPLEIADESDetEvent to the current fPar being used
Bool_t TPLEIADESParam::SetConfigDetEvent()
{
    TPLEIADESDetector::fParDet = this;
    TPLEIADESDetEvent::fParDEv = this;
    return kTRUE;
}

// this function connects the fParPEv of TPLEIADESPhysEvent to the current fPar being used
Bool_t TPLEIADESParam::SetConfigPhysEvent()
{
    TPLEIADESPhysEvent::fParPEv = this;
    return kTRUE;
}

// this function connects the fParDisp of TPLEIADESDisplay to the current fPar being used
Bool_t TPLEIADESParam::SetConfigDisplay()
{
    TPLEIADESDisplay::fParDisplay = this;
    return kTRUE;
}

//-----------------------------------------------------------------------
// this is the update function that interfaces with the Go4 GUI
//-----------------------------------------------------------------------

Bool_t TPLEIADESParam::UpdateFrom(TGo4Parameter *pp)
{
    if(pp->InheritsFrom("TPLEIADESParam"))
    {
        TPLEIADESParam * from;
        from = (TPLEIADESParam *) pp;
        cout << "**** TPLEIADESParam " << GetName() << " updated from auto save file" << endl;
        fSlowMotion=from->fSlowMotion;
        // usually, elemental members up to 1d arrays are copied automatically correctly this function is intended to pass more
        // complex data structures, or to exectue code whenever user applies the parameter from gui
    }
    else // NEVER COME HERE
    {
        cout << "Wrong parameter object: " << pp->ClassName() << endl;
    }

    SetConfigBoards();
    return kTRUE;
}

//----------------------------END OF GO4 SOURCE FILE ---------------------
