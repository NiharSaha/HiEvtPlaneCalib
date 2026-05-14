#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/MakerMacros.h"


#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "Math/Vector3D.h"

#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "DataFormats/CaloTowers/interface/CaloTowerCollection.h"
#include "DataFormats/CastorReco/interface/CastorTower.h"

#include "DataFormats/HeavyIonEvent/interface/Centrality.h"

#include "DataFormats/HeavyIonEvent/interface/EvtPlane.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"

#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "CondFormats/DataRecord/interface/HeavyIonRPRcd.h"
#include "CondFormats/DataRecord/interface/HeavyIonRcd.h"
#include "CondFormats/HIObjects/interface/CentralityTable.h"
#include "CondCore/DBOutputService/interface/PoolDBOutputService.h"
#include "CondFormats/HIObjects/interface/RPFlatParams.h"
#include "DataFormats/Math/interface/Point3D.h"
#include "DataFormats/Common/interface/RefProd.h"
#include "DataFormats/Common/interface/Ref.h"
#include "DataFormats/Common/interface/RefVector.h"

#include "RecoHI/HiEvtPlaneAlgos/interface/HiEvtPlaneFlatten.h"
#include "TFile.h"
#include "TH1.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TH2F.h"
#include "TTree.h"
#include "TH1I.h"
#include "TF1.h"
#include "TList.h"
#include "TString.h"
#include "TMath.h"
#include <time.h>
#include <cstdlib>
#include <vector>

#include "DataFormats/PatCandidates/interface/PackedCandidate.h"
#include "RecoHI/HiEvtPlaneAlgos/interface/HiEvtPlaneList.h"
#include "RecoHI/HiEvtPlaneAlgos/interface/LoadEPDB.h"
#include "RecoHI/HiEvtPlaneAlgos/interface/EPCuts.h"
#include "FWCore/Framework/interface/ESWatcher.h"

#include "CondFormats/HIObjects/interface/CentralityTable.h"

using namespace std;
using namespace hi;
using namespace reco;
using namespace edm;
static const int MaxNumFlatBins = 200;

class EvtPlaneCalibTree : public edm::one::EDAnalyzer<> {
public:
  explicit EvtPlaneCalibTree(const edm::ParameterSet&);
  ~EvtPlaneCalibTree();
  
private:
  virtual void beginJob() ;
  virtual void analyze(const edm::Event&, const edm::EventSetup&);
  virtual void endJob() ;
  EPCuts * cuts;
  edm::Service<TFileService> fs;
  
  edm::InputTag inputPlanesTag_;
  edm::EDGetTokenT<reco::EvtPlaneCollection> inputPlanesToken_;
  edm::Handle<reco::EvtPlaneCollection> inputPlanes_;

  std::string centralityVariable_;
  std::string centralityLabel_;
  std::string centralityMC_;

  edm::InputTag centralityBinTag_;
  edm::EDGetTokenT<int> centralityBinToken;

  edm::InputTag vertexTag_;
  edm::EDGetTokenT<std::vector<reco::Vertex>> vertexToken;
  edm::Handle<std::vector<reco::Vertex>> vertex_;

  edm::InputTag caloTag_;
  edm::EDGetTokenT<CaloTowerCollection> caloToken;
  edm::Handle<CaloTowerCollection> caloCollection_;

  edm::InputTag castorTag_;
  edm::EDGetTokenT<std::vector<reco::CastorTower>> castorToken;
  edm::Handle<std::vector<reco::CastorTower>> castorCollection_;

  edm::InputTag trackTag_;
  edm::EDGetTokenT<reco::TrackCollection> trackToken;
  edm::InputTag losttrackTag_;
  edm::EDGetTokenT<reco::TrackCollection> losttrackToken;
  edm::Handle<reco::TrackCollection> trackCollection_;
  string strack;
  //edm::EDGetTokenT<edm::View<pat::PackedCandidate>> packedToken;
  //edm::EDGetTokenT<edm::View<pat::PackedCandidate>> lostToken;

  
  edm::InputTag chi2MapTag_;
  edm::InputTag chi2MapLostTag_;
  std::vector< float > trkNormChi2;

  bool loadDB_;
  double minet_;
  double maxet_;
  double minpt_;
  double maxpt_;
  double minvtx_;
  double maxvtx_;
  int flatnvtxbins_;
  double flatminvtx_;
  double flatmaxvtx_;
  double flatdelvtx_;
  double dzdzerror_;
  double d0d0error_;
  double pterror_;
  double chi2perlayer_;
  double dzerr_;
  double dzdzerror_pix_;
  double chi2_;
  int nhitsValid_;
  int FlatOrder_;
  int NumFlatBins_;
  double nCentBins_;
  double caloCentRef_;
  double caloCentRefWidth_;
  int CentBinCompression_;
  int cutEra_;
  HiEvtPlaneFlatten * flat[NumEPNames];
  int evtCount;  
  TrackStructure track;

  edm::ESWatcher<HeavyIonRcd> hiWatcher;
 
  uint runno_; 
  float centval;
  int bin;
  int cbin;
  float zvert;
  int trkbin;
  uint ntrkval;

  TTree * tree;
  TH1D * fparams;
  TH1I * iparams;
  TH1D * hcent;
  TH1D * hvtx;
  TH1D * hcentbins;
  TH1D * hflatbins;

  float ws[NumEPNames];
  float wc[NumEPNames];
  float s[NumEPNames];
  float c[NumEPNames];
  int cnt[NumEPNames];
  float ws_no_w[NumEPNames];
  float wc_no_w[NumEPNames];
  float pt[NumEPNames];
  float pt2[NumEPNames];
  bool genFlatPsi_;
  bool useOffsetPsi_;
  bool storeNames_;
  int evcnt;
  int eptrkcnt;
  
private:
  
};
EvtPlaneCalibTree::EvtPlaneCalibTree(const edm::ParameterSet& iConfig):
  inputPlanesTag_ ( iConfig.getParameter<edm::InputTag>("inputPlanesTag") ),
  centralityVariable_ ( iConfig.getParameter<std::string>("centralityVariable") ),
  centralityBinTag_ ( iConfig.getParameter<edm::InputTag>("centralityBinTag") ),
  vertexTag_  ( iConfig.getParameter<edm::InputTag>("vertexTag") ),
  caloTag_ ( iConfig.getParameter<edm::InputTag>("caloTag") ),
  castorTag_ ( iConfig.getParameter<edm::InputTag>("castorTag") ),
  trackTag_ ( iConfig.getParameter<edm::InputTag>("trackTag") ),
  losttrackTag_ ( iConfig.getParameter<edm::InputTag>("lostTag") ),
  chi2MapTag_ ( iConfig.getParameter<edm::InputTag>("chi2MapTag") ),
  chi2MapLostTag_ ( iConfig.getParameter<edm::InputTag>("chi2MapLostTag") ),
  loadDB_ ( iConfig.getParameter<bool>("loadDB") ),
  minet_ ( iConfig.getParameter<double>("minet") ),
  maxet_ ( iConfig.getParameter<double>("maxet") ),
  minpt_ ( iConfig.getParameter<double>("minpt") ),
  maxpt_ ( iConfig.getParameter<double>("maxpt") ),
  minvtx_ ( iConfig.getParameter<double>("minvtx") ),
  maxvtx_ ( iConfig.getParameter<double>("maxvtx") ),
  flatnvtxbins_ ( iConfig.getParameter<int>("flatnvtxbins") ),
  flatminvtx_ ( iConfig.getParameter<double>("flatminvtx") ),
  flatdelvtx_ ( iConfig.getParameter<double>("flatdelvtx") ),
  dzdzerror_ (iConfig.getParameter<double>("dzdzerror")),
  d0d0error_ ( iConfig.getParameter<double>("d0d0error")),
  pterror_ ( iConfig.getParameter<double>("pterror")),
  chi2perlayer_  (iConfig.getParameter<double>("chi2perlayer")),
  dzdzerror_pix_ ( iConfig.getParameter<double>("dzdzerror_pix")) ,
  chi2_  ( iConfig.getParameter<double>("chi2") ),
  nhitsValid_ (iConfig.getParameter<int>("nhitsValid") ),
  FlatOrder_ ( iConfig.getParameter<int>("FlatOrder") ),
  NumFlatBins_ ( iConfig.getParameter<int>("NumFlatBins") ),
  caloCentRef_ ( iConfig.getParameter<double>("caloCentRef") ),
  caloCentRefWidth_ ( iConfig.getParameter<double>("caloCentRefWidth") ),
  CentBinCompression_ ( iConfig.getParameter<int>("CentBinCompression") ),
  cutEra_ ( iConfig.getParameter<int>("cutEra") )
{
  cuts = new EPCuts(EP_ERA(cutEra_),pterror_,dzdzerror_,d0d0error_,chi2perlayer_,dzdzerror_pix_,chi2_,nhitsValid_);
  nCentBins_ = 200.;
  
  hcent = fs->make<TH1D>("cent","cent",100,0,100);

  hvtx = fs->make<TH1D>("vtx","vtx",200,-20,20);
  hvtx->SetXTitle("Vertex (cm)");
  hvtx->SetYTitle("Counts");

  hcentbins = fs->make<TH1D>("centbins","centbins",200,0,200);
  hcentbins->SetXTitle("Centrality Bin");
  hcentbins->SetYTitle("counts");

  hflatbins = fs->make<TH1D>("flatbins","flatbins",200,0,200);
  hflatbins->SetXTitle("Flattening Bin");
  hflatbins->SetYTitle("counts");

  inputPlanesToken_ = consumes<reco::EvtPlaneCollection>(inputPlanesTag_);
  if(iConfig.exists("nonDefaultGlauberModel")){
    centralityMC_ = iConfig.getParameter<std::string>("nonDefaultGlauberModel");
  }
  centralityLabel_ = centralityVariable_+centralityMC_;
  centralityBinToken = consumes<int>(centralityBinTag_);
  vertexToken = consumes<std::vector<reco::Vertex>>(vertexTag_);
  caloToken = consumes<CaloTowerCollection>(caloTag_);  
  storeNames_ = 1;
  
  TString epnamesF = EPNames[0].data();
  epnamesF = epnamesF+"/F";
  for(int i = 0; i<NumEPNames; i++) if(i>0) epnamesF = epnamesF + ":" + EPNames[i].data() + "/F";  
  
  TString epnamesI = EPNames[0].data();
  epnamesI = epnamesI+"/I";
  for(int i = 0; i<NumEPNames; i++) if(i>0) epnamesI = epnamesI + ":" + EPNames[i].data() + "/I";  
  flatmaxvtx_ = flatminvtx_ + flatnvtxbins_*flatdelvtx_;
 
  string strack = trackTag_.label(); 
  cout<<"=========================="<<endl;
  cout<<"EvtPlaneProducer:         "<<endl;
  cout<<"  inputPlanesTag:         "<<inputPlanesTag_.label().data()<<endl;
  cout<<"  NumEPNames:             "<<NumEPNames<<endl;
  cout<<"  centralityVariable:     "<<centralityVariable_<<endl;
  cout<<"  centralityBinTag:       "<<centralityBinTag_<<endl;
  cout<<"  vertexTag:              "<<vertexTag_<<endl;;
  cout<<"  caloTag:                "<<caloTag_<<endl;
  cout<<"  castorTag:              "<<castorTag_<<endl;
  cout<<"  trackTag:               "<<trackTag_<<endl;
  cout<<"  losttrackTag:           "<<losttrackTag_<<endl;
  if (strack.find("packedPFCandidates")!=std::string::npos) {
    cout<<"  chi2MapTag:             "<<chi2MapTag_<<endl;
    cout<<"  chi2MapLostTag:         "<<chi2MapLostTag_<<endl;
  }
  cout<<"  loadDB:                 "<<loadDB_<<endl;
  cout<<"  minet :                 "<<minet_<<endl;
  cout<<"  maxet:                  "<<maxet_<<endl;
  cout<<"  minpt:                  "<<minpt_<<endl;
  cout<<"  maxpt:                  "<<maxpt_<<endl; 
  cout<<"  minvtx:                 "<<minvtx_<<endl;
  cout<<"  maxvtx:                 "<<maxvtx_<<endl;
  cout<<"  flatnvtxbins_:          "<<flatnvtxbins_<<endl;
  cout<<"  flatminvtx_:            "<<flatminvtx_<<endl;
  cout<<"  flatdelvtx_:            "<<flatdelvtx_<<endl;
  cout<<"  dzdzerror_:             "<<dzdzerror_<<endl;
  cout<<"  d0d0error_:             "<<d0d0error_<<endl;
  cout<<"  pterror_:               "<<pterror_<<endl;
  cout<<"  chi2perlayer_:          "<<chi2perlayer_<<endl;
  if( cutEra_ == 2 ) {
    cout<<"  dzdzerror_pix_          "<<dzdzerror_pix_<<endl;
    cout<<"  chi2_                   "<<chi2_<<endl;
  }
  cout<<"  nhitsValid:             "<<nhitsValid_<<endl;
  cout<<"  FlatOrder:              "<<FlatOrder_<<endl;
  cout<<"  NumFlatBins:            "<<NumFlatBins_<<endl;
  cout<<"  caloCentRef:            "<<caloCentRef_<<endl;
  cout<<"  caloCentRefWidth:       "<<caloCentRefWidth_<<endl;
  cout<<"  CentBinCompression_:    "<<CentBinCompression_<<endl;
  switch( cutEra_ ) 
    {
    case 0 :
      cout<<"ppReco replay"<<endl;
      break;
    case 1 :
      cout<<"HIReco replay"<<endl;
      break;
    case 2 :
      cout<<"Pixel replay"<<endl;
      break;
  case 3 :
    cout<<"GenMC replay"<<endl;
    break;
  default :
    cout<<" cutEra: "<<cutEra_<<" is not valid.  Assume ppReco"<<endl;
    cutEra_ = 0;
  }
  cout<<"=========================="<<endl;
  
  TDirectory * save = gDirectory;
  TFileDirectory conddir = fs->mkdir("Conditions");
  conddir.make<TH1I>(inputPlanesTag_.label().data(), inputPlanesTag_.label().data(),1,0,1);
  string note_NumEPNames = Form("NumEPNames_%d",NumEPNames);
  conddir.make<TH1I>(note_NumEPNames.data(), note_NumEPNames.data(),1,0,1);
  conddir.make<TH1I>(centralityVariable_.data(),centralityVariable_.data(),1,0,1);
  conddir.make<TH1I>(centralityBinTag_.label().data(),centralityBinTag_.label().data(),1,0,1);
  conddir.make<TH1I>(vertexTag_.label().data(), vertexTag_.label().data(),1,0,1);
  conddir.make<TH1I>(caloTag_.label().data(), caloTag_.label().data(),1,0,1);
  conddir.make<TH1I>(castorTag_.label().data(), castorTag_.label().data(),1,0,1);  
  conddir.make<TH1I>(trackTag_.label().data(), trackTag_.label().data(),1,0,1);
  conddir.make<TH1I>(losttrackTag_.label().data(), losttrackTag_.label().data(),1,0,1);
  if(loadDB_) {
    conddir.make<TH1I>("loadDB","loadDB",1,0,1);
  } else {
    conddir.make<TH1I>("loadDB_false","loadDB_false",1,0,1);
  }
  string note_minet = Form("minet_%06.1f",minet_);
  conddir.make<TH1I>(note_minet.data(),note_minet.data(),1,0,1);
  string note_maxet = Form("maxet_%06.1f",maxet_);
  conddir.make<TH1I>(note_maxet.data(),note_maxet.data(),1,0,1);
  string note_minpt = Form("minpt_%06.1f",minpt_);
  conddir.make<TH1I>(note_minpt.data(),note_minpt.data(),1,0,1);
  string note_maxpt = Form("maxpt_%06.1f",maxpt_);
  conddir.make<TH1I>(note_maxpt.data(),note_maxpt.data(),1,0,1);
  string note_minvtx = Form("minvtx_%06.2f",minvtx_);
  conddir.make<TH1I>(note_minvtx.data(),note_minvtx.data(),1,0,1);
  string note_maxvtx = Form("maxvtx_%06.1f",maxvtx_);
  conddir.make<TH1I>(note_maxvtx.data(),note_maxvtx.data(),1,0,1);
  string note_flatnvtxbins = Form("flatnvtxbins_%d",flatnvtxbins_);
  conddir.make<TH1I>(note_flatnvtxbins.data(), note_flatnvtxbins.data(),1,0,1);
  string note_flatminvtx = Form("flatminvtx_%07.2f",flatminvtx_);
  conddir.make<TH1I>(note_flatminvtx.data(), note_flatminvtx.data(),1,0,1);
  string note_flatdelvtx = Form("flatdelvtx_%07.2f",flatdelvtx_);
  conddir.make<TH1I>(note_flatdelvtx.data(), note_flatdelvtx.data(),1,0,1);
  string note_dzdzerror = Form("dzdzerror_%07.2f",dzdzerror_);
  conddir.make<TH1I>(note_dzdzerror.data(), note_dzdzerror.data(),1,0,1);
  string note_d0d0error = Form("d0d0error_%07.2f",d0d0error_);
  conddir.make<TH1I>(note_d0d0error.data(), note_d0d0error.data(),1,0,1);
  string note_pterror = Form("pterror_%07.2f",pterror_);
  conddir.make<TH1I>(note_pterror.data(), note_pterror.data(),1,0,1);
  string note_chi2perlayer = Form("chi2perlayer_%07.2f",chi2perlayer_);
  conddir.make<TH1I>(note_chi2perlayer.data(), note_chi2perlayer.data(),1,0,1);
  string note_dzdzerror_pix = Form("dzdzerror_pix_%07.2f",dzdzerror_pix_);
  conddir.make<TH1I>(note_dzdzerror_pix.data(), note_dzdzerror_pix.data(),1,0,1);
  string note_chi2 = Form("chi2_%07.2f",chi2_);
  conddir.make<TH1I>(note_chi2.data(), note_chi2.data(),1,0,1);
  string note_nhitsValid = Form("nhitsValid_%d",nhitsValid_);
  conddir.make<TH1I>(note_nhitsValid.data(), note_nhitsValid.data(),1,0,1);
  string note_FlatOrder = Form("FlatOrder_%d",FlatOrder_);
  conddir.make<TH1I>(note_FlatOrder.data(), note_FlatOrder.data(),1,0,1);
  string note_NumFlatBins = Form("NumFlatBins_%d",NumFlatBins_);
  conddir.make<TH1I>(note_NumFlatBins.data(), note_NumFlatBins.data(),1,0,1);
  string note_caloCentRef = Form("caloCentRef_%07.2f",caloCentRef_);
  conddir.make<TH1I>(note_caloCentRef.data(), note_caloCentRef.data(),1,0,1);
  string note_caloCentRefWidth = Form("caloCentRefWidth_%07.2f",caloCentRefWidth_);
  conddir.make<TH1I>(note_caloCentRefWidth.data(), note_caloCentRefWidth.data(),1,0,1);
  string note_CentBinCompression = Form("CentBinCompression_%d",CentBinCompression_);
  conddir.make<TH1I>(note_NumFlatBins.data(), note_NumFlatBins.data(),1,0,1);
  string note_cutEra = Form("cutEra_%d",cutEra_);
  conddir.make<TH1I>(note_cutEra.data(), note_cutEra.data(),1,0,1);
  
  save->cd();
  
  fparams = fs->make<TH1D>("fparams","fparams",12,0,12);
  iparams = fs->make<TH1I>("iparams","iparams",10,0,10);
  fparams->SetBinContent(1,minet_);
  fparams->SetBinContent(2,maxet_);
  fparams->SetBinContent(3,minpt_);
  fparams->SetBinContent(4,maxpt_);
  fparams->SetBinContent(5,flatminvtx_);
  fparams->SetBinContent(6,flatmaxvtx_);
  fparams->SetBinContent(7,flatdelvtx_);
  fparams->SetBinContent(8,caloCentRef_);
  fparams->SetBinContent(9,caloCentRefWidth_);
  fparams->SetBinContent(10,chi2perlayer_);
  iparams->SetBinContent(1,FlatOrder_);
  iparams->SetBinContent(2,NumFlatBins_);
  iparams->SetBinContent(3,CentBinCompression_);
  iparams->SetBinContent(4,flatnvtxbins_);
  tree = fs->make<TTree>("tree","EP tree");
  tree->Branch("Cent",    &centval,    "cent/F");
  tree->Branch("Vtx",     &zvert,        "vtx/F");
  tree->Branch("Run",     &runno_,     "run/i");
  tree->Branch("bin",     &bin,        "bin/I");
  tree->Branch("cbin",     &cbin,        "cbin/I");
  tree->Branch("trkbin",  &trkbin,     "trkbin/I");
  tree->Branch("NtrkOff",&ntrkval,     "ntrkoff/i");
  tree->Branch("ws", &ws, epnamesF.Data());
  tree->Branch("wc", &wc, epnamesF.Data());
  tree->Branch("ws_no_w", &ws_no_w, epnamesF.Data());
  tree->Branch("wc_no_w", &wc_no_w, epnamesF.Data());
  tree->Branch("pt", &pt, epnamesF.Data());
  tree->Branch("pt2", &pt2, epnamesF.Data());
  tree->Branch("cnt",&cnt,epnamesI.Data());
  runno_ = 0;
  evcnt = 0;
  eptrkcnt = 0;
}


EvtPlaneCalibTree::~EvtPlaneCalibTree()
{
  
  // do anything here that needs to be done at desctruction time
  // (e.g. close files, deallocate resources etc.)
  
}


//
// member functions
//

// ------------ method called to produce the data  ------------
void
EvtPlaneCalibTree::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  centval = -1;
  zvert = -100;
  
  using namespace edm;
  using namespace std;
  using namespace reco;
  runno_ = iEvent.id().run();
  
  if( hiWatcher.check(iSetup) ) {
    //
    //Get Size of Centrality Table
    //
//edm::ESHandle<CentralityTable> centDB_;
//    iSetup.get<HeavyIonRcd>().get(centralityLabel_,centDB_);
//    nCentBins_ = centDB_->m_table.size();
     nCentBins_ = 200;
  }

  //
  //Get Centrality
  //
  bin = -1;
  cbin = -1;
  edm::Handle<int> cbin_;
  iEvent.getByToken(centralityBinToken, cbin_);
  cbin = *cbin_;
  centval = cbin/2.;
  bin = cbin/CentBinCompression_;
  //
  //Get Vertex
  //
  edm::Handle<reco::VertexCollection> vertices;
  iEvent.getByToken(vertexToken,vertices);
  
  //best vertex
  double bestvz=-999.9, bestvx=-999.9, bestvy=-999.9;
  const reco::Vertex & vtx = (*vertices)[0];
  bestvz = vtx.z(); bestvx = vtx.x(); bestvy = vtx.y();
  math::XYZPoint bestvtx(bestvx,bestvy,bestvz);
  zvert = bestvz;

  
  if(bestvz<flatminvtx_ || bestvz>flatmaxvtx_) return;
  hcent->Fill(centval);
  hcentbins->Fill(cbin);
  hflatbins->Fill(bin);
  hvtx->Fill(bestvz);

  //
  //Get Event Planes
  //
  iEvent.getByToken(inputPlanesToken_,inputPlanes_);
  
  if(!inputPlanes_.isValid()){
    cout << "Error! Can't get hiEvtPlane product!" << endl;
    return ;
  }
  
  int i = 0;
  for (EvtPlaneCollection::const_iterator ep = inputPlanes_->begin();ep !=inputPlanes_->end(); ep++) {
    if(i>=NumEPNames || i!= ep->indx()) {
      cout<<"DATA INCONSISTENCY.  Too many or wrong EPs found in collection. Expect a crash!"<<endl;
    }
    ws[i]=0;
    wc[i]=0;
    cnt[i]=0;
    pt[i]=0;
    pt2[i]=0;
    ws_no_w[i]=0;
    wc_no_w[i]=0;

    double c = ep->sumCos(0);
    double s = ep->sumSin(0);
    double cnow = ep->sumCos(3);
    double snow = ep->sumSin(3);
    double ept = ep->sumPtOrEt();
    double ept2 = ep->sumPtOrEt2();
    uint m = ep->mult();

    if(i==trackmid2 ) eptrkcnt+=m;
    if(ep->angle(0)>-5) {      
      ws[i]=s;
      wc[i]=c;
      cnt[i]=m;
      pt[i] = ept;
      pt2[i] = ept2;
      ws_no_w[i] = snow;
      wc_no_w[i] = cnow;
    }
    ++i;
  }    
  ++evcnt;
  tree->Fill();   
  
}

// ------------ method called once each job just before starting event loop  ------------
void 
EvtPlaneCalibTree::beginJob()
{
}

// ------------ method called once each job just after ending the event loop  ------------
void 
EvtPlaneCalibTree::endJob() {
  cout<<"No. Events: "<<evcnt<<endl;
  cout<<"No. Tracks: "<<eptrkcnt<<endl;
}


//define this as a plug-in
DEFINE_FWK_MODULE(EvtPlaneCalibTree);
