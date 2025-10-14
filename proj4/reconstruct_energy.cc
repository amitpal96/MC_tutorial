// To run this program
// root -l 'reconstruct_energy.cc("../truth.ghep_converted.root")'

#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TCanvas.h>
#include <TMath.h>
#include <iostream>
#include <vector>

void reconstruct_energy(const char* filename = "genie_output.root") {

    // Open file
    TFile* f = TFile::Open(filename);
    if (!f || f->IsZombie()) { std::cerr << "Error: cannot open file\n"; return; }

    // Get trees
    TTree* tevent = (TTree*)f->Get("Event");
    TTree* tpart  = (TTree*)f->Get("Particles");
    if (!tevent || !tpart) { std::cerr << "Error: trees not found\n"; return; }

    // Event-level branches
    double nuE, xsection;
    bool IsCC;
    tevent->SetBranchAddress("nuE", &nuE);
    tevent->SetBranchAddress("xsection", &xsection);
    tevent->SetBranchAddress("IsCC", &IsCC);

    // Particle-level branches
    std::vector<int>* pdg = nullptr;
    std::vector<int>* status = nullptr;
    std::vector<double>* energy = nullptr;
    std::vector<double>* px = nullptr;
    std::vector<double>* py = nullptr;
    std::vector<double>* pz = nullptr;

    tpart->SetBranchAddress("pdg", &pdg);
    tpart->SetBranchAddress("status", &status);
    tpart->SetBranchAddress("energy", &energy);
    tpart->SetBranchAddress("px", &px);
    tpart->SetBranchAddress("py", &py);
    tpart->SetBranchAddress("pz", &pz);

    // Constants
    const double Mn = 939.565;   // MeV
    const double Mp = 938.272;   // MeV
    const double Eb = 27.0;      // binding energy (MeV)
    const double mmu = 105.66;   // MeV

    // Histograms
    TH1D* h_true = new TH1D("h_true", "True Neutrino Energy;E_{#nu}^{true} [GeV];Events", 50, 0, 5);
    TH1D* h_cal  = new TH1D("h_cal",  "Calorimetric Reconstructed Energy;E_{#nu}^{cal} [GeV];Events", 50, 0, 5);
    TH1D* h_qe   = new TH1D("h_qe",   "Kinematic Reconstructed Energy;E_{#nu}^{QE} [GeV];Events", 50, 0, 5);
    TH2D* h_resp = new TH2D("h_resp", "Response Matrix;E_{#nu}^{true} [GeV];E_{#nu}^{cal} [GeV]", 50, 0, 5, 50, 0, 5);

    Long64_t nEntries = tevent->GetEntries();
    for (Long64_t i = 0; i < nEntries; ++i) {
        tevent->GetEntry(i);
        tpart->GetEntry(i);
        if (!IsCC) continue;

        // --- True energy ---
        double Etrue = nuE * 1000.0; // convert to MeV

        // --- Calorimetric energy ---
        double Ecal = 0;
        for (size_t j = 0; j < status->size(); ++j) {
            if (status->at(j) != 1) continue;
            double E = energy->at(j) * 1000.0; // MeV
            int pid = pdg->at(j);
            double mass = 0;
            if (abs(pid) == 13) mass = 105.66;
            else if (abs(pid) == 211) mass = 139.57;
            else if (abs(pid) == 2212) mass = 938.27;
            else if (abs(pid) == 2112) mass = 939.57;
            else if (abs(pid) == 111) mass = 134.97;
            else continue;
            double kinE = E - mass;
            if (kinE > 0) Ecal += kinE;
        }

        // --- Kinematic QE energy ---
        double Eqe = -999;
        for (size_t j = 0; j < pdg->size(); ++j) {
            if (abs(pdg->at(j)) == 13 && status->at(j) == 1) {
                double E_mu = energy->at(j) * 1000.0;
                double p_mu = sqrt(px->at(j)*px->at(j) + py->at(j)*py->at(j) + pz->at(j)*pz->at(j)) * 1000.0;
                double costh = pz->at(j) / sqrt(px->at(j)*px->at(j) + py->at(j)*py->at(j) + pz->at(j)*pz->at(j));
                Eqe = (2*(Mn - Eb)*E_mu - (Eb*Eb - 2*Mn*Eb + mmu*mmu + (Mn*Mn - Mp*Mp))) /
                      (2*((Mn - Eb) - E_mu + p_mu * costh));
                break;
            }
        }

        // --- Fill histograms ---
        h_true->Fill(Etrue/1000.0, xsection);
        h_cal->Fill(Ecal/1000.0, xsection);
        if (Eqe > 0) h_qe->Fill(Eqe/1000.0, xsection);
        h_resp->Fill(Etrue/1000.0, Ecal/1000.0, xsection);
    }

    // --- Draw ---
    TCanvas* c1 = new TCanvas("c1", "Energy Comparison", 900, 700);
    h_true->SetLineColor(kBlack);
    h_cal->SetLineColor(kGreen);
    h_qe->SetLineColor(kBlue);
    h_true->SetStats(0);
    h_true->SetLineWidth(3);
    h_cal->SetLineWidth(3);
    h_qe->SetLineWidth(3);
    h_true->Draw("HIST");
    h_cal->Draw("HIST SAME");
    h_qe->Draw("HIST SAME");
    auto leg = new TLegend(0.55,0.65,0.85,0.85);
    leg->AddEntry(h_true,"True Energy","l");
    leg->AddEntry(h_cal,"Calorimetric Energy","l");
    leg->AddEntry(h_qe,"Kinematic Energy","l");
    leg->Draw();

    TCanvas* c2 = new TCanvas("c2","Response Matrix",800,700);
    h_resp->SetStats(0);
    h_resp->Draw("COLZ");

    c1->SaveAs("reconstructed_energy.png");
    c2->SaveAs("response_matrix.png");
}
