
void compare(string calibFile, string moveFile, string checkFile) {
  TFile *calib = new TFile(calibFile.data(), "r");
  TFile *mo = new TFile(moveFile.data(), "r");
  TFile *test = new TFile(checkFile.data(), "r");

  TH1D *hmove = (TH1D *)mo->Get("hiEvtPlaneFlatCalib/trackmid2/psiFlat");
  TH1D *htest = (TH1D *)test->Get("checkflattening/trackmid2/Psi_trackmid2");
  TH1D *hmoveRaw = (TH1D *)mo->Get("hiEvtPlaneFlatCalib/trackmid2/psi");
  TH1D *htestRaw = (TH1D *)test->Get("checkflattening/trackmid2/PsiRaw_trackmid2");
  htest->SetLineColor(kRed);
  TCanvas *c = new TCanvas("c", "c", 1400, 900);
  c->Divide(3);
  c->cd(1);
  hmove->Draw();
  htest->Draw("same");
  cout << " --- flat --- " << endl;
  cout << "move: " << hmove->Integral(1, 1000) << endl;
  cout << "test: " << htest->Integral(1, 1000) << endl;

  c->cd(2);
  htestRaw->SetLineColor(kRed);
  hmoveRaw->Draw();
  htestRaw->Draw("same");
  cout << " --- raw  ---" << endl;
  cout << "move: " << hmoveRaw->Integral(1, 1000) << endl;
  cout << "test: " << htestRaw->Integral(1, 1000) << endl;
  c->cd(3);
  TH1D *hcalibcbin = (TH1D *)calib->Get("evtPlaneCalibTree/centbins");
  TH1D *htestcbin = (TH1D *)test->Get("checkflattening/centbins");
  TH1D *hmovecbin = (TH1D *)mo->Get("hcentbin");
  htestcbin->SetLineColor(kRed);
  hcalibcbin->SetLineColor(kGreen);
  hcalibcbin->Draw();
  hmovecbin->Draw("same");
  htestcbin->Draw("same");
  cout << " -- cent ---" << endl;
  cout << "calib: " << hcalibcbin->Integral(1, 1000) << endl;
  cout << "move:  " << hmovecbin->Integral(1, 1000) << endl;
  cout << "test:  " << htestcbin->Integral(1, 1000) << endl;
  c->Print("checkPlot.pdf", "pdf");
}
