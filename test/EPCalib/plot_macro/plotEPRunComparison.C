/*
 * plotEPRunComparison.C
 * 
 * Macro to plot psi vs psiFlat from the EPCalib output across multiple runs.
 * Generates 8 canvases, saved as BOTH .pdf and .png (16 files total):
 * 1-3. Run-wise raw vs flat comparisons for HFm, HFp, HF
 * 4-6. Run-wise raw vs flat comparisons for trackm, trackp, trackmid
 *      (4 runs per row: upper row 2nd harmonic, lower row 3rd harmonic)
 * 7.   Normalized raw psi overlays for HF detectors across 4 runs
 * 8.   Normalized raw psi overlays for Tracker detectors across 4 runs
 *      (upper row 2nd harmonic, lower row 3rd harmonic)
 */

#include <iostream>
#include <vector>
#include <sys/stat.h>
#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TH1D.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TMath.h"
#include "TGaxis.h"
#include "TLatex.h"

using namespace std;

// Helper function to create directory if it doesn't exist
void createDirectory(const string& dirPath) {
    struct stat buffer;
    if (stat(dirPath.c_str(), &buffer) != 0) {
        cout << "Creating directory: " << dirPath << endl;
        system(("mkdir -p " + dirPath).c_str());
    }
}

// Helper to set styling
void styleHistogram(TH1D* hist, int color, string xTitle, string yTitle) {
    hist->SetLineColor(color);
    hist->SetLineWidth(1);
    hist->SetStats(0);
    hist->SetMarkerStyle(0);
    hist->SetXTitle(xTitle.c_str());
    hist->SetYTitle(yTitle.c_str());
    hist->GetXaxis()->SetLabelSize(0.05);
    hist->GetXaxis()->SetTitleSize(0.06);
    hist->GetYaxis()->SetLabelSize(0.05);
    hist->GetYaxis()->SetTitleSize(0.06);
    hist->GetXaxis()->SetTitleOffset(1.1);
    hist->GetYaxis()->SetTitleOffset(1.3);
}

// Helper to draw "CMS Internal" (top left) and collision label (top right) above the frame
void drawCMSLabels(TPad* pad) {
    TLatex latex;
    latex.SetNDC();
    latex.SetTextFont(42);
    latex.SetTextSize(0.042);
    double yText = 1 - pad->GetTopMargin() + 0.015;
    latex.SetTextAlign(11);
    latex.DrawLatex(pad->GetLeftMargin(), yText, "#bf{CMS} #it{Internal}");
    latex.SetTextAlign(31);
    latex.DrawLatex(1 - pad->GetRightMargin(), yText, "PbPb 5.36 TeV (2025)");
}

// Helper to draw the detector / run label inside the frame (top right)
void drawPadLabel(TPad* pad, string label) {
    TLatex latex;
    latex.SetNDC();
    latex.SetTextFont(42);
    latex.SetTextSize(0.05);
    latex.SetTextAlign(33);
    latex.DrawLatex(1 - pad->GetRightMargin() - 0.03, 1 - pad->GetTopMargin() - 0.03, label.c_str());
}

// TODO 2025: the default run list below is from 2024; pass the 2025 runs explicitly
void plotEPRunComparison(vector<string> runs = {"388468", "388713", "388750", "388784"}, 
                         string inputDir = "/scratch/negishi/saha115/EP_calib/EP2025_Runwise/output", 
                         string outputDir = "./EPplots_RunWise") {
    
    if (runs.size() != 4) {
        cout << "Warning: Macro designed specifically for 4 runs. Layouts use 2x2 or 4 entries." << endl;
    }

    createDirectory(outputDir);

    // Open all 4 files
    vector<TFile*> files;
    for (const auto& run : runs) {
        string filename = inputDir + "/ep_HIMB01_" + run + ".root";
        TFile *file = new TFile(filename.c_str(), "read");
        if (!file || file->IsZombie()) {
            cout << "Error: Cannot open file " << filename << endl;
            return;
        }
        files.push_back(file);
    }

    cout << "=== Run-wise EP Comparison Plotter ===" << endl;
    cout << "Output directory: " << outputDir << endl << endl;

    // Detector components; the harmonic order is appended (e.g. HFm -> HFm2, HFm3)
    vector<string> allComponents = {"HFm", "HFp", "HF", "trackm", "trackp", "trackmid"};
    vector<int> harmonics = {2, 3};

    // =========================================================================
    // TASKS 1 & 2: 6 Canvases (Raw vs Flat, 4 runs x 2 harmonics per component)
    // =========================================================================
    for (const auto& comp : allComponents) {
        string canvasName = "c_" + comp;
        string canvasTitle = comp + " Raw vs Flat Run Comparison";

        TCanvas *c = new TCanvas(canvasName.c_str(), canvasTitle.c_str(), 2000, 1000);
        c->Divide(runs.size(), harmonics.size(), 0.01, 0.01);

        for (size_t h = 0; h < harmonics.size(); h++) {
            string det = comp + to_string(harmonics[h]);
            string psiTitle = Form("#Psi_{%d}", harmonics[h]);
            for (size_t i = 0; i < runs.size(); i++) {
                TPad *pad = (TPad *)c->GetPad(h * runs.size() + i + 1);
                pad->cd();
                pad->SetLeftMargin(0.22);
                pad->SetBottomMargin(0.16);
                pad->SetRightMargin(0.05);
                pad->SetTopMargin(0.12);

                TDirectory *outdir = (TDirectory *)files[i]->Get("hiEvtPlaneFlatCalib");
                TDirectory *epDir = (TDirectory *)outdir->Get(det.c_str());
                
                TH1D *hPsi = (TH1D *)epDir->Get("psi");
                TH1D *hPsiFlat = (TH1D *)epDir->Get("psiFlat");

                TH1D *hist1 = (TH1D *)hPsi->Clone(Form("psi_%s_run%s", det.c_str(), runs[i].c_str()));
                TH1D *hist2 = (TH1D *)hPsiFlat->Clone(Form("psiFlat_%s_run%s", det.c_str(), runs[i].c_str()));
                hist1->SetDirectory(0);
                hist2->SetDirectory(0);

                styleHistogram(hist1, kBlue, psiTitle, "Counts");
                styleHistogram(hist2, kRed, psiTitle, "Counts");
                hist1->SetTitle("");
                hist1->GetYaxis()->SetTitleOffset(1.7);

                double maxVal = max(hist1->GetMaximum(), hist2->GetMaximum());
                hist1->SetMaximum(maxVal * 1.4);  
                
                hist1->Draw("hist");
                hist2->Draw("hist same");

                TLegend *leg = new TLegend(0.26, 0.72, 0.56, 0.86);
                leg->AddEntry(hist1, "Original", "l");
                leg->AddEntry(hist2, "Flattened", "l");
                leg->SetBorderSize(0);
                leg->SetFillStyle(0);
                leg->SetTextSize(0.05);
                leg->SetTextFont(42);
                leg->Draw();
                drawPadLabel(pad, "#splitline{" + det + "}{Run " + runs[i] + "}");
                drawCMSLabels(pad);
            }
        }

        string pdfName = outputDir + "/EP_" + comp + "_n2n3_RunWise.pdf";
        string pngName = outputDir + "/EP_" + comp + "_n2n3_RunWise.png";
        c->SaveAs(pdfName.c_str());
        c->SaveAs(pngName.c_str());
        cout << "Saved PDF: " << pdfName << endl;
        cout << "Saved PNG: " << pngName << endl;
        delete c;
    }

    // =========================================================================
    // TASKS 3 & 4: 2 Canvases (Overlays of normalized raw distributions)
    // =========================================================================
    
    // Force scientific notation (10^-N) on axes with more than 3 digits
    TGaxis::SetMaxDigits(3); 

    // Upper row 2nd harmonic, lower row 3rd harmonic
    vector<vector<string>> overlayGroups = {
        {"HFm2", "HFp2", "HF2", "HFm3", "HFp3", "HF3"},
        {"trackm2", "trackp2", "trackmid2", "trackm3", "trackp3", "trackmid3"}
    };
    vector<string> overlayBaseNames = {
        "EP_HF_Normalized_Overlay",
        "EP_Tracker_Normalized_Overlay"
    };
    
    int runColors[] = {kBlack, kRed, kBlue, kGreen+2};

    for (size_t g = 0; g < overlayGroups.size(); g++) {
        string canvasName = Form("c_overlay_%zu", g);
        TCanvas *cOverlay = new TCanvas(canvasName.c_str(), "Overlay Comparison", 1500, 1000);
        cOverlay->Divide(3, 2, 0.02, 0.02);

        for (size_t d = 0; d < overlayGroups[g].size(); d++) {
            string det = overlayGroups[g][d];
            string psiTitle = Form("#Psi_{%c}", det.back());
            TPad *pad = (TPad *)cOverlay->GetPad(d + 1);
            pad->cd();
            
            // Left margin set to 0.22 to give room for 10^-N multiplier & Y-axis title
            pad->SetLeftMargin(0.22); 
            pad->SetBottomMargin(0.16);
            pad->SetRightMargin(0.05);
            pad->SetTopMargin(0.12);

            vector<TH1D*> histsToDraw;
            double globalMax = 0;

            for (size_t i = 0; i < runs.size(); i++) {
                TDirectory *outdir = (TDirectory *)files[i]->Get("hiEvtPlaneFlatCalib");
                TDirectory *epDir = (TDirectory *)outdir->Get(det.c_str());
                TH1D *hPsi = (TH1D *)epDir->Get("psi");

                TH1D *hClone = (TH1D *)hPsi->Clone(Form("norm_%s_run%s", det.c_str(), runs[i].c_str()));
                hClone->SetDirectory(0);
                
                // Rebin by factor of 2 for fine modulation resolution without thick noise
                hClone->Rebin(2);
                
                if (hClone->Integral() > 0) {
                    hClone->Scale(1.0e3 / hClone->Integral());  // in units of 10^-3
                }

                styleHistogram(hClone, runColors[i], psiTitle, "Normalized Counts (#times10^{-3})");
                hClone->SetTitle("");
                
                hClone->GetYaxis()->SetTitleOffset(1.6); 
                hClone->SetLineWidth(1); 
                
                if (hClone->GetMaximum() > globalMax) {
                    globalMax = hClone->GetMaximum();
                }
                histsToDraw.push_back(hClone);
            }

            TLegend *leg = new TLegend(0.40, 0.20, 0.85, 0.40);
            leg->SetBorderSize(0);
            leg->SetFillStyle(0);
            leg->SetTextSize(0.045);
            
            for (size_t i = 0; i < histsToDraw.size(); i++) {
                if (i == 0) {
                    histsToDraw[i]->SetMaximum(globalMax * 1.3);
                    histsToDraw[i]->Draw("HIST L"); 
                } else {
                    histsToDraw[i]->Draw("HIST L SAME");
                }
                leg->AddEntry(histsToDraw[i], Form("Run %s", runs[i].c_str()), "l");
            }
            leg->Draw();
            drawPadLabel(pad, det + " Raw");
            drawCMSLabels(pad);
        }

        string pdfName = outputDir + "/" + overlayBaseNames[g] + ".pdf";
        string pngName = outputDir + "/" + overlayBaseNames[g] + ".png";
        cOverlay->SaveAs(pdfName.c_str());
        cOverlay->SaveAs(pngName.c_str());
        cout << "Saved PDF: " << pdfName << endl;
        cout << "Saved PNG: " << pngName << endl;
        delete cOverlay;
    }

    for(auto f : files) {
        f->Close();
    }
    
    cout << "\n=== Macro completed successfully! Total 8 PDFs and 8 PNGs generated. ===" << endl;
}