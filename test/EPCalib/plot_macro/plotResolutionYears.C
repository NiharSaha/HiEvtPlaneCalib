/*
 * plotResolutionYears.C
 *
 * Standalone macro to compare the EP resolution correction vs centrality
 * between data-taking years (e.g. PbPb 2023 vs 2024, later 2025).
 *
 * Input: the Rescor_<run>/<EP>_<binWidth>.dat files written by EPCalib.C
 *        (columns: centMin centMax R Rerr), one directory per year.
 * For every year and centrality bin the runs are combined with an
 * inverse-variance weighted mean; the run-to-run RMS is drawn as a band.
 *
 * Output: 6 canvases (HFm, HFp, HF, trackm, trackp, trackmid), each with
 *         n=2 (left) and n=3 (right), resolution on top and the ratio to the
 *         reference year below; saved as .pdf and .png (12 files), plus a
 *         printed table of the year-averaged resolution.
 *
 * Usage (from this directory):
 *   root -l -b -q plotResolutionYears.C+
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <cmath>
#include <algorithm>
#include <sys/stat.h>
#include "TCanvas.h"
#include "TPad.h"
#include "TH1D.h"
#include "TGraphErrors.h"
#include "TLegend.h"
#include "TLatex.h"
#include "TLine.h"
#include "TSystem.h"

using namespace std;

// One data-taking year: label, EPCalib output directory, runs (empty = all Rescor_* found)
struct YearInput {
    string label;
    string dir;
    vector<string> runs;
    int color;
    int marker;
};

// Year-averaged resolution in each centrality bin
struct ResCurve {
    vector<double> cLow, cHigh, r, rErr, rms;
};

// Helper function to create directory if it doesn't exist
void createDirectory(const string& dirPath) {
    struct stat buffer;
    if (stat(dirPath.c_str(), &buffer) != 0) {
        cout << "Creating directory: " << dirPath << endl;
        system(("mkdir -p " + dirPath).c_str());
    }
}

// Helper to draw "CMS Internal" (top left) and collision label (top right) above the frame
void drawCMSLabels(TPad* pad) {
    TLatex latex;
    latex.SetNDC();
    latex.SetTextFont(42);
    latex.SetTextSize(0.055);
    double yText = 1 - pad->GetTopMargin() + 0.02;
    latex.SetTextAlign(11);
    latex.DrawLatex(pad->GetLeftMargin(), yText, "#bf{CMS} #it{Internal}");
    latex.SetTextAlign(31);
    latex.DrawLatex(1 - pad->GetRightMargin(), yText, "PbPb 5.36 TeV");
}

// Helper to draw the detector label inside the frame (top right)
void drawPadLabel(TPad* pad, string label) {
    TLatex latex;
    latex.SetNDC();
    latex.SetTextFont(42);
    latex.SetTextSize(0.065);
    latex.SetTextAlign(33);
    latex.DrawLatex(1 - pad->GetRightMargin() - 0.03, 1 - pad->GetTopMargin() - 0.04, label.c_str());
}

// List the runs of a year from its Rescor_<run> directories
vector<string> findRuns(const string& dir) {
    vector<string> runs;
    void* d = gSystem->OpenDirectory(dir.c_str());
    if (!d) return runs;
    const char* e;
    while ((e = gSystem->GetDirEntry(d))) {
        string s(e);
        if (s.rfind("Rescor_", 0) == 0) runs.push_back(s.substr(7));
    }
    gSystem->FreeDirectory(d);
    sort(runs.begin(), runs.end());
    return runs;
}

// Read one Rescor .dat file; returns false if missing
bool readDat(const string& file, vector<double>& lo, vector<double>& hi, vector<double>& r, vector<double>& e) {
    ifstream in(file);
    if (!in) return false;
    double a, b, c, d;
    while (in >> a >> b >> c >> d) {
        lo.push_back(a); hi.push_back(b); r.push_back(c); e.push_back(d);
    }
    return !r.empty();
}

// Combine all runs of one year for one EP
ResCurve averageYear(const YearInput& y, const string& ep, const string& binWidth) {
    ResCurve out;
    vector<vector<double>> vals, errs;
    int nUsed = 0;
    for (auto& run : y.runs) {
        vector<double> lo, hi, r, e;
        string f = y.dir + "/Rescor_" + run + "/" + ep + "_" + binWidth + ".dat";
        if (!readDat(f, lo, hi, r, e)) {
            cout << "Warning: missing " << f << endl;
            continue;
        }
        if (out.cLow.empty()) {
            out.cLow = lo; out.cHigh = hi;
            vals.resize(r.size()); errs.resize(r.size());
        }
        for (size_t i = 0; i < r.size() && i < vals.size(); i++) {
            vals[i].push_back(r[i]); errs[i].push_back(e[i]);
        }
        nUsed++;
    }
    for (size_t i = 0; i < vals.size(); i++) {
        double sw = 0, swx = 0;
        for (size_t k = 0; k < vals[i].size(); k++) {
            if (errs[i][k] <= 0) continue;
            double w = 1. / (errs[i][k] * errs[i][k]);
            sw += w; swx += w * vals[i][k];
        }
        double mean = sw > 0 ? swx / sw : 0;
        double s2 = 0;
        for (auto v : vals[i]) s2 += (v - mean) * (v - mean);
        out.r.push_back(mean);
        out.rErr.push_back(sw > 0 ? 1. / sqrt(sw) : 0);
        out.rms.push_back(vals[i].size() > 1 ? sqrt(s2 / (vals[i].size() - 1)) : 0);
    }
    if (ep == "HF2") cout << y.label << ": " << nUsed << " runs used" << endl;
    return out;
}

// Ratio label, e.g. "2023/2024" (or "2023, 2025/2024" with more years)
string ratioLabel(const vector<YearInput>& years, int refIdx) {
    string num;
    for (size_t y = 0; y < years.size(); y++) {
        if ((int)y == refIdx) continue;
        if (!num.empty()) num += ", ";
        num += years[y].label;
    }
    return num + "/" + years[refIdx].label;
}

// Draw one harmonic column: resolution (top pad) and ratio to the reference year (bottom pad)
void drawColumn(TPad* top, TPad* bot, const vector<YearInput>& years, const vector<ResCurve>& curves,
                int refIdx, const string& epLabel, int order, double centMax, vector<TObject*>& keep) {
    // --- top: resolution ---
    top->cd();
    TH1D* frame = new TH1D(Form("frame_%s", epLabel.c_str()), "", 1, 0, centMax);
    frame->SetStats(0);
    frame->SetMinimum(0);
    frame->SetMaximum(order == 2 ? 1.15 : 0.95);
    frame->SetYTitle(Form("R_{%d}", order));
    frame->GetYaxis()->SetLabelSize(0.06);
    frame->GetYaxis()->SetTitleSize(0.07);
    frame->GetYaxis()->SetTitleOffset(1.0);
    frame->GetXaxis()->SetLabelSize(0);
    frame->Draw();
    keep.push_back(frame);

    TLegend* leg = new TLegend(0.20, 0.06, 0.55, 0.06 + 0.075 * years.size());
    leg->SetBorderSize(0);
    leg->SetFillStyle(0);
    leg->SetTextSize(0.06);
    for (size_t y = 0; y < years.size(); y++) {
        const ResCurve& c = curves[y];
        int n = 0;
        TGraphErrors* band = new TGraphErrors();
        TGraphErrors* g = new TGraphErrors();
        for (size_t i = 0; i < c.r.size(); i++) {
            if (c.cHigh[i] > centMax) continue;
            double x = 0.5 * (c.cLow[i] + c.cHigh[i]);
            double hw = 0.5 * (c.cHigh[i] - c.cLow[i]);
            band->SetPoint(n, x, c.r[i]);
            band->SetPointError(n, hw, c.rms[i]);
            g->SetPoint(n, x, c.r[i]);
            g->SetPointError(n, 0, c.rErr[i]);
            n++;
        }
        band->SetFillColorAlpha(years[y].color, 0.25);
        band->SetLineWidth(0);
        band->Draw("2 SAME");
        g->SetMarkerStyle(years[y].marker);
        g->SetMarkerColor(years[y].color);
        g->SetLineColor(years[y].color);
        g->SetMarkerSize(1.2);
        g->Draw("P SAME");
        leg->AddEntry(g, Form("%s (%d runs)", years[y].label.c_str(), (int)years[y].runs.size()), "p");
        keep.push_back(band); keep.push_back(g);
    }
    leg->Draw();
    keep.push_back(leg);
    drawCMSLabels(top);
    drawPadLabel(top, epLabel);
    TLatex note;
    note.SetNDC(); note.SetTextFont(42); note.SetTextSize(0.05); note.SetTextAlign(33);
    note.DrawLatex(1 - top->GetRightMargin() - 0.03, 1 - top->GetTopMargin() - 0.13, "band: run-to-run RMS");

    // --- bottom: ratio to reference year ---
    bot->cd();
    TH1D* rframe = new TH1D(Form("rframe_%s", epLabel.c_str()), "", 1, 0, centMax);
    rframe->SetStats(0);
    rframe->SetMinimum(0.9);
    rframe->SetMaximum(1.1);
    rframe->SetXTitle("Centrality (%)");
    rframe->SetYTitle(ratioLabel(years, refIdx).c_str());
    rframe->GetXaxis()->SetLabelSize(0.12);
    rframe->GetXaxis()->SetTitleSize(0.13);
    rframe->GetXaxis()->SetTitleOffset(1.0);
    rframe->GetYaxis()->SetLabelSize(0.11);
    rframe->GetYaxis()->SetTitleSize(0.11);
    rframe->GetYaxis()->SetTitleOffset(0.6);
    rframe->GetYaxis()->SetNdivisions(505);
    rframe->Draw();
    keep.push_back(rframe);
    TLine* one = new TLine(0, 1, centMax, 1);
    one->SetLineStyle(2);
    one->Draw();
    keep.push_back(one);
    const ResCurve& ref = curves[refIdx];
    for (size_t y = 0; y < years.size(); y++) {
        if ((int)y == refIdx) continue;
        const ResCurve& c = curves[y];
        TGraphErrors* g = new TGraphErrors();
        int n = 0;
        for (size_t i = 0; i < c.r.size() && i < ref.r.size(); i++) {
            if (c.cHigh[i] > centMax || ref.r[i] <= 0 || c.r[i] <= 0) continue;
            double x = 0.5 * (c.cLow[i] + c.cHigh[i]);
            double ratio = c.r[i] / ref.r[i];
            double err = ratio * sqrt(pow(c.rErr[i] / c.r[i], 2) + pow(ref.rErr[i] / ref.r[i], 2));
            g->SetPoint(n, x, ratio);
            g->SetPointError(n, 0, err);
            n++;
        }
        g->SetMarkerStyle(years[y].marker);
        g->SetMarkerColor(years[y].color);
        g->SetLineColor(years[y].color);
        g->SetMarkerSize(1.2);
        g->Draw("P SAME");
        keep.push_back(g);
    }
}

void plotResolutionYears(string outputDir = "./ResolutionYears",
                         string binWidth = "10",     // Rescor file binning: 1, 2, 5, 10, 20, 25, 30, 40
                         double centMax = 80.,
                         string refYear = "2024") {

    // Years to compare. The 2023 runs are the 18 runs in HeavyIonRPRcd_PbPb2023_Runwise.db
    // (create_db_runwise.sh builds it from EP2023_Runwise_Aug5); 2024 uses all 20 calibrated runs.
    vector<YearInput> years = {
        {"2023", "/scratch/negishi/saha115/EP_calib/EP2023_Runwise_Aug5",
         {"374681", "374719", "374730", "374778", "374803", "374810", "374833", "374970", "375007",
          "375013", "375055", "375064", "375252", "375391", "375413", "375754", "375790", "375823"},
         kBlue + 1, 20},
        {"2024", "/scratch/negishi/saha115/EP_calib/EP2024_Runwise_wEra_Sept29", {}, kRed + 1, 21},
        // TODO: add 2025 once its EPCalib step is done, e.g.
        // {"2025", "/scratch/negishi/saha115/EP_calib/EP2025_Runwise", {}, kGreen + 2, 33},
    };

    createDirectory(outputDir);

    int refIdx = 0;
    for (size_t y = 0; y < years.size(); y++) {
        if (years[y].runs.empty()) years[y].runs = findRuns(years[y].dir);
        if (years[y].label == refYear) refIdx = y;
        if (years[y].runs.empty()) {
            cout << "Error: no Rescor_<run> directories in " << years[y].dir << endl;
            return;
        }
    }

    vector<string> components = {"HFm", "HFp", "HF", "trackm", "trackp", "trackmid"};
    map<string, vector<ResCurve>> all;   // EP name -> one curve per year
    vector<TObject*> keep;

    for (auto& comp : components) {
        TCanvas* c = new TCanvas(Form("c_%s", comp.c_str()), "", 1200, 650);
        for (int col = 0; col < 2; col++) {
            int order = col + 2;
            string ep = comp + to_string(order);
            vector<ResCurve> curves;
            for (auto& y : years) curves.push_back(averageYear(y, ep, binWidth));
            all[ep] = curves;

            double x1 = col * 0.5, x2 = x1 + 0.5;
            c->cd();
            TPad* top = new TPad(Form("top_%s", ep.c_str()), "", x1, 0.32, x2, 1.0);
            top->SetLeftMargin(0.16); top->SetRightMargin(0.04);
            top->SetTopMargin(0.10); top->SetBottomMargin(0.02);
            top->Draw();
            c->cd();
            TPad* bot = new TPad(Form("bot_%s", ep.c_str()), "", x1, 0.0, x2, 0.32);
            bot->SetLeftMargin(0.16); bot->SetRightMargin(0.04);
            bot->SetTopMargin(0.03); bot->SetBottomMargin(0.32);
            bot->SetGridy();
            bot->Draw();
            drawColumn(top, bot, years, curves, refIdx, ep, order, centMax, keep);
        }
        string base = outputDir + "/Resolution_" + comp + "_n2n3_Years";
        c->SaveAs((base + ".pdf").c_str());
        c->SaveAs((base + ".png").c_str());
    }

    // Table of the year-averaged resolution
    cout << endl << "Year-averaged resolution (stat error of weighted mean, run RMS in [])" << endl;
    for (auto& comp : components) {
        for (int order = 2; order <= 3; order++) {
            string ep = comp + to_string(order);
            const vector<ResCurve>& curves = all[ep];
            printf("\n%-10s %9s", ep.c_str(), "cent");
            for (auto& y : years) printf("  %24s", y.label.c_str());
            for (size_t y = 0; y < years.size(); y++)
                if ((int)y != refIdx) printf("  %9s", (years[y].label + "/" + years[refIdx].label).c_str());
            printf("\n");
            for (size_t i = 0; i < curves[refIdx].r.size(); i++) {
                if (curves[refIdx].cHigh[i] > centMax) continue;
                printf("%-10s %3.0f-%3.0f%%", "", curves[refIdx].cLow[i], curves[refIdx].cHigh[i]);
                for (auto& cv : curves) {
                    if (i < cv.r.size()) printf("  %8.4f +- %6.4f [%6.4f]", cv.r[i], cv.rErr[i], cv.rms[i]);
                    else printf("  %24s", "-");
                }
                for (size_t y = 0; y < curves.size(); y++) {
                    if ((int)y == refIdx || i >= curves[y].r.size() || curves[refIdx].r[i] <= 0) continue;
                    printf("  %9.4f", curves[y].r[i] / curves[refIdx].r[i]);
                }
                printf("\n");
            }
        }
    }
    cout << "\n=== Macro completed successfully! Total " << 2 * components.size()
         << " files (PDF + PNG) in " << outputDir << " ===" << endl;
}
