#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TH1I.h"
#include "TDirectory.h"
#include <iostream>
#include "string.h"
#include "stdio.h"

void Loop2() {
  save = fopen(saveName.data(), "rb");
  ncnt = 0;
  while (fread(&sout, sizeof(struct sout_struct), 1, save) > 0) {
    vtx = sout.vtx;
    centval = sout.cent;
    bin = sout.bin;
    for (int j = 0; j < NumEPNames; j++) {
      double psiOffset = -10;
      double soff = 0;
      double coff = 0;
      if (sout.msum[j] > 0) {
        soff = flat[j]->soffset(sout.ws[j], vtx, bin);
        coff = flat[j]->coffset(sout.wc[j], vtx, bin);
        psiOffset = flat[j]->offsetPsi(soff, coff);
        if (centval < 80 && fabs(vtx) < 15 && bin >= 0) {
          hPsiOffset[j]->Fill(psiOffset);
        }
        flatOffset[j]->fill(psiOffset, vtx, bin);
      }
    }
    ++ncnt;
  }
}
