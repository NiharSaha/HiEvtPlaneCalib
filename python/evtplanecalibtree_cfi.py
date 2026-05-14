import FWCore.ParameterSet.Config as cms

evtPlaneCalibTree = cms.EDAnalyzer("EvtPlaneCalibTree",
                            inputPlanesTag = cms.InputTag("hiEvtPlane",""),
                            centralityVariable = cms.string("HFtowers"),
                            centralityBinTag = cms.InputTag("centralityBin","HFtowers"),
                            vertexTag = cms.InputTag("offlinePrimaryVertices"),
                            caloTag = cms.InputTag("towerMaker"),
                            castorTag = cms.InputTag("CastorTowerReco"),
                            trackTag = cms.InputTag("generalTracks"),
                            lostTag = cms.InputTag("lostTracks"),
                            chi2MapTag = cms.InputTag("packedPFCandidateTrackChi2"),
                            chi2MapLostTag = cms.InputTag("lostTrackChi2"),
                            nonDefaultGlauberModel = cms.string(""),
                            loadDB = cms.bool(False),
                            minet = cms.double(0.01),
                            maxet = cms.double(-1),
                            minpt = cms.double(0.5),
                            maxpt = cms.double(3.0),
                            minvtx = cms.double(-15.),
                            maxvtx = cms.double(15.),
                            flatnvtxbins = cms.int32(10),
                            flatminvtx = cms.double(-15.0),
                            flatdelvtx = cms.double(3.0),
                            dzdzerror = cms.double(3.0),
                            d0d0error = cms.double(3.0),
                            pterror = cms.double(0.1),
                            chi2perlayer = cms.double(0.18),
                            dzdzerror_pix = cms.double(40.),
                            chi2 = cms.double(40.),
                            nhitsValid = cms.int32(11),
                            FlatOrder = cms.int32(9),
                            NumFlatBins = cms.int32(40),
                            caloCentRef = cms.double(-1),
                            caloCentRefWidth = cms.double(-1),
                            CentBinCompression = cms.int32(5),
                            cutEra = cms.int32(0), # 0:ppReco, 1:HIReco, 2:Pixel, 3: GenMC
                            chi2Map = cms.InputTag("packedPFCandidateTrackChi2"),
                            chi2MapLost = cms.InputTag("lostTrackChi2"),
                            )
                            




    
