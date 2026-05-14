// system include files
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
#include "DataFormats/TrackReco/interface/TrackFwd.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/HeavyIonEvent/interface/EvtPlane.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"

#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "CondFormats/DataRecord/interface/HeavyIonRPRcd.h"
#include "CondFormats/DataRecord/interface/HeavyIonRcd.h"
#include "CondFormats/HIObjects/interface/CentralityTable.h"
#include "CondFormats/HIObjects/interface/RPFlatParams.h"
#include "DataFormats/RecoCandidate/interface/RecoChargedCandidate.h"
#include "DataFormats/RecoCandidate/interface/RecoChargedCandidateFwd.h"
#include "DataFormats/Math/interface/Point3D.h"
#include "DataFormats/Common/interface/RefProd.h"
#include "DataFormats/Common/interface/Ref.h"
#include "DataFormats/Common/interface/RefVector.h"
#include "FWCore/Framework/interface/ESWatcher.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "TF1.h"
#include "TFile.h"
#include "TH1.h"
#include "TH1I.h"
#include "TH2D.h"
#include "TH2F.h"
#include "TMath.h"
#include "TROOT.h"
#include "TRandom.h"
#include "TTree.h"
#include <cstdlib>
#include <ctime>

#include <vector>
using std::rand;
using std::vector;
using namespace std;
#include "RecoHI/HiEvtPlaneAlgos/interface/HiEvtPlaneFlatten.h"
#include "RecoHI/HiEvtPlaneAlgos/interface/HiEvtPlaneList.h"
#include "RecoHI/HiEvtPlaneAlgos/interface/LoadEPDB.h"
using namespace hi;
using namespace reco;
using namespace edm;

//
// class declaration
//
namespace{
	class CheckFlattening : public edm::one::EDAnalyzer<> {
	public:
  		CheckFlattening(const edm::ParameterSet& iPSet);
  		~CheckFlattening() override;

	private:
  		void beginJob() override;
  		void analyze(const edm::Event&, const edm::EventSetup&) override;
  		void endJob() override;
  // ----------member data ---------------------------
  		int eporder_;

  		unsigned int runno_;
  		edm::InputTag inputPlanesTag_;
  		std::string centralityVariable_;
  		edm::InputTag centralityBinTag_;
  		edm::InputTag vertexTag_;
  		double minvtx_;
  		double maxvtx_;
  		int flatnvtxbins_;
  		double flatminvtx_;
  		double flatdelvtx_;
  		int FlatOrder_;
  		int NumFlatBins_;
  		int CentBinCompression_;
  		double caloCentRef_;
  		double caloCentRefWidth_;
  		std::string centralityMC_;

  		int bin;
  		int cbin;
  		double flatmaxvtx_;
  		edm::Service<TFileService> fs;
  		edm::ESWatcher<HeavyIonRcd> hiWatcher;
  		edm::ESWatcher<HeavyIonRPRcd> hirpWatcher;

  		edm::EDGetTokenT<int> centralityBinToken;

  		std::string centralityLabel_;
  		edm::InputTag centralityTag_;
  		edm::EDGetTokenT<reco::Centrality> centralityToken;
  		edm::Handle<reco::Centrality> centrality_;

  		edm::Handle<int> cbin_;

  		edm::EDGetTokenT<std::vector<reco::Vertex>> vertexToken;
  		edm::Handle<VertexCollection> vertexCollection_;
  		edm::Handle<std::vector<reco::Vertex>> vertex_;

  		edm::EDGetTokenT<reco::EvtPlaneCollection> inputPlanesToken;
  		edm::Handle<reco::EvtPlaneCollection> inputPlanes_;
  		edm::ESGetToken<RPFlatParams, HeavyIonRPRcd> flatparmsToken_;

  		int nCentBins_;
  		//TFile *  frecenter;
  		int vs_sell;  // vertex collection size
  		float vzr_sell;
  		float vzErr_sell;
  		TH1D* hcent;
  		TH1D* hcentbins;
  		TH1D* hvtx;
  		TH1D* hvtx20;
  		double centval;
  		double vtx;
  		int noff;
  		int caloCentRefMinBin_;
  		int caloCentRefMaxBin_;
  		Double_t epang[NumEPNames];
  		Double_t epsin[NumEPNames];
  		Double_t epcos[NumEPNames];

  		Double_t epang_orig[NumEPNames];
  		Double_t epsin_orig[NumEPNames];
  		Double_t epcos_orig[NumEPNames];

  		Double_t epang_RecenterOnly[NumEPNames];
  		Double_t epsin_RecenterOnly[NumEPNames];
  		Double_t epcos_RecenterOnly[NumEPNames];

  		Double_t epang_NoWgt[NumEPNames];
  		Double_t epsin_NoWgt[NumEPNames];
  		Double_t epcos_NoWgt[NumEPNames];

  		Double_t sumw[NumEPNames];
  		Double_t sumw2[NumEPNames];

  		Double_t qx[NumEPNames];
  		Double_t qy[NumEPNames];
  		Double_t q[NumEPNames];
  		Double_t vn[NumEPNames];
  		Double_t epmult[NumEPNames];

  		Double_t rescor[NumEPNames];
  		Double_t rescorErr[NumEPNames];

  		string rpnames[NumEPNames];
  		TH1D* PsiRaw[NumEPNames];
  		TH1D* Psi[NumEPNames];
  		TH2D* FlatVsRaw[NumEPNames];

  		TTree* tree;

  		HiEvtPlaneFlatten* flat[NumEPNames];
  		bool loadDB_;
  		bool FirstEvent_;

  		bool Branch_Cent;
  		bool Branch_Vtx;
  		bool Branch_epang;
  		bool Branch_epang_orig;
  		bool Branch_epang_RecenterOnly;
  		bool Branch_NoWgt;
  		bool Branch_epsin;
  		bool Branch_epcos;
  		bool Branch_epsin_orig;
  		bool Branch_epcos_orig;
  		bool Branch_epsin_RecenterOnly;
  		bool Branch_epcos_RecenterOnly;
  		bool Branch_epsin_NoWgt;
  		bool Branch_epcos_NoWgt;
  		bool Branch_sumw;
  		bool Branch_sumw2;
  		bool Branch_qx;
  		bool Branch_qy;
  		bool Branch_q;
  		bool Branch_mult;
  		bool Branch_Run;
  		bool Branch_Rescor;
  		bool Branch_RescorErr;
  		bool Branch_vn;
	};

	//
	// constructors and destructor
	//
	CheckFlattening::CheckFlattening(const edm::ParameterSet& iConfig)
    	: runno_(0),
      	inputPlanesTag_(iConfig.getParameter<edm::InputTag>("inputPlanesTag")),
      	centralityVariable_(iConfig.getParameter<std::string>("centralityVariable")),
      	centralityBinTag_(iConfig.getParameter<edm::InputTag>("centralityBinTag")),
      	vertexTag_(iConfig.getParameter<edm::InputTag>("vertexTag")),
      	minvtx_(iConfig.getParameter<double>("minvtx")),
      	maxvtx_(iConfig.getParameter<double>("maxvtx")),
      	flatnvtxbins_(iConfig.getParameter<int>("flatnvtxbins")),
      	flatminvtx_(iConfig.getParameter<double>("flatminvtx")),
      	flatdelvtx_(iConfig.getParameter<double>("flatdelvtx")),
      	FlatOrder_(iConfig.getParameter<int>("FlatOrder")),
      	NumFlatBins_(iConfig.getParameter<int>("NumFlatBins")),
      	CentBinCompression_(iConfig.getParameter<int>("CentBinCompression")),
      	caloCentRef_(iConfig.getParameter<double>("caloCentRef")),
      	caloCentRefWidth_(iConfig.getParameter<double>("caloCentRefWidth"))

	{
  	loadDB_ = kTRUE;
  	flatparmsToken_ = esConsumes();
  	FirstEvent_ = kTRUE;

  	inputPlanesToken = consumes<reco::EvtPlaneCollection>(inputPlanesTag_);
  	if (iConfig.exists("nonDefaultGlauberModel")) {
    	centralityMC_ = iConfig.getParameter<std::string>("nonDefaultGlauberModel");
  	}
  	centralityLabel_ = centralityVariable_ + centralityMC_;
  	centralityBinToken = consumes<int>(centralityBinTag_);
  	vertexToken = consumes<std::vector<reco::Vertex>>(vertexTag_);

  	Branch_Cent = iConfig.getUntrackedParameter<bool>("Branch_Cent", true);
  	Branch_Vtx = iConfig.getUntrackedParameter<bool>("Branch_Vtx", true);
  	Branch_epang = iConfig.getUntrackedParameter<bool>("Branch_epang", true);
  	Branch_epang_orig = iConfig.getUntrackedParameter<bool>("Branch_epang_orig", true);
  	Branch_epang_RecenterOnly = iConfig.getUntrackedParameter<bool>("Branch_epang_RecenterOnly", true);
  	Branch_NoWgt = iConfig.getUntrackedParameter<bool>("Branch_NoWgt", true);
  	Branch_epsin = iConfig.getUntrackedParameter<bool>("Branch_epsin", true);
  	Branch_epcos = iConfig.getUntrackedParameter<bool>("Branch_epcos", true);
  	Branch_epsin_orig = iConfig.getUntrackedParameter<bool>("Branch_epsin_orig", true);
  	Branch_epcos_orig = iConfig.getUntrackedParameter<bool>("Branch_epcos_orig", true);
  	Branch_epsin_RecenterOnly = iConfig.getUntrackedParameter<bool>("Branch_epsin_RecenterOnly", true);
  	Branch_epcos_RecenterOnly = iConfig.getUntrackedParameter<bool>("Branch_epcos_RecenterOnly", true);
  	Branch_epsin_NoWgt = iConfig.getUntrackedParameter<bool>("Branch_epsin_NoWgt", true);
  	Branch_epcos_NoWgt = iConfig.getUntrackedParameter<bool>("Branch_epcos_NoWgt", true);
  	Branch_sumw = iConfig.getUntrackedParameter<bool>("Branch_sumw", true);
  	Branch_sumw2 = iConfig.getUntrackedParameter<bool>("Branch_sumw2", true);
  	Branch_qx = iConfig.getUntrackedParameter<bool>("Branch_qx", true);
  	Branch_qy = iConfig.getUntrackedParameter<bool>("Branch_qy", true);
  	Branch_q = iConfig.getUntrackedParameter<bool>("Branch_q", true);
  	Branch_mult = iConfig.getUntrackedParameter<bool>("Branch_mult", true);
  	Branch_Run = iConfig.getUntrackedParameter<bool>("Branch_Run", true);
  	Branch_Rescor = iConfig.getUntrackedParameter<bool>("Branch_Rescor", true);
  	Branch_RescorErr = iConfig.getUntrackedParameter<bool>("Branch_RescorErr", true);
  	Branch_vn = iConfig.getUntrackedParameter<bool>("Branch_vn", true);
  	hcent = fs->make<TH1D>("cent", "cent", 100, 0, 100);
  	hcent->SetXTitle("Centrality Bin");
  	hcent->SetYTitle("Counts");
  	hvtx = fs->make<TH1D>("vtx", "vtx", 200, -20, 20);
  	hvtx->SetXTitle("Vertex (cm)");
  	hvtx->SetYTitle("Counts");

  	hcentbins = fs->make<TH1D>("centbins", "centbins", 200, 0, 200);
  	hcentbins->SetXTitle("Centrality Bin");
  	hcentbins->SetYTitle("Counts");

  	TString epnames = EPNames[0].data();
  	epnames = epnames + "/D";
  	for (int i = 0; i < NumEPNames; i++) {
    		if (i > 0)
      		epnames = epnames + ":" + EPNames[i].data() + "/D";
    		TFileDirectory subdir = fs->mkdir(Form("%s", EPNames[i].data()));
    		Double_t psirange = 4;
    		if (EPOrder[i] == 2)
      		psirange = 2;
    		if (EPOrder[i] == 3)
      		psirange = 1.5;
    		if (EPOrder[i] == 4)
      		psirange = 1;
    		if (EPOrder[i] == 5)
      		psirange = 0.8;
    		if (EPOrder[i] == 6)
      		psirange = 0.6;
    		if (EPOrder[i] == 7)
      		psirange = 0.5;

    		PsiRaw[i] = subdir.make<TH1D>(
        		Form("PsiRaw_%s", EPNames[i].data()), Form("PsiRaw_%s", EPNames[i].data()), 800, -psirange, psirange);
    		Psi[i] = subdir.make<TH1D>(
        		Form("Psi_%s", EPNames[i].data()), Form("Psi_%s", EPNames[i].data()), 800, -psirange, psirange);
    		FlatVsRaw[i] = subdir.make<TH2D>(Form("FlatVsRaw_%s", EPNames[i].data()),
                                     Form("FlatVsRaw_%s", EPNames[i].data()),
                                     200,
                                     -psirange,
                                     psirange,
                                     200,
                                     -psirange,
                                     psirange);
    		FlatVsRaw[i]->SetOption("colz");
    		FlatVsRaw[i]->SetXTitle("#Psi_{raw}");
    		FlatVsRaw[i]->SetYTitle("#Psi_{flat}");

    		flat[i] = new HiEvtPlaneFlatten();
    		flat[i]->init(FlatOrder_, NumFlatBins_, flatnvtxbins_, flatminvtx_, flatdelvtx_, EPNames[i], EPOrder[i]);
  	}

  	tree = fs->make<TTree>("tree", "EP tree");

  	if (Branch_Cent)
    		tree->Branch("Cent", &centval, "cent/D");
  	if (Branch_Vtx)
    		tree->Branch("Vtx", &vtx, "vtx/D");
  	if (Branch_epang)
    		tree->Branch("epang", &epang, epnames.Data());
  	if (Branch_epang_orig)
    		tree->Branch("epang_orig", &epang_orig, epnames.Data());
  	if (Branch_epang_RecenterOnly)
    		tree->Branch("epang_RecenterOnly", &epang_RecenterOnly, epnames.Data());
  	if (Branch_NoWgt)
    		tree->Branch("epang_NoWgt", &epang_NoWgt, epnames.Data());

  	if (Branch_epsin)
    		tree->Branch("epsin", &epsin, epnames.Data());
  	if (Branch_epcos)
    		tree->Branch("epcos", &epcos, epnames.Data());
  	if (Branch_epsin_orig)
    		tree->Branch("epsin_orig", &epsin_orig, epnames.Data());
  	if (Branch_epcos_orig)
    		tree->Branch("epcos_orig", &epcos_orig, epnames.Data());
  	if (Branch_epsin_RecenterOnly)
    		tree->Branch("epsin_RecenterOnly", &epsin_RecenterOnly, epnames.Data());
  	if (Branch_epcos_RecenterOnly)
    		tree->Branch("epcos_RecenterOnly", &epcos_RecenterOnly, epnames.Data());
  	if (Branch_epsin_NoWgt)
    		tree->Branch("epsin_NoWgt", &epsin_NoWgt, epnames.Data());
  	if (Branch_epcos_NoWgt)
    		tree->Branch("epcos_NoWgt", &epcos_NoWgt, epnames.Data());
  	if (Branch_sumw)
    		tree->Branch("sumw", &sumw, epnames.Data());
  	if (Branch_sumw2)
    		tree->Branch("sumw2", &sumw2, epnames.Data());
  	if (Branch_qx)
    		tree->Branch("qx", &qx, epnames.Data());
  	if (Branch_qy)
    		tree->Branch("qy", &qy, epnames.Data());
  	if (Branch_q)
    		tree->Branch("q", &q, epnames.Data());
  	if (Branch_mult)
    		tree->Branch("mult", &epmult, epnames.Data());
  	if (Branch_Run)
    		tree->Branch("Run", &runno_, "run/i");
  	if (Branch_Rescor)
    		tree->Branch("Rescor", &rescor, epnames.Data());
  	if (Branch_RescorErr)
    		tree->Branch("RescorErr", &rescorErr, epnames.Data());
  	if (Branch_vn)
    		tree->Branch("vn", &vn, epnames.Data());
	}

	CheckFlattening::~CheckFlattening() {
  	// do anything here that needs to be done at desctruction time
  	// (e.g. close files, deallocate resources etc.)
	}

	//
	// member functions
	//

	// ------------ method called to for each event  ------------
	void CheckFlattening::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
  	using namespace edm;
  	using namespace std;
  	using namespace reco;

  	runno_ = iEvent.id().run();
  	if (hiWatcher.check(iSetup)) {
    		//
    		//Get Size of Centrality Table
    		//
    		//edm::ESHandle<CentralityTable> centDB_;
    		//iSetup.get<HeavyIonRcd>().get(centralityLabel_, centDB_);
    		//nCentBins_ = (int)centDB_->m_table.size();
    		nCentBins_ = 200;
    		for (int i = 0; i < NumEPNames; i++) {
      			flat[i]->setCaloCentRefBins(-1, -1);
    		}
  	}
  	if (hirpWatcher.check(iSetup)) {
    		//
    		//Get flattening parameter file.
    		//
    		//edm::ESHandle<RPFlatParams> flatparmsDB_;
    		//iSetup.get<HeavyIonRPRcd>().get(flatparmsDB_);
    		LoadEPDB db(iSetup.getData(flatparmsToken_), flat);
    		if (!db.IsSuccess()) {
      			std::cout << "Flattening db inconsistancy, will continue without: " << std::endl;
      			loadDB_ = kFALSE;
    		}
  	}
  	//
  	//Get Centrality
  	//
  	bin = -1;
  	cbin = -1;
  	edm::Handle<int> cbin_;
  	iEvent.getByToken(centralityBinToken, cbin_);
  	cbin = *cbin_;
  	centval = cbin / 2.;
  	bin = cbin / CentBinCompression_;
  	//
  	//Get Vertex
  	//
  	edm::Handle<reco::VertexCollection> vertices;
  	iEvent.getByToken(vertexToken, vertices);

  	//best vertex
  	double bestvz = -999.9, bestvx = -999.9, bestvy = -999.9;
  	const reco::Vertex& vtx = (*vertices)[0];
  	bestvz = vtx.z();
  	bestvx = vtx.x();
  	bestvy = vtx.y();
  	math::XYZPoint bestvtx(bestvx, bestvy, bestvz);

  	flatmaxvtx_ = flatminvtx_ + flatnvtxbins_ * flatdelvtx_;
  	if (bestvz < minvtx_ || bestvz > maxvtx_)
    		return;
  	hcent->Fill(centval);
  	hcentbins->Fill(cbin);
  	hvtx->Fill(bestvz);
  	//Get Event Planes
  	//
  	iEvent.getByToken(inputPlanesToken, inputPlanes_);

  	if (!inputPlanes_.isValid()) {
    		cout << "Error! Can't get hiEvtPlaneFlat product!" << endl;
    		return;
  	}

  	Int_t indx = 0;
  	for (EvtPlaneCollection::const_iterator rp = inputPlanes_->begin(); rp != inputPlanes_->end(); rp++) {
    		if (indx != rp->indx()) {
      			cout << "EP inconsistency found. Abort." << endl;
      			return;
    		}
    		epang[indx] = rp->angle(2);
    		epsin[indx] = rp->sumSin(2);
    		epcos[indx] = rp->sumCos(2);

    		epang_orig[indx] = rp->angle(0);
    		epsin_orig[indx] = rp->sumSin(0);
    		epcos_orig[indx] = rp->sumCos(0);

    		epang_RecenterOnly[indx] = rp->angle(1);
    		epsin_RecenterOnly[indx] = rp->sumSin(1);
    		epcos_RecenterOnly[indx] = rp->sumCos(1);

    		epang_NoWgt[indx] = rp->angle(3);
    		epsin_NoWgt[indx] = rp->sumSin(3);
    		epcos_NoWgt[indx] = rp->sumCos(3);

    		qx[indx] = rp->qx();
    		qy[indx] = rp->qy();
    		q[indx] = rp->q();
    		vn[indx] = rp->vn(0);
    		sumw[indx] = rp->sumw();
    		sumw2[indx] = rp->sumw2();
    		epmult[indx] = (double)rp->mult();

    		if (Branch_Rescor || Branch_RescorErr) {
      			rescor[indx] = flat[indx]->getCentRes1((int)centval);
      			rescorErr[indx] = flat[indx]->getCentResErr1((int)centval);
    		}
    		if (centval <= 80 && rp->angle() > -5) {
      			Psi[indx]->Fill(rp->angle());
      			PsiRaw[indx]->Fill(rp->angle(0));
      			FlatVsRaw[indx]->Fill(rp->angle(0), rp->angle());
    		}
    		++indx;
  	}

  	tree->Fill();
   }

	// ------------ method called once each job just before starting event loop  ------------
	void CheckFlattening::beginJob() {}

	// ------------ method called once each job just after ending the event loop  ------------
	void CheckFlattening::endJob() {}
}
//define this as a plug-in
DEFINE_FWK_MODULE(CheckFlattening);
