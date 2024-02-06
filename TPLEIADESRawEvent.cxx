// $Id$
//-----------------------------------------------------------------------
//       The GSI Online Offline Object Oriented (Go4) Project
//         Experiment Data Processing at EE department, GSI
//-----------------------------------------------------------------------
// Copyright (C) 2000- GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
//                     Planckstr. 1, 64291 Darmstadt, Germany
// Contact:            http://go4.gsi.de
//-----------------------------------------------------------------------
// This software can be used under the license agreements as stated
// in Go4License.txt file which is part of the distribution.
//-----------------------------------------------------------------------

#include "TPLEIADESRawEvent.h"

#include "TGo4Log.h"

//***********************************************************
TPLEIADESRawEvent::TPLEIADESRawEvent() :
   TGo4EventElement()
{
   TGo4Log::Info("TPLEIADESRawEvent: Create instance");
}
//***********************************************************
TPLEIADESRawEvent::TPLEIADESRawEvent(const char *name) :
   TGo4EventElement(name)
{
   TGo4Log::Info("TPLEIADESRawEvent: Create instance %s", name);
}
//***********************************************************
TPLEIADESRawEvent::~TPLEIADESRawEvent()
{
   TGo4Log::Info("TPLEIADESRawEvent: Delete instance");
}

//-----------------------------------------------------------
void TPLEIADESRawEvent::Clear(Option_t *)
{
  // all members should be cleared, i.e. assigned to a "not filled" value

  fSequenceNumber=-1;

  for (int i = 0; i < MAX_SFP; i++)
    {
      for (int j = 0; j < MAX_SLAVE; j++)
      {
        for (int k = 0; k < N_CHA; k++)
        {
          fE_FPGA_Trapez[i][j][k]=0;

#ifdef TPLEIADES_FILL_TRACES
          fTrace[i][j][k].clear();
          fTraceBLR[i][j][k].clear();
          fTrapezFPGA[i][j][k].clear();
#endif

        }
      }
    }


}
