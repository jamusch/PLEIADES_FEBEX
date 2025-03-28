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
    param0->fSlowMotion = kFALSE; // stop go4 analysis after each event if

    //------------------------------------------------------------------------
    // set FEBEX board IDs here. Only define boards if they really exist.
    // code for Board IDs is 8 bit word: bit 5,4 are the SFP, bit 3-0 are the Board slots in crate.
    // e.g. i plug a Board into SFP 2 at slot 9 (0 being first slot). the position code is 0x28.

    // JAM28-03-25: first index is kinpex crate number now [0=0x100, 1=0x200
    //------------------------------------------------------------------------
    param0->fBoardID[0][1][0] = 0x10;
    param0->fBoardID[0][1][1] = 0x11;
    param0->fBoardID[0][1][2] = 0x12;
    param0->fBoardID[0][1][3] = 0x13;
    param0->fBoardID[0][1][4] = 0x14;

    param0->fBoardID[1][1][0] = 0x00;
    param0->fBoardID[1][1][1] = 0x01;
    param0->fBoardID[1][1][2] = 0x02;
    param0->fBoardID[1][1][3] = 0x03;

    //------------------------------------------------------------------------
    // set Detector IDs here. NB: using operator[] overwrites map entry. Ensure no duplicate keys (ie det names)!
    //------------------------------------------------------------------------
    // code for channel map is a 12 bit word: bit 9,8 are for SFP, bit 7-4 are for Board slot, bit 3-0 are for channel position.
    // for fpSideMap, beacuse p-sides are plugged in via 16-ch ribbon cables (and thus must come in order), only first pos is listed.

    param0->fDetNameVec.clear(); // JAM 27-06-2024: remove previous names, otherwise we duplicate the elements for each submit...
    //----- Slot 1 -----
    TString s1name = "slot1_MSPad_17A";
    param0->fDetNameVec.push_back(s1name);  // add detector to named list
    param0->fDetTypeMap[s1name] = "SiPad";  // define detector type
    param0->fpSideMap[s1name] = 0x100;      // slot 1 p-side goes to card 0, ch 0-6
    param0->fnSideMap[s1name] = 0x140;      // slot 1 n-side goes to card 4, ch 0
    //----- Slot 2 -----
//    TString s2name = "slot2_DSSD_C161054";
    TString s2name = "slot2_DSSD_SFP1";
    param0->fDetNameVec.push_back(s2name);  // add detector to named list
    param0->fDetTypeMap[s2name] = "DSSD";   // define detector type
    // for DSSD, make sure p-sides (ie L/R) go to pos 0/1 in fDSSDMap
    param0->fDSSDMap[s2name][0] = 0x148;            // pin1 goes to card 4, ch 8
    param0->fDSSDMap[s2name][1] = 0x149;            // pin2 goes to card 4, ch 9
    param0->fDSSDMap[s2name][2] = 0x14A;            // pin6 goes to card 4, ch 10
    param0->fDSSDMap[s2name][3] = 0x14B;            // pin7 goes to card 4, ch 11
    //----- Slot 3 -----
    TString s3name = "slot3_MSPad_17B";
    param0->fDetNameVec.push_back(s3name);  // add detector to named list
    param0->fDetTypeMap[s3name] = "SiPad";  // define detector type
    param0->fpSideMap[s3name] = 0x108;      // slot 3 p-side goes to card 0, ch 8-15
    param0->fnSideMap[s3name] = 0x141;      // slot 3 n-side goes to card 4, ch 1
    //----- Slot 4 -----
    TString s4name = "slot4_MSPad_17C";
    param0->fDetNameVec.push_back(s4name);  // add detector to named list
    param0->fDetTypeMap[s4name] = "SiPad";  // define detector type
    param0->fpSideMap[s4name] = 0x110;      // slot 4 p-side goes to card 1, ch 0-6
    param0->fnSideMap[s4name] = 0x142;      // slot 4 n-side goes to card 4, ch 2
    //----- Slot 5 -----
    TString s5name = "slot5_MSPad_16A";
    param0->fDetNameVec.push_back(s5name);  // add detector to named list
    param0->fDetTypeMap[s5name] = "SiPad";  // define detector type
    param0->fpSideMap[s5name] = 0x121;      // slot 5 p-side goes to card 2, ch 0-6 (WEIRD don't ask me)
    param0->fnSideMap[s5name] = 0x143;      // slot 5 n-side goes to card 4, ch 3
    //----- Slot 6 -----
    TString s6name = "slot6_MSPad_16B";
    param0->fDetNameVec.push_back(s6name);  // add detector to named list
    param0->fDetTypeMap[s6name] = "SiPad";  // define detector type
    param0->fpSideMap[s6name] = 0x118;      // slot 6 p-side goes to card 1, ch 8-15 (ahhhh)
    param0->fnSideMap[s6name] = 0x144;      // slot 6 n-side goes to card 4, ch 4
    //----- Slot 7 -----
    TString s7name = "slot7_MSPad_16C";
    param0->fDetNameVec.push_back(s7name);  // add detector to named list
    param0->fDetTypeMap[s7name] = "SiPad";  // define detector type
    param0->fpSideMap[s7name] = 0x130;      // slot 7 p-side goes to card 3, ch 0-6 (who tf wired this)
    param0->fnSideMap[s7name] = 0x145;      // slot 7 n-side goes to card 4, ch 5
    //----- Slot 8 -----
    /* EMPTY SLOT!!!    TString s8name = "slot8_EMPTY";
    param0->fDetNameVec.push_back(s8name);  // add detector to named list
    param0->fDetTypeMap[s8name] = "SiPad";  // define detector type
    param0->fpSideMap[s8name] = 0x138;      // slot 8 p-side goes to card 3, ch 8-15
    param0->fnSideMap[s8name] = 0x146;      // slot 8 n-side goes to card 4, ch 6    */
    //----- Slot 9 -----
    /* EMPTY SLOT!!!    TString s9name = "slot9_EMPTY";
    param0->fDetNameVec.push_back(s9name);  // add detector to named list
    param0->fDetTypeMap[s9name] = "SiPad";  // define detector type
    param0->fpSideMap[s9name] = 0x128;      // slot 8 p-side goes to card 2, ch 8-15
    param0->fnSideMap[s9name] = 0x147;      // slot 9 n-side goes to card 4, ch 7    */
    //----- Slot BP -----
//    TString sBPname = "slotBP_Crys_SFP1";
    TString sBPname = "slotBP_Crys_BGO";
    param0->fDetNameVec.push_back(sBPname); // add detector to named list
    param0->fDetTypeMap[sBPname] = "Crystal"; // define detector type
    // for Crystal, front should go to 0, back to 1
    param0->fCrystalMap[sBPname][0] = 0x14E;         // BP pin1 goes to card 4, ch 12
    param0->fCrystalMap[sBPname][1] = 0x14F;         // BP pin2 goes to card 4, ch 13

    // Detector number 2 ---> kinpex 1

     
     //----- Slot 1 -----
     TString s1name2 = "slot1_DesignI-14";
     param0->fDetNameVec.push_back(s1name2);  // add detector to named list
     param0->fDetTypeMap[s1name2] = "SiPad";  // define detector type
     param0->fpSideMap[s1name2] = 0x000;      // slot 1 p-side goes to card 0, ch 0-6
     param0->fnSideMap[s1name2] = 0x030;      // slot 1 n-side goes to card 3, ch 0
     //----- Slot 2 -----
     TString s2name2 = "slot2_DSSD_SFP0";
     param0->fDetNameVec.push_back(s2name2);  // add detector to named list
     param0->fDetTypeMap[s2name2] = "DSSD";   // define detector type
     // for DSSD, make sure p-sides (ie L/R) go to pos 0/1 in fDSSDMap
     param0->fDSSDMap[s2name2][0] = 0x038;            // pin1 goes to card 3, ch 8
     param0->fDSSDMap[s2name2][1] = 0x039;            // pin2 goes to card 3, ch 9
     param0->fDSSDMap[s2name2][2] = 0x03A;            // pin6 goes to card 3, ch 10
     param0->fDSSDMap[s2name2][3] = 0x03B;            // pin7 goes to card 3, ch 11
     //----- Slot 3 -----
     TString s3name2 = "slot3_MSPad_15B";
     param0->fDetNameVec.push_back(s3name2);  // add detector to named list
     param0->fDetTypeMap[s3name2] = "SiPad";  // define detector type
     param0->fpSideMap[s3name2] = 0x008;      // slot 3 p-side goes to card 0, ch 8-15
     param0->fnSideMap[s3name2] = 0x031;      // slot 3 n-side goes to card 3, ch 1
     //----- Slot 4 -----
     TString s4name2 = "slot4_DesignI-13";
     param0->fDetNameVec.push_back(s4name2);  // add detector to named list
     param0->fDetTypeMap[s4name2] = "SiPad";  // define detector type
     param0->fpSideMap[s4name2] = 0x010;      // slot 4 p-side goes to card 1, ch 0-6
     param0->fnSideMap[s4name2] = 0x032;      // slot 4 n-side goes to card 3, ch 2
     //----- Slot 5 -----
     TString s5name2 = "slot5_DesignI-8";
     param0->fDetNameVec.push_back(s5name2);  // add detector to named list
     param0->fDetTypeMap[s5name2] = "SiPad";  // define detector type
     param0->fpSideMap[s5name2] = 0x020;      // slot 5 p-side goes to card 2, ch 0-6 (WEIRD don't ask me)
     param0->fnSideMap[s5name2] = 0x033;      // slot 5 n-side goes to card 3, ch 3
     //----- Slot 6 -----
     TString s6name2 = "slot6_DesignI-6";
     param0->fDetNameVec.push_back(s6name2);  // add detector to named list
     param0->fDetTypeMap[s6name2] = "SiPad";  // define detector type
     param0->fpSideMap[s6name2] = 0x018;      // slot 6 p-side goes to card 1, ch 8-15 (ahhhh)
     param0->fnSideMap[s6name2] = 0x034;      // slot 6 n-side goes to card 3, ch 4
     //----- Slot 7 -----
       /* EMPTY SLOT!!!   TString s7name2 = "slot7_MSPad_16C";
       param0->fDetNameVec.push_back(s7name2);  // add detector to named list
       param0->fDetTypeMap[s7name2] = "SiPad";  // define detector type
       param0->fpSideMap[s7name2] = 0x030;      // slot 7 p-side goes to card 3, ch 0-6 (who tf wired this)
       param0->fnSideMap[s7name2] = 0x035;      // slot 7 n-side goes to card 3, ch 5
       */
       //----- Slot 8 -----
       TString s8name2 = "slot8_DesignI-3";
       param0->fDetNameVec.push_back(s8name2);  // add detector to named list
       param0->fDetTypeMap[s8name2] = "SiPad";  // define detector type
       param0->fpSideMap[s8name2] = 0x028;      // slot 8 p-side goes to card 2, ch 8-15
       param0->fnSideMap[s8name2] = 0x035;      // slot 8 n-side goes to card 3, ch 6
       //----- Slot 9 -----
       /* EMPTY SLOT!!!    TString s9name2 = "slot9_EMPTY";
       param0->fDetNameVec.push_back(s9name2);  // add detector to named list
       param0->fDetTypeMap[s9name2] = "SiPad";  // define detector type
       param0->fpSideMap[s9name2] = 0x028;      // slot 8 p-side goes to card 3, ch 8-15
       param0->fnSideMap[s9name2] = 0x037;      // slot 9 n-side goes to card 3, ch 7
       */
       //----- Slot BP -----
       TString sBPname2 = "slotBP_Crys_SFP0";
       param0->fDetNameVec.push_back(sBPname2); // add detector to named list
       param0->fDetTypeMap[sBPname2] = "Crystal"; // define detector type
       // for Crystal, front should go to 0, back to 1
       param0->fCrystalMap[sBPname2][0] = 0x03E;         // BP pin1 goes to card 3, ch 12
       param0->fCrystalMap[sBPname2][1] = 0x03F;         // BP pin2 goes to card 3, ch 13

}
