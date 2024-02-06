//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Thu Dec 14 14:30:38 2023 by ROOT version 6.28/04
// from TTree AnalysisxTree/Go4FileStore
// found on file: Plejades.root
// JAM 14-Dec-23: modified constructor to take additional filename argument
//////////////////////////////////////////////////////////

#ifndef MyTreeAnalysis_h
#define MyTreeAnalysis_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TH1D.h>
#include <TCanvas.h>
#include "Riostream.h"
// Header file for the classes stored in the TTree if any.
//#include "TPLEIADESRawEvent.h"
//#include "TObject.h"
//#include "TNamed.h"
//#include <TGo4EventElement.h>

class MyTreeAnalysis {
public :
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain
   Int_t           fCurrent; //!current Tree number in a TChain

// Fixed size dimensions of array or collections stored in the TTree if any.
   static constexpr Int_t kMaxPLEIADESRawEvent = 1;

   // Declaration of leaf types
 //TPLEIADESRawEvent *PLEIADESRawEvent_;
   UInt_t          PLEIADESRawEvent_TGo4EventElement_fUniqueID;
   UInt_t          PLEIADESRawEvent_TGo4EventElement_fBits;
   TString         PLEIADESRawEvent_TGo4EventElement_fName;
   TString         PLEIADESRawEvent_TGo4EventElement_fTitle;
   Bool_t          PLEIADESRawEvent_TGo4EventElement_fbIsValid;
   Short_t         PLEIADESRawEvent_TGo4EventElement_fIdentifier;
   Int_t           PLEIADESRawEvent_fSequenceNumber;
   Double_t        PLEIADESRawEvent_fE_FPGA_Trapez[4][4][16];
   vector<double>  PLEIADESRawEvent_fTrace[4][4][16];
   vector<double>  PLEIADESRawEvent_fTraceBLR[4][4][16];
   vector<double>  PLEIADESRawEvent_fTrapezFPGA[4][4][16];

   // List of branches
   TBranch        *b_PLEIADESRawEvent_TGo4EventElement_fUniqueID;   //!
   TBranch        *b_PLEIADESRawEvent_TGo4EventElement_fBits;   //!
   TBranch        *b_PLEIADESRawEvent_TGo4EventElement_fName;   //!
   TBranch        *b_PLEIADESRawEvent_TGo4EventElement_fTitle;   //!
   TBranch        *b_PLEIADESRawEvent_TGo4EventElement_fbIsValid;   //!
   TBranch        *b_PLEIADESRawEvent_TGo4EventElement_fIdentifier;   //!
   TBranch        *b_PLEIADESRawEvent_fSequenceNumber;   //!
   TBranch        *b_PLEIADESRawEvent_fE_FPGA_Trapez;   //!
   TBranch        *b_PLEIADESRawEvent_fTrace;   //!
   TBranch        *b_PLEIADESRawEvent_fTraceBLR;   //!
   TBranch        *b_PLEIADESRawEvent_fTrapezFPGA;   //!

   TH1* fTrace[16];
   TCanvas* fCanvas;

   MyTreeAnalysis(TTree *tree=0, const char* filename=0);
   virtual ~MyTreeAnalysis();
   virtual Int_t    Cut(Long64_t entry);
   virtual Int_t    GetEntry(Long64_t entry);
   virtual Long64_t LoadTree(Long64_t entry);
   virtual void     Init(TTree *tree);
   virtual void     Loop();
   virtual Bool_t   Notify();
   virtual void     Show(Long64_t entry = -1);
};

#endif

#ifdef MyTreeAnalysis_cxx
MyTreeAnalysis::MyTreeAnalysis(TTree *tree, const char* filename) : fChain(0)
{
// if parameter tree is not specified (or zero), connect the file
// used to generate this class and read the Tree.
   if (tree == 0) {
      if(filename==0)
        filename="Plejades.root";
      TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject(filename);
      if (!f || !f->IsOpen()) {
         f = new TFile(filename);
      }
      f->GetObject("AnalysisxTree",tree);

   }
   Init(tree);
   TString obname, obtitle;
   fCanvas=new TCanvas("Traces","Examine traces in vectors",2);
   fCanvas->Divide(4,4);
   fCanvas->Draw();
   for(Int_t i=0; i<16; ++i)
   {
     obname.Form("SampleTrace_%d", i);
     obtitle.Form("Demo trace %d ", i);
      fTrace[i]= new TH1D(obname.Data(), obtitle.Data(), 3000, 0, 3000);
      std::cout<< "created histogram" << obname.Data()<< std::endl;
      fCanvas->cd(i+1);
      fTrace[i]->Draw();
   }



   }

MyTreeAnalysis::~MyTreeAnalysis()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
}

Int_t MyTreeAnalysis::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!fChain) return 0;
   return fChain->GetEntry(entry);
}
Long64_t MyTreeAnalysis::LoadTree(Long64_t entry)
{
// Set the environment to read one entry
   if (!fChain) return -5;
   Long64_t centry = fChain->LoadTree(entry);
   if (centry < 0) return centry;
   if (fChain->GetTreeNumber() != fCurrent) {
      fCurrent = fChain->GetTreeNumber();
      Notify();
   }
   return centry;
}

void MyTreeAnalysis::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   // Set branch addresses and branch pointers
   if (!tree) return;
   fChain = tree;
   fCurrent = -1;
   fChain->SetMakeClass(1);

   fChain->SetBranchAddress("PLEIADESRawEvent.TGo4EventElement.fUniqueID", &PLEIADESRawEvent_TGo4EventElement_fUniqueID, &b_PLEIADESRawEvent_TGo4EventElement_fUniqueID);
   fChain->SetBranchAddress("PLEIADESRawEvent.TGo4EventElement.fBits", &PLEIADESRawEvent_TGo4EventElement_fBits, &b_PLEIADESRawEvent_TGo4EventElement_fBits);
   fChain->SetBranchAddress("PLEIADESRawEvent.TGo4EventElement.fName", &PLEIADESRawEvent_TGo4EventElement_fName, &b_PLEIADESRawEvent_TGo4EventElement_fName);
   fChain->SetBranchAddress("PLEIADESRawEvent.TGo4EventElement.fTitle", &PLEIADESRawEvent_TGo4EventElement_fTitle, &b_PLEIADESRawEvent_TGo4EventElement_fTitle);
   fChain->SetBranchAddress("PLEIADESRawEvent.TGo4EventElement.fbIsValid", &PLEIADESRawEvent_TGo4EventElement_fbIsValid, &b_PLEIADESRawEvent_TGo4EventElement_fbIsValid);
   fChain->SetBranchAddress("PLEIADESRawEvent.TGo4EventElement.fIdentifier", &PLEIADESRawEvent_TGo4EventElement_fIdentifier, &b_PLEIADESRawEvent_TGo4EventElement_fIdentifier);
   fChain->SetBranchAddress("PLEIADESRawEvent.fSequenceNumber", &PLEIADESRawEvent_fSequenceNumber, &b_PLEIADESRawEvent_fSequenceNumber);
   fChain->SetBranchAddress("PLEIADESRawEvent.fE_FPGA_Trapez[4][4][16]", PLEIADESRawEvent_fE_FPGA_Trapez, &b_PLEIADESRawEvent_fE_FPGA_Trapez);
   fChain->SetBranchAddress("PLEIADESRawEvent.fTrace[4][4][16]", PLEIADESRawEvent_fTrace, &b_PLEIADESRawEvent_fTrace);
   fChain->SetBranchAddress("PLEIADESRawEvent.fTraceBLR[4][4][16]", PLEIADESRawEvent_fTraceBLR, &b_PLEIADESRawEvent_fTraceBLR);
   fChain->SetBranchAddress("PLEIADESRawEvent.fTrapezFPGA[4][4][16]", PLEIADESRawEvent_fTrapezFPGA, &b_PLEIADESRawEvent_fTrapezFPGA);
   Notify();
}

Bool_t MyTreeAnalysis::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

void MyTreeAnalysis::Show(Long64_t entry)
{
// Print contents of entry.
// If entry is not specified, print current entry
   if (!fChain) return;
   fChain->Show(entry);
}
Int_t MyTreeAnalysis::Cut(Long64_t entry)
{
// This function may be called from Loop.
// returns  1 if entry is accepted.
// returns -1 otherwise.
   return 1;
}
#endif // #ifdef MyTreeAnalysis_cxx
