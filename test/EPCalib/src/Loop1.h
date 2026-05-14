#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TH1I.h"
#include "TDirectory.h"
#include <iostream>
#include "string.h"
#include "stdio.h"

void Loop1() {
  for (int i = 0; i < nentries; i++) {
    if (test && ncnt > ntest)
      break;
    if (tree->GetEntry(i) <= 0)
      continue;
    if (runno_ < minRun_ || runno_ > maxRun_)
      continue;
    hflatbins->Fill(bin);
    sout.vtx = vtx;
    sout.bin = bin;
    sout.cent = centval;
    sout.run = runno_;
    sout.ntrk = ntrkval;
    ++totentries;
    for (int j = 0; j < NumEPNames; j++) {
      int indx = flat[j]->offsetIndx(bin, vtx);
      sout.ws[j] = 0;
      sout.wc[j] = 0;
      sout.msum[j] = 0;
      sout.wsum[j] = 0;
      double order = EPOrder[j];
      if (wsv[j] == 0 && wcv[j] == 0)
        continue;
      double scale = flat[j]->etScale(vtx, bin);
      double s = wsv[j] * scale;
      double c = wcv[j] * scale;
      double snow = wsv_no_w[j] * scale;
      double cnow = wcv_no_w[j] * scale;
      double psiin = atan2(s, c) / order;
      double sin = s;
      double cin = c;
      double pts = pt2_[j][indx] / pt_[j][indx];
      if (MomConsWeight[j][0] == 'y' && ptav[j] > 0) {
        s = s - (pt2_[j][indx] / pt_[j][indx]) * snow;
        c = c - (pt2_[j][indx] / pt_[j][indx]) * cnow;
      }
      double psi = atan2(s, c) / order;
      flat[j]->fill(psi, vtx, bin);
      flat[j]->fillOffset(s, c, ptcnt[j], vtx, bin);
      flatOffset[j]->fillOffset(s, c, ptcnt[j], vtx, bin);
      sout.ws[j] = s;
      sout.wc[j] = c;
      sout.wsum[j] = ptcnt[j];
      sout.msum[j] = ptcnt[j];
      if (centval <= 80 && psi > -5)
        hPsi[j]->Fill(psi);
    }
    ++ncnt;
    fwrite(&sout, sizeof(struct sout_struct), 1, save);
  }
}
