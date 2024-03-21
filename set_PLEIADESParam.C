// JAM feel free to modify this setup manually :)
void set_PLEIADESParam()
{
#ifndef __GO4ANAMACRO__
    std::cout << "Macro set_PLEIADESParam can execute only in analysis" << std::endl;
    return;
#endif
    TPLEIADESParam* param0 = (TPLEIADESParam*) go4->GetParameter("PLEIADESParam","TPLEIADESParam");

    if(param0==0)
    {
        TGo4Log::Error("Could not find parameter PLEIADESParam of class TPLEIADESParam");
        return;
    }

    TGo4Log::Info("Set parameter PLEIADESParam from script!");

    // we do not override this here, want to use gui setup from parameter editor
    //param0->fSlowMotion = kFALSE; // stop go4 analysis after each event if

    //------------------------------------------------------------------------
    // set FEBEX board IDs here. Only define boards if they really exist.
    // code for board unique IDs is SFP + Board slot. So SFP 2, Board slot 3 has unique ID 23.
    //------------------------------------------------------------------------
    param0->fBoardID[1][0] = 10;
    param0->fBoardID[1][1] = 11;
    param0->fBoardID[1][2] = 12;
    param0->fBoardID[1][3] = 13;
    param0->fBoardID[1][4] = 14;

    //------------------------------------------------------------------------
    // set Detector IDs here. NB: using operator[] overwrites map entry. Ensure no duplicate keys (ie det names)!
    //------------------------------------------------------------------------
    // For fpSideMap, code is: first 2 digits = fBoardID, third digit: 1 = ch 0--7; 2 = ch 8--15.
    // For fnSideMap, fDSSDPos, and fCrystalPos code is: first 2 digits = fBoardID, 3rd+4th digits = channel number (starting at 0)
    //----- Slot 1 -----
    TString s1name = "slot1-MSPad_17A";
    param0->detNameVec.pushback(s1name);    // add detector to named list
    param0->fDetTypeMap[s1name] = "SiPad";  // define detector type
    param0->fpSideMap[s1name] = 101;        // slot 1 p-side goes to card 0, ch 1-7
    param0->fnSideMap[s1name] = 1400;       // slot 1 n-side goes to card 4, ch 0
    //----- Slot 2 -----
    TString s2name = "slot2-DSSD_C161054";
    param0->detNameVec.pushback(s2name);    // add detector to named list
    param0->fDetTypeMap[s2name] = "DSSD";   // define detector type
    param0->fDSSDPos[0] = 1408;             // pin1 goes to card 4, ch 8
    param0->fDSSDPos[1] = 1409;             // pin2 goes to card 4, ch 9
    param0->fDSSDPos[2] = 1410;             // pin6 goes to card 4, ch 10
    param0->fDSSDPos[3] = 1411;             // pin7 goes to card 4, ch 11
    //----- Slot 3 -----
    TString s3name = "slot3-MSPad_17B";
    param0->detNameVec.pushback(s3name);    // add detector to named list
    param0->fDetTypeMap[s3name] = "SiPad";  // define detector type
    param0->fpSideMap[s3name] = 102;        // slot 3 p-side goes to card 0, ch 8-15
    param0->fnSideMap[s3name] = 1401;       // slot 3 n-side goes to card 4, ch 1
    //----- Slot 4 -----
    TString s4name = "slot4-MSPad_17C";
    param0->detNameVec.pushback(s4name);    // add detector to named list
    param0->fDetTypeMap[s4name] = "SiPad";  // define detector type
    param0->fpSideMap[s4name] = 111;        // slot 4 p-side goes to card 1, ch 1-7
    param0->fnSideMap[s4name] = 1402;       // slot 4 n-side goes to card 4, ch 2
    //----- Slot 5 -----
    TString s5name = "slot5-MSPad_16A";
    param0->detNameVec.pushback(s5name);    // add detector to named list
    param0->fpSideMap[s5name] = 121;        // slot 5 p-side goes to card 2, ch 1-7 (WEIRD don't ask me)
    param0->fnSideMap[s5name] = 1403;       // slot 5 n-side goes to card 4, ch 3
    //----- Slot 6 -----
    TString s6name = "slot6-MSPad_16B";
    param0->detNameVec.pushback(s6name);    // add detector to named list
    param0->fDetTypeMap[s6name] = "SiPad";  // define detector type
    param0->fpSideMap[s6name] = 112;        // slot 6 p-side goes to card 1, ch 8-15 (ahhhh)
    param0->fnSideMap[s6name] = 1404;       // slot 6 n-side goes to card 4, ch 4
    //----- Slot 7 -----
    TString s7name = "slot7-MSPad_16C";
    param0->detNameVec.pushback(s7name);    // add detector to named list
    param0->fDetTypeMap[s7name] = "SiPad";  // define detector type
    param0->fpSideMap[s7name] = 131 = ;     // slot 7 p-side goes to card 3, ch 1-7 (who tf wired this)
    param0->fnSideMap[s7name] = 1405;       // slot 7 n-side goes to card 4, ch 5
    //----- Slot 8 -----
    /* EMPTY SLOT!!!    TString s8name = "slot8-EMPTY";
    param0->detNameVec.pushback(s8name);    // add detector to named list
    param0->fDetTypeMap[s8name] = "SiPad";  // define detector type
    param0->fpSideMap[s8name] = 122;        // slot 8 p-side goes to card 3, ch 8-15
    param0->fnSideMap[s8name] = 1406;       // slot 8 n-side goes to card 4, ch 6    */
    //----- Slot 9 -----
    /* EMPTY SLOT!!!    TString s9name = "slot9-EMPTY";
    param0->fDetTypeMap[s9name] = "SiPad";  // define detector type
    param0->detNameVec.pushback(s9name);    // add detector to named list
    param0->fpSideMap[s9name] = 132;        // slot 8 p-side goes to card 2, ch 8-15
    param0->fnSideMap[s9name] = 1407;       // slot 9 n-side goes to card 4, ch 7    */
    //----- Slot BP -----
    TString sBPname = "slotBP-Crys_GAGG";
    param0->detNameVec.pushback(sBPname);   // add detector to named list
    param0->fDetTypeMap[sBPname] = "Crystal"; // define detector type
    param0->fCrystalPos[0] = 1414;          // BP pin1 goes to card 4, ch 14
    param0->fCrystalPos[1] = 1415;          // BP pin2 goes to card 4, ch 15

}
