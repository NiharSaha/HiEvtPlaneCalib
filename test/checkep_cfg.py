import FWCore.ParameterSet.Config as cms
import FWCore.ParameterSet.VarParsing as VarParsing
import os
import sys
ivars = VarParsing.VarParsing('standard')

ivars.register ('lumifile',
                '',
                mult=ivars.multiplicity.singleton,
                mytype=ivars.varType.string,
                info="lumi file")

ivars.register ('dbfile',
                'HeavyIonRPRcd_offline.db',
                mult=ivars.multiplicity.singleton,
                mytype=ivars.varType.string,
                info="dbfile file")

ivars.register ('checkfile',
                'check.root',
                mult=ivars.multiplicity.singleton,
                mytype=ivars.varType.string,
                info="check output file")

ivars.register ('aodType',
                'MiniAOD',
                mult=ivars.multiplicity.singleton,
                mytype=ivars.varType.string,
                info="AOD/TestAOD/MiniAOD/testMiniAOD")

ivars.register ('repFile',
                'infile.root',
                mult=ivars.multiplicity.singleton,
                mytype=ivars.varType.string,
                info="single file replay")

ivars.register ('inputType',
                'MC',
                mult=ivars.multiplicity.singleton,
                mytype=ivars.varType.string,
                info="MC or Data: selects which input files to use for MiniAOD mode")

ivars.parseArguments()

# Same era as calibtree_cfg.py: the pp_on_AA modifier sets the hiEvtPlane cuts
# (minet, minpt, caloTag, cutEra) used to derive the calibration
from Configuration.Eras.Era_Run3_pp_on_PbPb_2025_cff import Run3_pp_on_PbPb_2025
process = cms.Process("check", Run3_pp_on_PbPb_2025)
process.load('Configuration.StandardSequences.Services_cff')
process.load("CondCore.CondDB.CondDB_cfi")
process.load('FWCore.MessageService.MessageLogger_cfi')
process.load('Configuration.StandardSequences.GeometryDB_cff')
process.load('Configuration.StandardSequences.MagneticField_38T_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
process.load('Configuration.StandardSequences.EndOfProcess_cff')
process.load('HeavyIonsAnalysis.EventAnalysis.collisionEventSelection_cff')
process.load('HeavyIonsAnalysis.EventAnalysis.hffilterPF_cfi')

process.load("HeavyIonsAnalysis.HiEvtPlaneCalib.checkflattening_cfi")
process.load("RecoHI.HiEvtPlaneAlgos.HiEvtPlane_cfi")
process.load("RecoHI.HiEvtPlaneAlgos.hiEvtPlaneFlat_cfi")

from Configuration.AlCa.GlobalTag import GlobalTag
# Select global tag based on inputType:
#   Data: 151X_dataRun3_Prompt_v1 (2025 PromptReco GT) + offline HFtowers centrality table below
#   MC:   151X_mcRun3_2025_realistic_HI_v5 (HINPbPbWinter25 GT)
# Must match calibtree_cfg.py, otherwise centrality bins differ from the calibration
if ivars.inputType == 'Data':
    process.GlobalTag = GlobalTag(process.GlobalTag, '151X_dataRun3_Prompt_v1', '')
    # The 2025 Prompt GT only has the express HFtowers table; no 2025 offline table exists yet.
    # Use the offline PbPb table used for 2024 (its last IOV starts at 387853 and covers 2025).
    process.GlobalTag.toGet.extend([
        cms.PSet(record = cms.string("HeavyIonRcd"),
                 tag = cms.string("CentralityTable_HFtowers200_DataPbPb_offline_v0"),
                 connect = cms.string("frontier://FrontierProd/CMS_CONDITIONS"),
                 label = cms.untracked.string("HFtowers")
                 )
    ])
else:
    process.GlobalTag = GlobalTag(process.GlobalTag, '151X_mcRun3_2025_realistic_HI_v5', '')


process.load('RecoHI.HiCentralityAlgos.HiCentrality_cfi')
process.load("RecoHI.HiCentralityAlgos.CentralityBin_cfi")
process.centralityBin.Centrality = cms.InputTag("hiCentrality")
process.centralityBin.centralityVariable = cms.string("HFtowers")

#process.load('HeavyIonsAnalysis.Configuration.hfCoincFilter_cff')
#process.load('HeavyIonsAnalysis.Configuration.collisionEventSelection_cff')

from HLTrigger.HLTfilters.hltHighLevel_cfi import hltHighLevel
process.hltFilter = hltHighLevel.clone(
    HLTPaths = ["HLT_HIMinimumBias*"]
)
process.HLT_filters = cms.Sequence(process.hltFilter)
process.event_filters = cms.Sequence(
    process.primaryVertexFilter *
    process.clusterCompatibilityFilter *
    process.phfCoincFilterPF3Th5
)

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(ivars.maxEvents) )
process.MessageLogger.cerr.FwkReport.reportEvery=100

process.CondDB.connect = "sqlite_file:"+ivars.dbfile
process.PoolDBESSource = cms.ESSource("PoolDBESSource",
                                       process.CondDB,
                                       toGet = cms.VPSet(cms.PSet(record = cms.string('HeavyIonRPRcd'),
                                                                  tag = cms.string('HeavyIonRPRcd')
                                                                  )
                                                         )
                                      )
process.es_prefer_flatparms = cms.ESPrefer('PoolDBESSource','')

if ivars.aodType == 'testMiniAOD':
    process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring(
            'file:'+ivars.repFile
        ),
        inputCommands=cms.untracked.vstring(
            'keep *',
            'drop *_hiEvtPlane_*_*'
        )
    )
elif ivars.aodType == 'MiniAOD':
    if ivars.inputType == 'Data':
        # 2025 PbPb real data: HIRun2025A HIPhysicsRawPrime0 MINIAOD PromptReco-v1 (golden run 400243)
        process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring(
            '/store/hidata/HIRun2025A/HIPhysicsRawPrime0/MINIAOD/PromptReco-v1/000/400/243/00000/9f217f14-1ddf-41e7-99a3-aebc9a55a294.root'
            ),
            inputCommands=cms.untracked.vstring(
                'keep *',
                'drop *_hiEvtPlane_*_*'
            )
        )
    else:  # MC
        # 2025 PbPb MC: Hydjet Quenched MinBias MiniAODSIM (HINPbPbWinter25, 5.36 TeV)
        process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring(
            '/store/mc/HINPbPbWinter25MiniAOD/Hydjet_Quenched_MinBias_TuneCELLO_5p36TeV_pythia8/MINIAODSIM/NoPU_151X_mcRun3_2025_realistic_HI_v5-v2/2560000/e41e63ed-8934-4fe4-829e-8f5e40ee7ad7.root'
            ),
            inputCommands=cms.untracked.vstring(
                'keep *',
                'drop *_hiEvtPlane_*_*'
            )
        )

# Apply the lumi mask (local running; for CRAB use config.Data.lumiMask instead)
if ivars.lumifile.strip():
    import FWCore.PythonUtilities.LumiList as LumiList
    goodLumiSecs = LumiList.LumiList(filename = ivars.lumifile ).getCMSSWString().split(',')
    process.source.lumisToProcess = cms.untracked.VLuminosityBlockRange(goodLumiSecs)

process.TFileService = cms.Service("TFileService",
        fileName = cms.string(ivars.checkfile)
    )


process.dump = cms.EDAnalyzer("EventContentAnalyzer")

process.hiEvtPlane.trackTag = cms.InputTag("generalTracks")
process.hiEvtPlane.vertexTag = cms.InputTag("offlinePrimaryVertices")
process.hiEvtPlaneFlat.vertexTag = cms.InputTag("offlinePrimaryVertices")
if ivars.aodType == 'MiniAOD':
    process.hiEvtPlane.trackTag = cms.InputTag("packedPFCandidates")
    process.hiEvtPlane.vertexTag = cms.InputTag("offlineSlimmedPrimaryVertices")
    process.hiEvtPlaneFlat.vertexTag = cms.InputTag("offlineSlimmedPrimaryVertices")

process.hiEvtPlane.loadDB = cms.bool(True)
process.hiEvtPlaneFlat.centralityVariable=process.hiEvtPlane.centralityVariable
process.hiEvtPlaneFlat.vertexTag=process.hiEvtPlane.vertexTag
process.hiEvtPlaneFlat.flatminvtx=process.hiEvtPlane.flatminvtx
process.hiEvtPlaneFlat.flatnvtxbins=process.hiEvtPlane.flatnvtxbins
process.hiEvtPlaneFlat.flatdelvtx=process.hiEvtPlane.flatdelvtx
process.hiEvtPlaneFlat.FlatOrder=process.hiEvtPlane.FlatOrder
process.hiEvtPlaneFlat.CentBinCompression=process.hiEvtPlane.CentBinCompression
process.hiEvtPlaneFlat.caloCentRef=process.hiEvtPlane.caloCentRef
process.hiEvtPlaneFlat.caloCentRefWidth=process.hiEvtPlane.caloCentRefWidth

process.checkflattening.centralityVariable=process.hiEvtPlane.centralityVariable
process.checkflattening.vertexTag=process.hiEvtPlane.vertexTag
process.checkflattening.minvtx=process.hiEvtPlane.flatminvtx
process.checkflattening.flatnvtxbins=process.hiEvtPlane.flatnvtxbins
process.checkflattening.flatminvtx=process.hiEvtPlane.flatminvtx
process.checkflattening.flatdelvtx=process.hiEvtPlane.flatdelvtx
process.checkflattening.FlatOrder=process.hiEvtPlane.FlatOrder
process.checkflattening.CentBinCompression=process.hiEvtPlane.CentBinCompression
process.checkflattening.caloCentRef=process.hiEvtPlane.caloCentRef
process.checkflattening.caloCentRefWidth=process.hiEvtPlane.caloCentRefWidth

# Per-event tree content for Q-vector / EP validation:
#   level 0 = raw (_orig), level 1 = recentered (_RecenterOnly), level 2 = flattened (epang)
process.checkflattening.Branch_Run = cms.untracked.bool(True)
process.checkflattening.Branch_epang_orig = cms.untracked.bool(True)
process.checkflattening.Branch_epang_RecenterOnly = cms.untracked.bool(True)
process.checkflattening.Branch_epsin_orig = cms.untracked.bool(True)
process.checkflattening.Branch_epcos_orig = cms.untracked.bool(True)
process.checkflattening.Branch_epsin_RecenterOnly = cms.untracked.bool(True)
process.checkflattening.Branch_epcos_RecenterOnly = cms.untracked.bool(True)
process.checkflattening.Branch_sumw = cms.untracked.bool(True)
process.checkflattening.Branch_Rescor = cms.untracked.bool(False)
process.checkflattening.Branch_RescorErr = cms.untracked.bool(False)

process.dump = cms.EDAnalyzer("EventContentAnalyzer")

if ivars.aodType == 'testMiniAOD' or ivars.aodType == 'MiniAOD':
    if ivars.inputType == 'Data':
        process.p = cms.Path(process.HLT_filters * process.event_filters * process.hiEvtPlane * process.hiEvtPlaneFlat * process.checkflattening)
    else:
        process.p = cms.Path(process.event_filters * process.hiEvtPlane * process.hiEvtPlaneFlat * process.checkflattening)

