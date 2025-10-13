// To run the program
// root -l 'osc_approx_matter.cc("../truth.ghep_converted.root")'


#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TStyle.h>
#include <TMath.h>
#include <iostream>

using namespace std;

// --- Constants ---
const double rho_default = 2.8;   // g/cm3 (Earth crust typical)
const double Ye = 0.5;            // electron fraction
// Numeric conversion: A(eV^2) = 1.512e-4 * rho[g/cm3] * Ye * E[GeV]
const double A_prefactor = 1.512e-4; 

// Osc parameters (you can change)
const double th12 = 33.44 * M_PI/180.0;
const double th13 = 8.57  * M_PI/180.0;
const double th23 = 49.2  * M_PI/180.0;
const double dm21 = 7.42e-5; // eV^2
const double dm31 = 2.517e-3; // eV^2
const double deltaCP = 197.0 * M_PI/180.0;

const double L_default = 810.0; // km (NOvA baseline)

// --- Helpers: matter-effective 1-3 parameters ---
void matter_effective_1p3(double E_GeV, double rho, double Ye,
                          double Delta /*dm31*/, double th13,
                          double &Delta_m, double &sin2_2th13_m, double &cos4th13_m) {
    if (E_GeV <= 0) { Delta_m = Delta; sin2_2th13_m = sin(2*th13)*sin(2*th13); cos4th13_m = pow(cos(th13),4); return; }
    double A = A_prefactor * rho * Ye * E_GeV; // eV^2
    double cos2 = cos(2.0 * th13);
    double sin2 = sin(2.0 * th13);
    double denom = sqrt( (cos2 - A/Delta)*(cos2 - A/Delta) + sin2*sin2 );
    Delta_m = Delta * denom; 
    sin2_2th13_m = (sin2*sin2) / ( (cos2 - A/Delta)*(cos2 - A/Delta) + sin2*sin2 );
    // cos^4(theta13^m) approx: derive theta13^m then cos^4
    double sin2th_m = sqrt(sin2_2th13_m) / 2.0; // sin(theta13^m) * cos(theta13^m)
    // better: compute theta13^m via tan2θ_m = sin2 / (cos2 - A/Δ)
    double tan2 = 0.0;
    if (fabs(cos2 - A/Delta) > 1e-12) tan2 = sin2 / (cos2 - A/Delta);
    double th13_m = 0.5 * atan(tan2);
    double c13m = cos(th13_m);
    cos4th13_m = c13m*c13m*c13m*c13m;
}

// --- Vacuum approximate probabilities (leading terms) ---
double P_mu_to_e_vac(double E, double Lkm, double th23, double th13, double Delta) {
    if (E <= 0) return 0.0;
    double arg = 1.267 * Delta * Lkm / E;
    double s23 = sin(th23);
    double s2_13 = sin(2.0 * th13);
    double P = s23*s23 * (s2_13*s2_13) * (sin(arg)*sin(arg));
    if (P < 0) P = 0; if (P > 1) P = 1;
    return P;
}


double P_mu_to_mu_vac(double E, double Lkm, double th23, double th13, double Delta) {
    if (E <= 0) return 1.0;
    double arg = 1.267 * Delta * Lkm / E;
    double c13 = cos(th13);
    double s2_23 = sin(2.0 * th23);
    double P = 1.0 - (c13*c13*c13*c13) * (s2_23*s2_23) * (sin(arg)*sin(arg));
    if (P < 0) P = 0; if (P > 1) P = 1;
    return P;
}

// matter-approx appearance: 
double P_mu_to_e_matter_approx(double E, double Lkm, double rho, double Ye,
                               double th23, double Delta, double th13) {
    if (E <= 0) return 0.0;
    double Delta_m, sin2_2th13_m, cos4th13_m;
    matter_effective_1p3(E, rho, Ye, Delta, th13, Delta_m, sin2_2th13_m, cos4th13_m);
    double arg = 1.267 * Delta_m * Lkm / E;
    double s23sq = sin(th23)*sin(th23);
    double P = s23sq * sin2_2th13_m * sin(arg)*sin(arg);
    if (P < 0) P = 0; if (P > 1) P = 1;
    return P;
}

// matter-approx disappearance: 
double P_mu_to_mu_matter_approx(double E, double Lkm, double rho, double Ye,
                                double th23, double Delta, double th13) {
    if (E <= 0) return 1.0;
    double Delta_m, sin2_2th13_m, cos4th13_m;
    matter_effective_1p3(E, rho, Ye, Delta, th13, Delta_m, sin2_2th13_m, cos4th13_m);
    double arg = 1.267 * Delta_m * Lkm / E;
    double s2_23 = sin(2.0 * th23);
    double P = 1.0 - cos4th13_m * (s2_23*s2_23) * sin(arg)*sin(arg);
    if (P < 0) P = 0; if (P > 1) P = 1;
    return P;
}


// === Macro entry ===
void osc_approx_matter(const char* filename = "genie_output.root",
                       double baseline_km = L_default,
                       double density = rho_default,
                       bool normalize = true) {

    gStyle->SetOptStat(0);

    // Open file and Event tree
    TFile *f = TFile::Open(filename);
    if (!f || f->IsZombie()) { cout << "Cannot open " << filename << endl; return; }
    TTree *t = (TTree*)f->Get("Event");
    if (!t) { cout << "Event tree not found." << endl; f->Close(); return; }

    double nuE = 0;
    int nupdg = 14;
    double xsec = 1.0;

    t->SetBranchAddress("nuE", &nuE);
    if (t->GetBranch("nupdg")) t->SetBranchAddress("nupdg", &nupdg);
    if (t->GetBranch("xsection")) t->SetBranchAddress("xsection", &xsec);

    // Histograms
    int nbins = 100;
    double Emin = 0.0, Emax = 5.0;
    TH1D *h_no = new TH1D("h_no", ";E_{#nu} [GeV];Arb", nbins, Emin, Emax);
    TH1D *h_vac = new TH1D("h_vac", ";E_{#nu} [GeV];Arb", nbins, Emin, Emax);
    TH1D *h_mat = new TH1D("h_mat", ";E_{#nu} [GeV];Arb", nbins, Emin, Emax);

    Long64_t N = t->GetEntries();
    cout << "Entries: " << N << endl;

    for (Long64_t i = 0; i < N; ++i) {
        t->GetEntry(i);
        if (nuE <= 0) continue;
        if (abs(nupdg) != 14) continue; // only muon neutrinos considered here

        double w = xsec;
        h_no->Fill(nuE, w);

        // vacuum approx (dominant terms)
        double Pvac = P_mu_to_mu_vac(nuE, baseline_km, th23, th13, dm31);
        h_vac->Fill(nuE, w * Pvac);

        // matter approx
        double Pmat = P_mu_to_mu_matter_approx(nuE, baseline_km, density, Ye, th23, dm31, th13);
        h_mat->Fill(nuE, w * Pmat);
    }

    // Normalize if requested
    if (normalize) {
        if (h_no->Integral() > 0) h_no->Scale(1.0 / h_no->Integral());
        if (h_vac->Integral() > 0) h_vac->Scale(1.0 / h_vac->Integral());
        if (h_mat->Integral() > 0) h_mat->Scale(1.0 / h_mat->Integral());
    }

    // Draw
    h_no->SetLineColor(kBlack); h_no->SetLineWidth(3);
    h_vac->SetLineColor(kBlue); h_vac->SetLineWidth(3); h_vac->SetLineStyle(2);
    h_mat->SetLineColor(kMagenta); h_mat->SetLineWidth(3); h_mat->SetLineStyle(3);

    TCanvas *c = new TCanvas("c", "", 900, 700);
    h_no->Draw("HIST");
    h_vac->Draw("HIST SAME");
    h_mat->Draw("HIST SAME");

    TLegend *leg = new TLegend(0.58,0.65,0.88,0.88);
    leg->AddEntry(h_no, "Unoscillated", "l");
    leg->AddEntry(h_vac, "Oscillated (vacuum approx)", "l");
    leg->AddEntry(h_mat, "Oscillated (matter approx)", "l");
    leg->Draw();

    c->SetGrid();
    c->SaveAs("osc_approx_compare.png");
    cout << "Saved osc_approx_compare.png" << endl;

    f->Close();
}
