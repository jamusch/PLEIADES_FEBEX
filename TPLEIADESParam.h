//-------------------------------------------------------------
//        Go4 Release Package v3.03-05 (build 30305)
//                      05-June-2008
//---------------------------------------------------------------
//   The GSI Online Offline Object Oriented (Go4) Project
//   Experiment Data Processing at EE department, GSI
//---------------------------------------------------------------
//
//Copyright (C) 2000- Gesellschaft f. Schwerionenforschung, GSI
//                    Planckstr. 1, 64291 Darmstadt, Germany
//Contact:            http://go4.gsi.de
//----------------------------------------------------------------
//This software can be used under the license agreements as stated
//in Go4License.txt file which is part of the distribution.
//----------------------------------------------------------------
#ifndef SPAR_H
#define SPAR_H


#include "TGo4Parameter.h"

class TPLEIADESParam : public TGo4Parameter {
   public:
      TPLEIADESParam();
      TPLEIADESParam(const char* name);
      virtual ~TPLEIADESParam();
      virtual Bool_t UpdateFrom(TGo4Parameter *);

      Bool_t fSlowMotion; // if true than only process one MBS event and stop.


   ClassDef(TPLEIADESParam,1)
};

#endif //SPAR_H

//----------------------------END OF GO4 SOURCE FILE ---------------------
