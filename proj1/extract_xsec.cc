///////////////////////////////////////////////////////
////     Author: Amit Pal                  ////////////
////     Email:  amit.pal@niser.ac.in      ////////////
///////////////////////////////////////////////////////

/////  To run the program:
////   $root -l extract_xsec.cc
////   It will ask for a root file that you have created from splines. Provide full path of the file.
////   Then it will ask for directory. Just above that line you can see available directories, copy one of them and paste


#include "TFile.h"
#include "TDirectory.h"
#include "TGraph.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TString.h"
#include "TSystem.h"
#include "TROOT.h"
#include <iostream>
#include <string>
#include <vector>

// Helper function to create a total TGraph from multiple TGraphs
TGraph* createTotalGraph(const std::vector<TGraph*>& graphs, double massNumber) {
    if (graphs.empty()) {
        return nullptr;
    }
    int nPoints = graphs[0]->GetN();
    TGraph* totalGraph = new TGraph();

    for (int i = 0; i < nPoints; ++i) {
        double x, y;
        graphs[0]->GetPoint(i, x, y);
        double totalY = y;
        for (size_t j = 1; j < graphs.size(); ++j) {
            graphs[j]->GetPoint(i, x, y);
            totalY += y;
        }
        if (x != 0) { // Avoid division by zero
            totalGraph->SetPoint(i, x, totalY / (x * massNumber));
        } else {
            totalGraph->SetPoint(i, x, 0); // Set to 0 if x is 0
        }
    }
    return totalGraph;
}

// Helper function to set graph style
void setGraphStyle(TGraph* graph, int color, int width, int style) {
    if (graph) {
        graph->SetLineColor(color);
        graph->SetLineWidth(width);
        graph->SetLineStyle(style);
    }
}

void extract_xsec() {
    gSystem->Load("libTree");
    gROOT->SetStyle("Plain");

    std::string filePath;
    std::cout << "Enter the path to the ROOT file: ";
    std::cin >> filePath;

    //Read input file
    TFile *inputSpline = TFile::Open(filePath.c_str(), "READ");
    if (!inputSpline || inputSpline->IsZombie()) {
        std::cerr << "Error: Could not open file " << filePath << std::endl;
        return;
    }

    std::cout << "Available directories in the file:" << std::endl;
    inputSpline->cd();
    TIter next(gDirectory->GetListOfKeys());
    TKey *key;
    std::vector<std::string> dirNames;
    while ((key = (TKey*)next())) {
        if (TString(key->GetClassName()).Contains("TDirectory")) {
            std::cout << "  - " << key->GetName() << std::endl;
            dirNames.push_back(key->GetName());
        }
    }

    if (dirNames.empty()) {
        std::cerr << "Error: No directories found in the file." << std::endl;
        inputSpline->Close();
        return;
    }

    std::string dirName;
    std::cout << "Enter the directory name to analyze (e.g., nu_mu_Ar40): ";
    std::cin >> dirName;

    TDirectory *dir = (TDirectory*)inputSpline->Get(dirName.c_str());
    if (!dir) {
        std::cerr << "Error: Directory '" << dirName << "' not found." << std::endl;
        inputSpline->Close();
        return;
    }
    
    // Extract mass number from directory name
    TString dirTString(dirName);
    int massNumber = -1;

 
    int firstDigitPos = -1;
    for (int i = 0; i < dirTString.Length(); ++i) {
      if (isdigit(dirTString[i])) {
        firstDigitPos = i;
        break; // Stop after finding the first digit
      }
    }
    
 
    if (firstDigitPos != -1) {
      TString massStr = dirTString(firstDigitPos, dirTString.Length() - firstDigitPos);
      massNumber = massStr.Atoi();
      std::cout << "Extracted mass number: " << massNumber << std::endl;
    } else {
      std::cout << "Warning: Could not automatically detect mass number. Defaulting to 1." << std::endl;
      massNumber = 1; // Default value if no number is found
    }

    // Create the 'plots' directory if it doesn't exist
    if (gSystem->mkdir("plots", true) != 0) {
        std::cerr << "'plots' directory exists!!" << std::endl;
        //inputSpline->Close();
    }

    //
    // QEL Analysis
    //
    TGraph* qel_nc_n = (TGraph*)dir->Get("qel_nc_n");
    TGraph* qel_nc_p = (TGraph*)dir->Get("qel_nc_p");
    TGraph* qel_cc_n = (TGraph*)dir->Get("qel_cc_n");
    if (!qel_nc_n || !qel_nc_p || !qel_cc_n) {
      std::cerr << "Error: One or more QEL graphs not found." << std::endl;
    }
    TGraph* qel_total = createTotalGraph({qel_nc_n, qel_nc_p, qel_cc_n}, massNumber);
    TGraph* qel_cc = createTotalGraph({qel_cc_n}, massNumber);
    TGraph* qel_nc = createTotalGraph({qel_nc_n, qel_nc_p}, massNumber);

    TCanvas* c1 = new TCanvas("c1", "QEL Cross-sections", 800, 600);
    setGraphStyle(qel_total, kBlack, 4, kSolid);
    setGraphStyle(qel_cc, kGreen-3, 4, kDashed);
    setGraphStyle(qel_nc, kTeal+10, 4, kDotted);

    qel_total->GetXaxis()->SetLimits(0,10);
    qel_total->GetXaxis()->SetTitle("Neutrino energy (GeV)");
    qel_total->GetYaxis()->SetTitle("#sigma (per nucleon)/E_{#nu} (10^{-38} cm^{2} / GeV)");
    qel_total->Draw("AL");

    qel_cc->Draw("Lsame");
    qel_nc->Draw("Lsame");

    TLegend* l1 = new TLegend(0.6, 0.6, 0.9, 0.85, "");
    l1->AddEntry(qel_total, "QEL Total", "L");
    l1->AddEntry(qel_cc, "QEL CC", "L");
    l1->AddEntry(qel_nc, "QEL NC", "L");
    l1->SetTextSize(0.04);
    l1->Draw();
    c1->SaveAs(Form("plots/qel_numu_%s.png", dirName.c_str()));
    c1->SaveAs(Form("plots/qel_numu_%s.pdf", dirName.c_str()));

    // RES Analysis
    //
    TGraph* res_cc_p = (TGraph*)dir->Get("res_cc_p");
    TGraph* res_cc_n = (TGraph*)dir->Get("res_cc_n");
    TGraph* res_nc_p = (TGraph*)dir->Get("res_nc_p");
    TGraph* res_nc_n = (TGraph*)dir->Get("res_nc_n");

    TGraph* res_total = createTotalGraph({res_cc_p, res_cc_n, res_nc_p, res_nc_n}, massNumber);
    TGraph* res_cc = createTotalGraph({res_cc_p, res_cc_n}, massNumber);
    TGraph* res_nc = createTotalGraph({res_nc_p, res_nc_n}, massNumber);

    TCanvas* c2 = new TCanvas("c2", "RES Cross-sections", 800, 600);
    setGraphStyle(res_total, kBlack, 4, kSolid);
    setGraphStyle(res_cc, kGreen-3, 4, kDashed);
    setGraphStyle(res_nc, kTeal+10, 4, kDotted);

    res_total->GetXaxis()->SetLimits(0,10);
    res_total->GetXaxis()->SetTitle("Neutrino energy (GeV)");
    res_total->GetYaxis()->SetTitle("#sigma (per nucleon)/E_{#nu} (10^{-38} cm^{2} / GeV)");
    res_total->Draw("AL");
    res_cc->Draw("Lsame");
    res_nc->Draw("Lsame");

    TLegend* l2 = new TLegend(0.6, 0.6, 0.9, 0.85, "");
    l2->AddEntry(res_total, "RES Total", "L");
    l2->AddEntry(res_cc, "RES CC", "L");
    l2->AddEntry(res_nc, "RES NC", "L");
    l2->SetTextSize(0.04);
    l2->Draw();
    c2->SaveAs(Form("plots/res_numu_%s.png", dirName.c_str()));
    c2->SaveAs(Form("plots/res_numu_%s.pdf", dirName.c_str()));
    
    //
    // DIS Analysis
    //
    TGraph* dis_cc = (TGraph*)dir->Get("dis_cc");
    TGraph* dis_nc = (TGraph*)dir->Get("dis_nc");
    TGraph* dis_total = createTotalGraph({dis_cc, dis_nc}, massNumber);
    dis_cc = createTotalGraph({dis_cc}, massNumber);
    dis_nc = createTotalGraph({dis_nc}, massNumber);

    TCanvas* c3 = new TCanvas("c3", "DIS Cross-sections", 800, 600);
    setGraphStyle(dis_total, kBlack, 4, kSolid);
    setGraphStyle(dis_cc, kGreen-3, 4, kDashed);
    setGraphStyle(dis_nc, kTeal+10, 4, kDotted);

    dis_total->GetXaxis()->SetLimits(0,10);
    dis_total->GetXaxis()->SetTitle("Neutrino energy (GeV)");
    dis_total->GetYaxis()->SetTitle("#sigma (per nucleon)/E_{#nu} (10^{-38} cm^{2} / GeV)");
    dis_total->Draw("AL");
    dis_cc->Draw("Lsame");
    dis_nc->Draw("Lsame");

    TLegend* l3 = new TLegend(0.6, 0.6, 0.9, 0.85, "");
    l3->AddEntry(dis_total, "DIS Total", "L");
    l3->AddEntry(dis_cc, "DIS CC", "L");
    l3->AddEntry(dis_nc, "DIS NC", "L");
    l3->SetTextSize(0.04);
    l3->Draw();
    c3->SaveAs(Form("plots/dis_numu_%s.png", dirName.c_str()));
    c3->SaveAs(Form("plots/dis_numu_%s.pdf", dirName.c_str()));
    
    //
    // COH Analysis
    //
    TGraph* coh_cc = (TGraph*)dir->Get("coh_cc");
    TGraph* coh_nc = (TGraph*)dir->Get("coh_nc");
    TGraph* coh_total = createTotalGraph({coh_cc, coh_nc}, massNumber);
    coh_cc = createTotalGraph({coh_cc}, massNumber);
    coh_nc = createTotalGraph({coh_nc}, massNumber);
    
    TCanvas* c4 = new TCanvas("c4", "COH Cross-sections", 800, 600);
    setGraphStyle(coh_total, kBlack, 4, kSolid);
    setGraphStyle(coh_cc, kGreen-3, 4, kDashed);
    setGraphStyle(coh_nc, kTeal+10, 4, kDotted);

    coh_total->GetXaxis()->SetLimits(0,10);
    coh_total->GetXaxis()->SetTitle("Neutrino energy (GeV)");
    coh_total->GetYaxis()->SetTitle("#sigma (per nucleon)/E_{#nu} (10^{-38} cm^{2} / GeV)");
    coh_total->Draw("AL");
    coh_cc->Draw("Lsame");
    coh_nc->Draw("Lsame");

    TLegend* l4 = new TLegend(0.6, 0.6, 0.9, 0.85, "");
    l4->AddEntry(coh_total, "COH Total", "L");
    l4->AddEntry(coh_cc, "COH CC", "L");
    l4->AddEntry(coh_nc, "COH NC", "L");
    l4->SetTextSize(0.04);
    l4->Draw();
    c4->SaveAs(Form("plots/coh_numu_%s.png", dirName.c_str()));
    c4->SaveAs(Form("plots/coh_numu_%s.pdf", dirName.c_str()));
    
    //
    // MEC Analysis
    //
    TGraph* mec_cc = (TGraph*)dir->Get("mec_cc");
    TGraph* mec_nc = (TGraph*)dir->Get("mec_nc");
    TGraph* mec_total = createTotalGraph({mec_cc, mec_nc}, massNumber);
    mec_cc = createTotalGraph({mec_cc}, massNumber);
    mec_nc = createTotalGraph({mec_nc}, massNumber);

    TCanvas* c5 = new TCanvas("c5", "MEC Cross-sections", 800, 600);
    setGraphStyle(mec_total, kBlack, 4, kSolid);
    setGraphStyle(mec_cc, kGreen-3, 4, kDashed);
    setGraphStyle(mec_nc, kTeal+10, 4, kDotted);

    mec_total->GetXaxis()->SetLimits(0,10);
    mec_total->GetXaxis()->SetTitle("Neutrino energy (GeV)");
    mec_total->GetYaxis()->SetTitle("#sigma (per nucleon)/E_{#nu} (10^{-38} cm^{2} / GeV)");
    mec_total->Draw("AL");
    mec_cc->Draw("Lsame");
    mec_nc->Draw("Lsame");

    TLegend* l5 = new TLegend(0.6, 0.6, 0.9, 0.85, "");
    l5->AddEntry(mec_total, "MEC Total", "L");
    l5->AddEntry(mec_cc, "MEC CC", "L");
    l5->AddEntry(mec_nc, "MEC NC", "L");
    l5->SetTextSize(0.04);
    l5->Draw();
    c5->SaveAs(Form("plots/mec_numu_%s.png", dirName.c_str()));
    c5->SaveAs(Form("plots/mec_numu_%s.pdf", dirName.c_str()));
    
    //
    // Total Cross-section
    //
    TGraph* tot_cc = (TGraph*)dir->Get("tot_cc");
    TGraph* tot_nc = (TGraph*)dir->Get("tot_nc");
    TGraph* total = createTotalGraph({tot_cc, tot_nc}, massNumber);
    tot_cc = createTotalGraph({tot_cc}, massNumber);
    tot_nc = createTotalGraph({tot_nc}, massNumber);

    TCanvas* c6 = new TCanvas("c6", "Total Cross-section", 800, 600);
    setGraphStyle(total, kBlack, 4, kSolid);
    setGraphStyle(tot_cc, kGreen-3, 4, kDashed);
    setGraphStyle(tot_nc, kTeal+10, 4, kDotted);

    total->GetXaxis()->SetLimits(0,10);
    total->GetXaxis()->SetTitle("Neutrino energy (GeV)");
    total->GetYaxis()->SetTitle("#sigma (per nucleon)/E_{#nu} (10^{-38} cm^{2} / GeV)");
    total->Draw("AL");
    tot_cc->Draw("Lsame");
    tot_nc->Draw("Lsame");

    TLegend* l6 = new TLegend(0.6, 0.6, 0.9, 0.85, "");
    l6->AddEntry(total, "Total", "L");
    l6->AddEntry(tot_cc, "Total CC", "L");
    l6->AddEntry(tot_nc, "Total NC", "L");
    l6->SetTextSize(0.04);
    l6->Draw();
    c6->SaveAs(Form("plots/total_numu_%s.png", dirName.c_str()));
    c6->SaveAs(Form("plots/total_numu_%s.pdf", dirName.c_str()));



    TCanvas* c7 = new TCanvas("c7", "Total Cross-section", 800, 600);

    setGraphStyle(tot_cc, kRed, 4, kSolid);
    setGraphStyle(qel_cc, kGreen-3, 4, kDashed);
    setGraphStyle(res_cc, kTeal+10, 4, kDashed);
    setGraphStyle(dis_cc, kBlack, 4, kDashed);
    
    c7->SetLogx();
    tot_cc->GetXaxis()->SetLimits(0.1, 100);
    tot_cc->GetXaxis()->SetTitle("Neutrino energy (GeV)");
    tot_cc->GetYaxis()->SetTitle("#sigma (per nucleon)/E_{#nu} (10^{-38} cm^{2} / GeV)");
    tot_cc->Draw("AL");
    qel_cc->Draw("Lsame");
    res_cc->Draw("Lsame");
    dis_cc->Draw("Lsame");

    TLegend* l7 = new TLegend(0.6, 0.6, 0.9, 0.85, "");
    l7->AddEntry(tot_cc, "Total CC", "L");
    l7->AddEntry(qel_cc, "QEL CC", "L");
    l7->AddEntry(res_cc, "RES CC", "L");
    l7->AddEntry(dis_cc, "DIS CC", "L");
    l7->SetTextSize(0.04);
    l7->Draw();
    c7->SaveAs(Form("plots/total_numu_combined_%s.png", dirName.c_str()));
    c7->SaveAs(Form("plots/total_numu_combined_%s.pdf", dirName.c_str()));
    
    // Clean up
    inputSpline->Close();
}
