// JAM feel free to modify this setup manually :)
void set_PLEIADESParam()
{
#ifndef __GO4ANAMACRO__
   std::cout << "Macro set_PLEIADESRawParam can execute only in analysis" << std::endl;
   return;
#endif
   TPLEIADESParam* param0 = (TPLEIADESParam*) go4->GetParameter("PLEIADESParam","TPLEIADESParam");

   if (param0==0) {
      TGo4Log::Error("Could not find parameter PLEIADESParam of class TPLEIADESParam");
      return;
   }

   TGo4Log::Info("Set parameter PLEIADESParam from script!");


   // we do not override this here, want to use gui setup from parameter editor
   //param0->fSlowMotion = kFALSE; // stop go4 analysis after each event if

   // PLEASE ADD new members of parameter object here!
   param0->fBoardID[1][0] = 10;
   param0->fBoardID[1][1] = 11;
   param0->fBoardID[1][2] = 12;
   param0->fBoardID[1][3] = 13;
   param0->fBoardID[1][4] = 14;
}
