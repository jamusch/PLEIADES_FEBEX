#define MyTreeAnalysis_cxx
#include "MyTreeAnalysis.h"
#include <TH1D.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TSystem.h>

void MyTreeAnalysis::Loop()
{
//   In a ROOT session, you can do:
//      root> .L MyTreeAnalysis.C
//      root> MyTreeAnalysis t
//      root> t.GetEntry(12); // Fill t data members with entry number 12
//      root> t.Show();       // Show values of entry 12
//      root> t.Show(16);     // Read and show values of entry 16
//      root> t.Loop();       // Loop on all entries
//

//     This is the loop skeleton where:
//    jentry is the global entry number in the chain
//    ientry is the entry number in the current Tree
//  Note that the argument to GetEntry must be:
//    jentry for TChain::GetEntry
//    ientry for TTree::GetEntry and TBranch::GetEntry
//
//       To read only selected branches, Insert statements like:
// METHOD1:
//    fChain->SetBranchStatus("*",0);  // disable all branches
//    fChain->SetBranchStatus("branchname",1);  // activate branchname
// METHOD2: replace line
//    fChain->GetEntry(jentry);       //read all branches
//by  b_branchname->GetEntry(ientry); //read only this branch
   if (fChain == 0) return;




   Long64_t nentries = fChain->GetEntriesFast();
   std::cout<< "Loop finds " << nentries<<" entries"<< std::endl;
   // debug
   if(nentries>20) nentries=20; // JAM DEBUG: just show the first events
   // debug
   Long64_t nbytes = 0, nb = 0;
   for (Long64_t jentry=0; jentry<nentries;jentry++) {
      Long64_t ientry = LoadTree(jentry);
      if (ientry < 0) break;
      nb = fChain->GetEntry(jentry);   nbytes += nb;
      // if (Cut(ientry) < 0) continue;
      for(Int_t c=0; c<16; ++c)
      {
        fTrace[c]->Reset("");
        std::vector<Double_t> & theTrace = PLEIADESRawEvent_fTrace[1][1][c];
        for (size_t bin = 0; bin < theTrace.size(); bin++)
             {

                //fTrace[c]->Fill(bin, theTrace[bin]);
                fTrace[c]->SetBinContent(bin+1, theTrace[bin]);
                //std::cout<< " - filling: "<<bin<<", "<< theTrace[bin] << std::endl;
             }
       }
      //std::cout<< "Processed entry: " << jentry<< std::endl;
      fCanvas->Update();
      fCanvas->Draw();
      gSystem->Sleep(200);
   }
   std::cout<< "Loop has processed " << nentries<<" entries. The end"<< std::endl;

   fCanvas->Draw();
 }

