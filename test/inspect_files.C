void inspect_files() {
  TFile *calib = new TFile("calib.root", "r");
  cout << "=== calib.root ===" << endl;
  calib->cd("evtPlaneCalibTree");
  gDirectory->ls();

  TFile *mo = new TFile("ep_387853_388784.root", "r");
  cout << "\n=== ep_387853_388784.root ===" << endl;
  mo->ls();
  mo->cd("hiEvtPlaneFlatCalib");
  gDirectory->ls();

  TFile *test = new TFile("checkep.root", "r");
  cout << "\n=== checkep.root ===" << endl;
  test->ls();
  test->cd("checkflattening");
  gDirectory->ls();
  test->cd("checkflattening/trackmid2");
  gDirectory->ls();
}
