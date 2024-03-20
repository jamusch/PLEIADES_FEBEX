//------------------------------------------------------------------------
//************************** TPLEIADESParam.cxx **************************
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

#include "TPLEIADESRawEvent.h"

#include "TGo4Log.h"

//------------------------------------------------------------------------
// this is the lowest class for the individual FEBEX channel readouts
//------------------------------------------------------------------------

TPLEIADESFebChannel::TPLEIADESFebChannel() :
    TGo4EventElement()
{
    TGo4Log::Info("TPLEIADESFebChannel: Create instance");
}

TPLEIADESFebChannel::TPLEIADESFebChannel(const char *name, Short_t id) :
    TGo4EventElement(name, name, id)
{
    TGo4Log::Info("TPLEIADESFebBoard: Create instance %s", name);
}

TPLEIADESFebChannel::~TPLEIADESFebChannel()
{
    TGo4Log::Info("TPLEIADESFebChannel: Delete instance");
}

void TPLEIADESFebChannel::Clear(Option_t *opt)
{
    // all members should be cleared, i.e. assigned to a "not filled" value
    /** FEBEX special channel properties **/
    fFPGAEnergy = 0;
    fFGPAHitTime = 0;
    fFPGATRAPEZ.clear();

    /** FEBEX trace properties **/
#ifdef TPLEIADES_FILL_TRACES
    fTrace.clear();
    fTraceBLR.clear();
    fTraceTRAPEZ.clear();
#endif
}

//------------------------------------------------------------------------
// this class represents one FEBEX-3 board with 16 Feb Channels
//------------------------------------------------------------------------

TPLEIADESFebBoard::TPLEIADESFebBoard() :
    TGo4CompositeEvent(), fLastEventNumber(-1)
{
    TGo4Log::Info("TPLEIADESFebBoard: Create instance");
}

TPLEIADESFebBoard::TPLEIADESFebBoard(const char *name, UInt_t unid, Short_t id) :
    TGo4CompositeEvent(name, name, id), fUniqueId(unid), fLastEventNumber(-1)
{
    TGo4Log::Info("TPLEIADESFebBoard: Create instance %s with unique ID %d, composite ID %d", name, unid, id);

    //create channels for FEBEX board
    TString modname;
    for (int i=0; i<N_CHA; ++i)
    {
        modname.Form("PLEIADES_Bd%02d_Ch%02d", fUniqueId, i);
        addEventElement(new TPLEIADESFebChannel(modname.Data(), i));
    }
}

TPLEIADESFebBoard::~TPLEIADESFebBoard()
{
    TGo4Log::Info("TPLEIADESFebChannel: Delete instance");
}

void TPLEIADESFebBoard::Clear(Option_t *opt)
{
    TGo4CompositeEvent::Clear();
}

//------------------------------------------------------------------------
// this is the top event structure with all FEBEX boards in the chain
//------------------------------------------------------------------------

std::vector<UInt_t> TPLEIADESRawEvent::fgConfigBoards;

TPLEIADESRawEvent::TPLEIADESRawEvent() :
    TGo4CompositeEvent(), fSequenceNumber(-1)
{
    TGo4Log::Info("TPLEIADESRawEvent: Create instance");
}

TPLEIADESRawEvent::TPLEIADESRawEvent(const char *name, Short_t id) :
    TGo4CompositeEvent(name, name, id), fSequenceNumber(-1)
{
    TGo4Log::Info("TPLEIADESRawEvent: Create instance %s with composite ID %d", name, id);

    //create boards based on fgConfigBoards list
    TString modname;
    UInt_t unid;
    for (unsigned i=0; i<TPLEIADESRawEvent::fgConfigBoards.size(); ++i)
    {
        unid = TPLEIADESRawEvent::fgConfigBoards[i];
        modname.Form("PLEIADES_Board_%02d", unid);
        addEventElement(new TPLEIADESFebBoard(modname.Data(), unid, i));
    }
}

TPLEIADESRawEvent::~TPLEIADESRawEvent()
{
    TGo4Log::Info("TPLEIADESRawEvent: Delete instance");
}

TPLEIADESFebBoard* TPLEIADESRawEvent::GetBoard(UInt_t unid)
{
    TPLEIADESFebBoard* theBoard = 0;
    Short_t numBoards = getNElements();
    for(int i=0; i<numBoards; ++i)
    {
        theBoard = (TPLEIADESFebBoard*) getEventElement(i);
        if(theBoard->GetBoardID() == unid)
        {
            return theBoard;
        }
    }
    return 0;
}

void TPLEIADESRawEvent::Clear(Option_t *opt)
{
    TGo4CompositeEvent::Clear();
    fSequenceNumber = -1;
}

//----------------------------END OF GO4 SOURCE FILE ---------------------
