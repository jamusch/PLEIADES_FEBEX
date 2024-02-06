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

#ifndef TPLEIADESRawEvent_H
#define TPLEIADESRawEvent_H


// disable this define to speed up analysis, only histogram filling then
#define TPLEIADES_FILL_TRACES 1

#include "TGo4EventElement.h"


#define MAX_SFP          4
#define MAX_SLAVE        4
#define N_CHA            16


class TPLEIADESRawEvent : public TGo4EventElement {
   public:
      TPLEIADESRawEvent();
      TPLEIADESRawEvent(const char *name);
      virtual ~TPLEIADESRawEvent();

      /** Method called by the framework to clear the event element. */
      void Clear(Option_t *opt = "") override;

      /** Event sequence number incremented by MBS Trigger*/
       Int_t fSequenceNumber;

      /** Example: put corrected Energy from filter for each channel here */
      Double_t fE_FPGA_Trapez[MAX_SFP][MAX_SLAVE][N_CHA];

#ifdef TPLEIADES_FILL_TRACES
      std::vector<Double_t> fTrace[MAX_SFP][MAX_SLAVE][N_CHA];
      std::vector<Double_t> fTraceBLR[MAX_SFP][MAX_SLAVE][N_CHA];
      std::vector<Double_t> fTrapezFPGA[MAX_SFP][MAX_SLAVE][N_CHA];
#endif

   ClassDefOverride(TPLEIADESRawEvent,1)
};

#endif
