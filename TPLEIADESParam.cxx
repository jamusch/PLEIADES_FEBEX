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

#include "Riostream.h"
#include "TGo4Log.h"

using namespace std;

TPLEIADESParam::TPLEIADESParam() : TGo4Parameter("Parameter")
{
    fSlowMotion=kFALSE;

    InitBoardMapping();
}

TPLEIADESParam::TPLEIADESParam(const char* name) : TGo4Parameter(name)
{
    fSlowMotion=kFALSE;

    InitBoardMapping();
}

TPLEIADESParam::~TPLEIADESParam()
{
}

//-----------------------------------------------------------------------
// these functions govern the board id mapping. this functionality is so
// boards in multiple SFPs can be written out, if necessary.
//-----------------------------------------------------------------------

void TPLEIADESParam::InitBoardMapping()
{
    for(int sfp=0; sfp<MAX_SFP; ++sfp)
    {
        for(int brd=0; brd<MAX_SLAVE; ++brd)
        {
            fBoardID[sfp][brd] = -1;
        }
    }
}

Bool_t TPLEIADESParam::SetConfigBoards()
{
    TPLEIADESRawEvent::fgConfigBoards.clear();
    for(int sfp=0; sfp<MAX_SFP; ++sfp)
    {
        for(int brd=0; brd<MAX_SLAVE; ++brd)
        {
            Int_t bid = fBoardID[sfp][brd];
            if(bid < 0) continue;
            TPLEIADESRawEvent::fgConfigBoards.push_back(bid);
            TGo4Log::Info("TPLEIADESParam::SetConfigBoards registers board unique ID %u configured at SFP %d Board %d \n", bid, sfp, brd);
        }
    }
    return kTRUE;
}

Bool_t TPLEIADESParam::SetConfigDetectors()
{
    TPLEIADESDetEvent::fgConfigDetectors.clear();
    for(const TString& dname : fDetNameVec)
        TPLEIADESDetEvent::fgConfigDetectors.push_back(dname);
        TGo4Log::Info("TPLEIADESParam::SetConfigDetectors registers a detector with name %s \n", dname);
        }
    }
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
    SetConfigDetectors();
    return kTRUE;
}

//----------------------------END OF GO4 SOURCE FILE ---------------------
