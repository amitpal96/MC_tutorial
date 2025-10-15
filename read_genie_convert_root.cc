///////////////////////////////////////////
////   Author: Amit Pal               /////
////   Email:  amit.pal@niser.ac.in   /////
///////////////////////////////////////////

//// To run this program, use following command
//// $genie 'read_genie_convert_root.cc("gntp.0.ghep.root")'
//// Here gntp.0.ghep.root is the genie output file.
#include <TTree.h>
#include <TFile.h>
#include <TTree.h>
#include <TString.h>
#include <iostream>

using namespace genie;

void read_genie_convert_root(const char* infile)
{
  // Open input file
  TFile *myFile = new TFile(infile, "READ");
  if (!myFile || myFile->IsZombie()) {
    std::cerr << "Error: cannot open input file " << infile << std::endl;
    return;
  }
  
  // Get tree
  TTree *myTree = dynamic_cast<TTree*>(myFile->Get("gtree"));
  if (!myTree) {
    std::cerr << "Error: could not find TTree 'gtree' in " << infile << std::endl;
    return;
  }
  
  // Set branch
  NtpMCEventRecord* myEventRecord = new NtpMCEventRecord();
  myTree->SetBranchAddress("gmcrec", &myEventRecord);
  
  int nentries = myTree->GetEntries();
  
  // Construct output file name
  TString inName(infile);
  TString outName = inName;
  if (outName.EndsWith(".root")) outName.ReplaceAll(".root", "_converted.root");
  else outName.Append("_converted.root");
  
  TFile *outputFile = new TFile(outName, "RECREATE");
  TTree *Event = new TTree("Event", "Event info");
  
  int nupdg;
  double nuE;
  double nuPx, nuPy, nuPz;
  double xsection;
  bool IsQE, IsRES, IsDIS, IsCoh, IsMEC, IsCC, IsNC;
  double Q2, W;

  //Add branches to event tree
  Event->Branch("nupdg", &nupdg, "nupdg/I");
  Event->Branch("nuE", &nuE, "nuE/D");
  Event->Branch("nuPx", &nuPx, "nuPx/D");
  Event->Branch("nuPy", &nuPy, "nuPy/D");
  Event->Branch("nuPz", &nuPz, "nuPz/D");
  Event->Branch("xsection", &xsection, "xsection/D");
  Event->Branch("IsQE", &IsQE, "IsQE/O");
  Event->Branch("IsRES", &IsRES, "IsRES/O");
  Event->Branch("IsDIS", &IsDIS, "IsDIS/O");
  Event->Branch("IsCoh", &IsCoh, "IsCoh/O");
  Event->Branch("IsMEC", &IsMEC, "IsMEC/O");
  Event->Branch("IsCC", &IsCC, "IsCC/O");
  Event->Branch("IsNC", &IsNC, "IsNC/O");
  //Event->Branch("Q2", &Q2, "Q2/D");
  //Event->Branch("W", &W, "W/D");

  TTree *Particles = new TTree("Particles", "Particles info");

  std::vector<int> status, pdg;
  std::vector<double> energy, px, py, pz;

  //add branches to Particles tree
  Particles->Branch("status", &status);
  Particles->Branch("pdg", &pdg);
  Particles->Branch("energy", &energy);
  Particles->Branch("px", &px);
  Particles->Branch("py", &py);
  Particles->Branch("pz", &pz);

  //Loop over event
  for(int i=0; i<nentries; i++)
    {
      myTree->GetEntry(i);

      genie::EventRecord *myEvent = myEventRecord->event;

      //process info
      const ProcessInfo & proc = myEvent->Summary()->ProcInfo();
      //kinematic info
      const Kinematics & kine = myEvent->Summary()->Kine();

      GHepParticle * neu = myEvent->Probe();
      const TLorentzVector & k1 = *(neu->P4());

      //incoming neutrino information
      nupdg = neu->Pdg();
      nuE = k1.Energy();
      nuPx = k1.Px();
      nuPy = k1.Py();
      nuPz = k1.Pz();
      
      xsection = myEvent->XSec()/(5.07*pow(10, 13) * 5.07*pow(10,13)); //conversion from natural unit to cm^2 

      //$GENIE/Framework/Interaction/Processinfo.h
      IsQE = proc.IsQuasiElastic();
      IsRES = proc.IsResonant();
      IsDIS = proc.IsDeepInelastic();
      IsCoh = proc.IsCoherentProduction();
      IsMEC = proc.IsMEC();
      IsCC = proc.IsWeakCC();
      IsNC = proc.IsWeakNC();
      
      Event->Fill();
      
      TObjArrayIter iter(myEvent);
      GHepParticle * p = 0;
      status.clear();
      pdg.clear();
      energy.clear();
      px.clear();
      py.clear();
      pz.clear();
      
       //loop over event particles
       // $GENIE/Framework/GHEP/GHEPparticle.h
      while ((p = dynamic_cast<GHepParticle *>(iter.Next())) != nullptr) {

	 status.push_back(p->Status()); // 0=initial particles, 1=final particles
	 pdg.push_back(p->Pdg());
	 energy.push_back(p->P4()->Energy());
	 px.push_back(p->P4()->Px());
	 py.push_back(p->P4()->Py());
	 pz.push_back(p->P4()->Pz());

	 // Particles->Fill();
       }
       Particles->Fill();
       //delete myEventRecord;
       //myEventRecord = nullptr;
    }

  outputFile->Write();
  outputFile->Close();
}
