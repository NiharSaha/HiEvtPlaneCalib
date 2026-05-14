#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TH1I.h"
#include "TDirectory.h"
#include <iostream>
#include "string.h"
#include "stdio.h"

//#include "EPCalib/HiEvtPlaneList.h"
//#include "EPCalib/HiEvtPlaneFlatten.h"
void Loop3() {
  save = fopen(saveName.data(), "rb");
  ncnt = 0;
  int pcnt = 0;
  while (fread(&sout, sizeof(struct sout_struct), 1, save) > 0) {
    EPcent = (float)sout.cent;
    EPvtx = (float)sout.vtx;
    EPntrk = sout.ntrk;
    EPrun = sout.run;
    vtx = sout.vtx;
    bin = sout.bin;
    centval = sout.cent;
    for (int j = 0; j < NumEPNames; j++) {
      double psiOffset = -10;
      double psiFlat = -10;
      double psi = -10;
      EPAngs[j] = -10;
      if (sout.msum[j] > 0) {
        double soff = flat[j]->soffset(sout.ws[j], vtx, bin);
        double coff = flat[j]->coffset(sout.wc[j], vtx, bin);

        psi = atan2(sout.ws[j], sout.wc[j]) / EPOrder[j];
        psiOffset = flat[j]->offsetPsi(soff, coff);
        if (centval < 80 && fabs(vtx) < 15 && bin >= 0) {
          hPsiOffset2[j]->Fill(psiOffset);
        }
        psiFlat = flatOffset[j]->getFlatPsi(psiOffset, vtx, bin);
        EPAngs[j] = psiFlat;
        EPOrig[j] = psi;
        if (centval <= 80 && psiFlat > -5) {
          string schk = EPNames[j];
          hPsiFlat[j]->Fill(psiFlat);
        }
      }
    }
    ++ncnt;
    EPtree->Fill();
  }
}
