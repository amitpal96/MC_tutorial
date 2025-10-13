//// To run the program
//// root -l 'plot_genie_kinematics.cc("../truth.ghep_converted.root")'

#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TMath.h>
#include <iostream>
#include <vector>

using namespace std;

void plot_genie_kinematics(const char* filename = "genie_output.root") {

    const double mN = 0.939; // nucleon mass [GeV]

    // --- Open file and get trees
    TFile *f = TFile::Open(filename);
    if (!f || f->IsZombie()) {
        cerr << "Error: cannot open " << filename << endl;
        return;
    }

    TTree *tevt = (TTree*)f->Get("Event");
    TTree *tpart = (TTree*)f->Get("Particles");

    if (!tevt || !tpart) {
        cerr << "Error: cannot find Event or Particles tree." << endl;
        return;
    }

    // --- Event tree variables
    int nupdg;
    double nuE, nuPx, nuPy, nuPz;
    bool IsQE, IsRES, IsDIS, IsMEC, IsCoh;

    tevt->SetBranchAddress("nupdg", &nupdg);
    tevt->SetBranchAddress("nuE", &nuE);
    tevt->SetBranchAddress("nuPx", &nuPx);
    tevt->SetBranchAddress("nuPy", &nuPy);
    tevt->SetBranchAddress("nuPz", &nuPz);
    tevt->SetBranchAddress("IsQE", &IsQE);
    tevt->SetBranchAddress("IsRES", &IsRES);
    tevt->SetBranchAddress("IsDIS", &IsDIS);
    tevt->SetBranchAddress("IsMEC", &IsMEC);
    tevt->SetBranchAddress("IsCoh", &IsCoh);

    // --- Particle tree variables
    vector<int>   *status = nullptr, *pdg = nullptr;
    vector<double>* energy = nullptr, *px = nullptr, *py = nullptr, *pz = nullptr;
    tpart->SetBranchAddress("status", &status);
    tpart->SetBranchAddress("pdg", &pdg);
    tpart->SetBranchAddress("energy", &energy);
    tpart->SetBranchAddress("px", &px);
    tpart->SetBranchAddress("py", &py);
    tpart->SetBranchAddress("pz", &pz);

    // --- Histograms
    const int nbins = 50;
    const double Emax = 10.0;

    auto makeHist = [&](const char* name, const char* title){
        return new TH1D(name, title, nbins, 0, Emax);
    };

    TH1D *hE_nu_total = makeHist("hE_nu_total", "Neutrino Energy;E_{#nu} [GeV];Events");
    TH1D *hE_nu_qe  = makeHist("hE_nu_qe",  "QE;E_{#nu} [GeV];Events");
    TH1D *hE_nu_res = makeHist("hE_nu_res", "RES;E_{#nu} [GeV];Events");
    TH1D *hE_nu_dis = makeHist("hE_nu_dis", "DIS;E_{#nu} [GeV];Events");
    TH1D *hE_nu_mec = makeHist("hE_nu_mec", "MEC;E_{#nu} [GeV];Events");
    TH1D *hE_nu_coh = makeHist("hE_nu_coh", "COH;E_{#nu} [GeV];Events");

    TH1D *hE_lep = makeHist("hE_lep", "Outgoing Lepton Energy;E_{lep} [GeV];Events");
    TH1D *hQ2 = new TH1D("hQ2", "Four-Momentum Transfer;Q^{2} [GeV^{2}];Events", 50, 0, 5);
    TH1D *hq3 = new TH1D("hq3", "Three-Momentum Transfer;|q| [GeV];Events", 50, 0, 5);
    TH1D *hw = new TH1D("hw", "Energy Transfer;#omega [GeV];Events", 50, 0, 5);
    TH1D *hx = new TH1D("hx", "Bjorken x;x;Events", 50, 0, 1);
    TH1D *hy = new TH1D("hy", "Bjorken y;y;Events", 50, 0, 1);

    // --- Event loop
    Long64_t N = tevt->GetEntries();
    for (Long64_t i = 0; i < N; i++) {

        tevt->GetEntry(i);
        tpart->GetEntry(i);

        hE_nu_total->Fill(nuE);
        if (IsQE)  hE_nu_qe->Fill(nuE);
        if (IsRES) hE_nu_res->Fill(nuE);
        if (IsDIS) hE_nu_dis->Fill(nuE);
        if (IsMEC) hE_nu_mec->Fill(nuE);
        if (IsCoh) hE_nu_coh->Fill(nuE);

        // Find outgoing lepton (status==1, lepton PDG)
        double Elep = -1, pxl=0, pyl=0, pzl=0;
        for (size_t j = 0; j < pdg->size(); j++) {
            if ((*status)[j] == 1 && (abs((*pdg)[j]) == 11 || abs((*pdg)[j]) == 13)) {
                Elep = (*energy)[j];
                pxl = (*px)[j]; pyl = (*py)[j]; pzl = (*pz)[j];
                break;
            }
        }
        if (Elep < 0) continue; // no outgoing lepton found

        hE_lep->Fill(Elep);

        // --- compute Q2, q3, omega, x, y
        double qx = nuPx - pxl;
        double qy = nuPy - pyl;
        double qz = nuPz - pzl;
        double q3 = sqrt(qx*qx + qy*qy + qz*qz);
        double omega = nuE - Elep;
        double Q2 = q3*q3 - omega*omega; // Q^2 = |q|^2 - ω^2
        if (Q2 < 0) Q2 = 0;

        double y = omega / nuE;
        double x = (2*mN*omega > 0) ? (Q2 / (2*mN*omega)) : 0;

        hQ2->Fill(Q2);
        hq3->Fill(q3);
        hw->Fill(omega);
        hx->Fill(x);
        hy->Fill(y);
    }

    // --- Draw
    TCanvas *c1 = new TCanvas("c1", "Neutrino Energy by Interaction Type", 800, 600);
    hE_nu_total->SetLineColor(kBlack);
    hE_nu_qe->SetLineColor(kBlue);
    hE_nu_res->SetLineColor(kRed);
    hE_nu_dis->SetLineColor(kGreen+2);
    hE_nu_mec->SetLineColor(kMagenta);
    hE_nu_coh->SetLineColor(kOrange+1);

    hE_nu_total->SetLineWidth(3);
    hE_nu_qe->SetLineWidth(3);
    hE_nu_res->SetLineWidth(3);
    hE_nu_dis->SetLineWidth(3);
    hE_nu_mec->SetLineWidth(3);
    hE_nu_coh->SetLineWidth(3);
    
    hE_nu_total->Draw("HIST");
    hE_nu_qe->Draw("HIST SAME");
    hE_nu_res->Draw("HIST SAME");
    hE_nu_dis->Draw("HIST SAME");
    hE_nu_mec->Draw("HIST SAME");
    hE_nu_coh->Draw("HIST SAME");

    auto leg = new TLegend(0.6, 0.6, 0.88, 0.88);
    leg->AddEntry(hE_nu_total, "Total", "l");
    leg->AddEntry(hE_nu_qe, "QE", "l");
    leg->AddEntry(hE_nu_res, "RES", "l");
    leg->AddEntry(hE_nu_dis, "DIS", "l");
    leg->AddEntry(hE_nu_mec, "MEC", "l");
    leg->AddEntry(hE_nu_coh, "COH", "l");
    leg->Draw();

    c1->SaveAs("neutrino_energy_types.png");

    // Draw kinematic plots
    TCanvas *c2 = new TCanvas("c2", "Kinematic Variables", 1200, 800);
    c2->Divide(3,2);
    c2->cd(1);
    hE_lep->SetStats(0);
    hE_lep->SetLineWidth(3);
    hE_lep->Draw();
    c2->cd(2);
    hQ2->SetStats(0);
    hQ2->SetLineWidth(3);
    hQ2->Draw();
    c2->cd(3);
    hq3->SetStats(0);
    hq3->SetLineWidth(3);
    hq3->Draw();
    c2->cd(4);
    hw->SetStats(0);
    hw->SetLineWidth(3);
    hw->Draw();
    c2->cd(5);
    hx->SetStats(0);
    hx->SetLineWidth(3);
    hx->Draw();
    c2->cd(6);
    hy->SetStats(0);
    hy->SetLineWidth(3);
    hy->Draw();
    c2->SaveAs("kinematics.png");

    cout << "✅ Plots saved: neutrino_energy_types.png, kinematics.png" << endl;

    f->Close();
}

