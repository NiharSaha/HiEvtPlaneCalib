/*
 * plotQvectorCheck.C
 *
 * Macro to plot Q-vectors and EP angles from the checkep_cfg.py output
 * (check.root, tree "checkflattening/tree"), i.e. with the calibration DB applied.
 *   level 0 = raw (epcos_orig / epsin_orig / epang_orig)
 *   level 1 = recentered (epcos_RecenterOnly / epsin_RecenterOnly / epang_RecenterOnly)
 *   level 2 = flattened (epang)
 *
 * Generates 12 canvases, saved as BOTH .pdf and .png (24 files total):
 * 1-6.  Raw Q-vectors per component (HFm, HFp, HF, trackm, trackp, trackmid):
 *       columns Qx, Qy; upper row 2nd harmonic, lower row 3rd harmonic; runs overlaid
 * 7-12. Recentered vs raw Q-vectors followed by EP angles (raw, recentered, flattened)
 *       per component, all selected runs combined; upper row n=2, lower row n=3
 *
 * Input: a .lis file with one check.root path per line, or a single path / local wildcard.
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <array>
#include <string>
#include <algorithm>
#include <sys/stat.h>
#include "TFile.h"
#include "TChain.h"
#include "TH1D.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TMath.h"
#include "TLatex.h"
#include "TPad.h"

#include "../HiEvtPlaneList.h"

using namespace std;
using namespace hi;

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
    hist->SetTitle("");
    hist->SetXTitle(xTitle.c_str());
    hist->SetYTitle(yTitle.c_str());
    hist->GetXaxis()->SetLabelSize(0.05);
    hist->GetXaxis()->SetTitleSize(0.06);
    hist->GetYaxis()->SetLabelSize(0.05);
    hist->GetYaxis()->SetTitleSize(0.06);
    hist->GetXaxis()->SetTitleOffset(1.1);
    hist->GetYaxis()->SetTitleOffset(1.7);
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
    latex.DrawLatex(1 - pad->GetRightMargin(), yText, "PbPb 5.36 TeV (2024)");
}

// Helper to draw the detector label inside the frame (top right)
void drawPadLabel(TPad* pad, string label) {
    TLatex latex;
    latex.SetNDC();
    latex.SetTextFont(42);
    latex.SetTextSize(0.05);
    latex.SetTextAlign(33);
    latex.DrawLatex(1 - pad->GetRightMargin() - 0.03, 1 - pad->GetTopMargin() - 0.03, label.c_str());
}

void setPadMargins(TPad* pad) {
    pad->SetLeftMargin(0.22);
    pad->SetBottomMargin(0.16);
    pad->SetRightMargin(0.05);
    pad->SetTopMargin(0.12);
}

// Draw a set of histograms normalized to unit area, with a common maximum
void drawNormalized(vector<TH1D*>& hists, double headroom) {
    double globalMax = 0;
    for (auto h : hists) {
        if (h->Integral() > 0) h->Scale(1.0 / h->Integral());
        globalMax = max(globalMax, h->GetMaximum());
    }
    for (size_t i = 0; i < hists.size(); i++) {
        if (i == 0) {
            hists[i]->SetMaximum(globalMax * headroom);
            hists[i]->SetMinimum(0);
            hists[i]->Draw("HIST");
        } else {
            hists[i]->Draw("HIST SAME");
        }
    }
}

int epIndex(const string& name) {
    for (int i = 0; i < NumEPNames; i++)
        if (EPNames[i] == name) return i;
    cout << "Error: unknown EP name " << name << endl;
    return -1;
}

void plotQvectorCheck(string input = "check_files.lis",
                      vector<unsigned int> runs = {388468, 388622, 388713, 388784},
                      string outputDir = "./QvecPlots",
                      double centMax = 80.,
                      Long64_t maxEvents = -1) {

    createDirectory(outputDir);

    // ---------------------------------------------------------------------
    // Input chain
    // ---------------------------------------------------------------------
    TChain *chain = new TChain("checkflattening/tree");
    if (input.size() > 4 && input.substr(input.size() - 4) == ".lis") {
        ifstream lis(input);
        string line;
        while (getline(lis, line)) {
            if (!line.empty()) chain->Add(line.c_str());
        }
    } else {
        chain->Add(input.c_str());
    }
    Long64_t nEntries = chain->GetEntries();
    if (nEntries == 0) {
        cout << "Error: no entries found in " << input << endl;
        return;
    }
    if (maxEvents > 0 && maxEvents < nEntries) nEntries = maxEvents;

    UInt_t run;
    Double_t cent;
    Double_t qx[2][NumEPNames], qy[2][NumEPNames], ang[3][NumEPNames];
    chain->SetBranchStatus("*", 0);
    for (auto b : {"Run", "Cent", "epcos_orig", "epsin_orig", "epcos_RecenterOnly", "epsin_RecenterOnly",
                   "epang_orig", "epang_RecenterOnly", "epang"})
        chain->SetBranchStatus(b, 1);
    chain->SetBranchAddress("Run", &run);
    chain->SetBranchAddress("Cent", &cent);
    chain->SetBranchAddress("epcos_orig", qx[0]);
    chain->SetBranchAddress("epsin_orig", qy[0]);
    chain->SetBranchAddress("epcos_RecenterOnly", qx[1]);
    chain->SetBranchAddress("epsin_RecenterOnly", qy[1]);
    chain->SetBranchAddress("epang_orig", ang[0]);
    chain->SetBranchAddress("epang_RecenterOnly", ang[1]);
    chain->SetBranchAddress("epang", ang[2]);

    cout << "=== Q-vector / EP check plotter ===" << endl;
    cout << "Input: " << input << " (" << nEntries << " events used)" << endl;
    cout << "Output directory: " << outputDir << endl << endl;

    map<unsigned int, int> runIndex;
    for (size_t r = 0; r < runs.size(); r++) runIndex[runs[r]] = r;

    // ---------------------------------------------------------------------
    // Pass 1: Q-vector ranges per detector (99.5% of |Q| from the first events)
    // ---------------------------------------------------------------------
    Long64_t nRange = min(nEntries, (Long64_t)200000);
    vector<vector<double>> absQ(NumEPNames);
    for (Long64_t ev = 0; ev < nRange; ev++) {
        chain->GetEntry(ev);
        if (cent > centMax) continue;
        for (int d = 0; d < NumEPNames; d++) {
            if (ang[0][d] < -5) continue;
            absQ[d].push_back(fabs(qx[0][d]));
            absQ[d].push_back(fabs(qy[0][d]));
        }
    }
    double qRange[NumEPNames];
    for (int d = 0; d < NumEPNames; d++) {
        qRange[d] = 1.;
        if (!absQ[d].empty()) {
            size_t k = (size_t)(0.995 * (absQ[d].size() - 1));
            nth_element(absQ[d].begin(), absQ[d].begin() + k, absQ[d].end());
            qRange[d] = 1.1 * absQ[d][k];
        }
    }

    // ---------------------------------------------------------------------
    // Histograms: [run][det][level]
    // ---------------------------------------------------------------------
    const char* levelName[3] = {"raw", "rec", "flat"};
    size_t nRuns = runs.size();
    vector<vector<array<TH1D*, 2>>> hQx(nRuns, vector<array<TH1D*, 2>>(NumEPNames));
    vector<vector<array<TH1D*, 2>>> hQy(nRuns, vector<array<TH1D*, 2>>(NumEPNames));
    vector<vector<array<TH1D*, 3>>> hPsi(nRuns, vector<array<TH1D*, 3>>(NumEPNames));
    for (size_t r = 0; r < nRuns; r++) {
        for (int d = 0; d < NumEPNames; d++) {
            double psiRange = TMath::Pi() / EPOrder[d];
            for (int l = 0; l < 3; l++) {
                if (l < 2) {
                    hQx[r][d][l] = new TH1D(Form("Qx_%s_%s_%u", EPNames[d].c_str(), levelName[l], runs[r]), "",
                                            200, -qRange[d], qRange[d]);
                    hQy[r][d][l] = new TH1D(Form("Qy_%s_%s_%u", EPNames[d].c_str(), levelName[l], runs[r]), "",
                                            200, -qRange[d], qRange[d]);
                }
                hPsi[r][d][l] = new TH1D(Form("Psi_%s_%s_%u", EPNames[d].c_str(), levelName[l], runs[r]), "",
                                         200, -psiRange, psiRange);
            }
        }
    }

    // ---------------------------------------------------------------------
    // Pass 2: fill
    // ---------------------------------------------------------------------
    for (Long64_t ev = 0; ev < nEntries; ev++) {
        chain->GetEntry(ev);
        if (ev % 1000000 == 0) cout << "Processing event " << ev << " / " << nEntries << endl;
        auto it = runIndex.find(run);
        if (it == runIndex.end()) continue;
        if (cent > centMax) continue;
        int r = it->second;
        for (int d = 0; d < NumEPNames; d++) {
            if (ang[0][d] < -5) continue;
            for (int l = 0; l < 2; l++) {
                hQx[r][d][l]->Fill(qx[l][d]);
                hQy[r][d][l]->Fill(qy[l][d]);
            }
            for (int l = 0; l < 3; l++) {
                if (ang[l][d] > -5) hPsi[r][d][l]->Fill(ang[l][d]);
            }
        }
    }

    // Summary of mean Q before and after recentering (should be ~0 after)
    cout << endl << "Mean Q-vectors (cent <= " << centMax << "%): raw -> recentered" << endl;
    for (size_t r = 0; r < nRuns; r++) {
        cout << "Run " << runs[r] << " (" << hQx[r][0][0]->GetEntries() << " events)" << endl;
        for (int d = 0; d < NumEPNames; d++) {
            printf("  %-10s <Qx> %9.4f -> %9.4f   <Qy> %9.4f -> %9.4f\n", EPNames[d].c_str(),
                   hQx[r][d][0]->GetMean(), hQx[r][d][1]->GetMean(), hQy[r][d][0]->GetMean(), hQy[r][d][1]->GetMean());
        }
    }
    cout << endl;

    vector<string> allComponents = {"HFm", "HFp", "HF", "trackm", "trackp", "trackmid"};
    vector<int> harmonics = {2, 3};
    int runColors[] = {kBlack, kRed, kBlue, kGreen + 2};

    // =========================================================================
    // SET 1: Raw Q-vectors, runs overlaid (columns Qx, Qy; rows n=2, n=3)
    // =========================================================================
    for (const auto& comp : allComponents) {
        TCanvas *c = new TCanvas(("c_raw_" + comp).c_str(), (comp + " raw Q").c_str(), 1200, 1000);
        c->Divide(2, harmonics.size(), 0.01, 0.01);

        for (size_t h = 0; h < harmonics.size(); h++) {
            string det = comp + to_string(harmonics[h]);
            int d = epIndex(det);
            for (int axis = 0; axis < 2; axis++) {
                TPad *pad = (TPad *)c->GetPad(h * 2 + axis + 1);
                pad->cd();
                setPadMargins(pad);

                string xTitle = Form("Q_{%d,%s}", harmonics[h], axis == 0 ? "x" : "y");
                vector<TH1D*> hists;
                TLegend *leg = new TLegend(0.26, 0.62, 0.56, 0.86);
                leg->SetBorderSize(0);
                leg->SetFillStyle(0);
                leg->SetTextSize(0.045);
                leg->SetTextFont(42);
                for (size_t r = 0; r < nRuns; r++) {
                    TH1D *hist = (TH1D *)(axis == 0 ? hQx[r][d][0] : hQy[r][d][0])->Clone();
                    styleHistogram(hist, runColors[r % 4], xTitle, "Normalized Counts");
                    hists.push_back(hist);
                    leg->AddEntry(hist, Form("Run %u", runs[r]), "l");
                }
                drawNormalized(hists, 1.5);
                leg->Draw();
                drawPadLabel(pad, det + " Raw");
                drawCMSLabels(pad);
            }
        }

        string base = outputDir + "/Qvec_" + comp + "_n2n3_Raw";
        c->SaveAs((base + ".pdf").c_str());
        c->SaveAs((base + ".png").c_str());
        delete c;
    }

    // =========================================================================
    // SET 2: Recentered vs raw Q-vectors, then EP angles (raw, recentered, flattened)
    //        all selected runs combined; columns Qx, Qy, Psi; rows n=2, n=3
    // =========================================================================
    for (const auto& comp : allComponents) {
        TCanvas *c = new TCanvas(("c_rec_" + comp).c_str(), (comp + " recentered Q").c_str(), 1800, 1000);
        c->Divide(3, harmonics.size(), 0.01, 0.01);

        for (size_t h = 0; h < harmonics.size(); h++) {
            string det = comp + to_string(harmonics[h]);
            int d = epIndex(det);
            for (int col = 0; col < 3; col++) {
                TPad *pad = (TPad *)c->GetPad(h * 3 + col + 1);
                pad->cd();
                setPadMargins(pad);

                int nLevels = (col < 2) ? 2 : 3;
                const char* levelLabel[3] = {"Raw", "Recentered", "Flattened"};
                int levelColors[3] = {kBlue, kRed, kGreen + 2};
                string xTitle = (col < 2) ? Form("Q_{%d,%s}", harmonics[h], col == 0 ? "x" : "y")
                                          : Form("#Psi_{%d}", harmonics[h]);

                vector<TH1D*> hists;
                TLegend *leg = new TLegend(0.26, 0.64, 0.56, 0.86);
                leg->SetBorderSize(0);
                leg->SetFillStyle(0);
                leg->SetTextSize(0.042);
                leg->SetTextFont(42);
                for (int l = 0; l < nLevels; l++) {
                    TH1D *sum = nullptr;
                    for (size_t r = 0; r < nRuns; r++) {
                        TH1D *src = (col == 0) ? hQx[r][d][l] : (col == 1) ? hQy[r][d][l] : hPsi[r][d][l];
                        if (!sum) {
                            sum = (TH1D *)src->Clone(Form("%s_sum", src->GetName()));
                        } else {
                            sum->Add(src);
                        }
                    }
                    styleHistogram(sum, levelColors[l], xTitle, "Normalized Counts");
                    sum->SetLineWidth(2);
                    hists.push_back(sum);
                    if (col < 2) {
                        leg->AddEntry(sum, Form("%s (#LTQ#GT = %.3f)", levelLabel[l], sum->GetMean()), "l");
                    } else {
                        leg->AddEntry(sum, levelLabel[l], "l");
                    }
                }
                // Psi distributions are close to flat: leave room above for the legend
                drawNormalized(hists, col < 2 ? 1.5 : 1.6);
                leg->Draw();
                drawPadLabel(pad, det);
                drawCMSLabels(pad);
            }
        }

        string base = outputDir + "/Qvec_" + comp + "_n2n3_RecenterVsRaw";
        c->SaveAs((base + ".pdf").c_str());
        c->SaveAs((base + ".png").c_str());
        delete c;
    }

    cout << "\n=== Macro completed successfully! Total 12 PDFs and 12 PNGs generated. ===" << endl;
}
