import os
import sys
import FWCore.ParameterSet.Config as cms
import FWCore.ParameterSet.VarParsing as VarParsing

from Configuration.StandardSequences.Eras import eras
from Configuration.Eras.Era_Run3_pp_on_PbPb_2025_cff import Run3_pp_on_PbPb_2025

process = cms.Process("FlatCalib", Run3_pp_on_PbPb_2025)

ivars = VarParsing.VarParsing('analysis')

ivars.register('outfile',
               'calib.root',
		VarParsing.VarParsing.multiplicity.singleton,
		VarParsing.VarParsing.varType.string,
                "output file name")

ivars.register ('aodType',
                'MiniAOD',
                mult=ivars.multiplicity.singleton,
                mytype=ivars.varType.string,
                info="AOD/testAOD/MiniAOD/testMiniAOD")

ivars.register ('repFile',
                'root://cmsxrootd.fnal.gov//store',
                mult=ivars.multiplicity.singleton,
                mytype=ivars.varType.string,
                info="Data file to be replayed. Null if crab submission.")

ivars.register ('inputType',
                'Data',
                mult=ivars.multiplicity.singleton,
                mytype=ivars.varType.string,
                info="MC or Data: selects which input files to use for MiniAOD mode")


ivars.parseArguments()

if ivars.outfile == 'calib.root':  # still at default -> not user-overridden
    if ivars.inputType == 'Data':
        ivars.outfile = 'calibData.root'
    else:
        ivars.outfile = 'calibMC.root'

        


process.load('Configuration.StandardSequences.Services_cff')
process.load("CondCore.CondDB.CondDB_cfi")
process.load('FWCore.MessageService.MessageLogger_cfi')
process.load('Configuration.StandardSequences.GeometryDB_cff')
process.load('Configuration.StandardSequences.MagneticField_38T_cff')
process.load('Configuration.StandardSequences.EndOfProcess_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')

process.load('HeavyIonsAnalysis.EventAnalysis.skimanalysis_cfi')
process.load('HeavyIonsAnalysis.EventAnalysis.collisionEventSelection_cff')
process.load('HeavyIonsAnalysis.EventAnalysis.hffilterPF_cfi')
process.load('HeavyIonsAnalysis.EventAnalysis.clusterCompatibilityFilter_cfi')
process.load('HeavyIonsAnalysis.EventAnalysis.hievtanalyzer_data_cfi')
process.load('HeavyIonsAnalysis.EventAnalysis.hltanalysis_cfi')

process.load("RecoHI.HiEvtPlaneAlgos.HiEvtPlane_cfi")
process.load("RecoHI.HiEvtPlaneAlgos.EvtPlaneFilter_cfi")
process.load("RecoHI.HiEvtPlaneAlgos.hiEvtPlaneFlat_cfi")
process.load("HeavyIonsAnalysis.HiEvtPlaneCalib.evtplanecalibtree_cfi")

from Configuration.AlCa.GlobalTag import GlobalTag
# Select global tag based on inputType:
#   Data: 151X_dataRun3_Prompt_v1 (2025 PromptReco GT) + offline HFtowers centrality table below
#   MC:   151X_mcRun3_2025_realistic_HI_v5 (HINPbPbWinter25 GT)
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

process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(10000))
process.MessageLogger.cerr.FwkReport.reportEvery = 1000


if ivars.aodType == 'testMiniAOD':
    process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring(
            'file:'+ivars.repFile
        ),
        inputCommands=cms.untracked.vstring(
            'keep *',
            'drop *_hiEvtPlane_*_*'
        )
    )

if ivars.aodType == 'MiniAOD':
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

process.TFileService = cms.Service("TFileService",
        fileName = cms.string(ivars.outfile)
    )
if ivars.aodType == 'MiniAOD':
    process.hiEvtPlane.trackTag = cms.InputTag("packedPFCandidates")
    process.hiEvtPlane.vertexTag = cms.InputTag("offlineSlimmedPrimaryVertices")
 
process.dump = cms.EDAnalyzer("EventContentAnalyzer")
process.evtPlaneCalibTree.centralityVariable = process.hiEvtPlane.centralityVariable
process.evtPlaneCalibTree.centralityBinTag = process.hiEvtPlane.centralityBinTag
process.evtPlaneCalibTree.vertexTag = process.hiEvtPlane.vertexTag
process.evtPlaneCalibTree.caloTag = process.hiEvtPlane.caloTag
process.evtPlaneCalibTree.castorTag = process.hiEvtPlane.castorTag
process.evtPlaneCalibTree.trackTag = process.hiEvtPlane.trackTag
process.evtPlaneCalibTree.lostTag = process.hiEvtPlane.lostTag
process.evtPlaneCalibTree.chi2MapTag = process.hiEvtPlane.chi2MapTag
process.evtPlaneCalibTree.chi2MapLostTag = process.hiEvtPlane.chi2MapLostTag
process.evtPlaneCalibTree.nonDefaultGlauberModel = process.hiEvtPlane.nonDefaultGlauberModel
process.evtPlaneCalibTree.loadDB = process.hiEvtPlane.loadDB
process.evtPlaneCalibTree.minet = process.hiEvtPlane.minet
process.evtPlaneCalibTree.maxet = process.hiEvtPlane.maxet
process.evtPlaneCalibTree.minpt = process.hiEvtPlane.minpt
process.evtPlaneCalibTree.flatnvtxbins = process.hiEvtPlane.flatnvtxbins
process.evtPlaneCalibTree.flatminvtx = process.hiEvtPlane.flatminvtx
process.evtPlaneCalibTree.flatdelvtx = process.hiEvtPlane.flatdelvtx
process.evtPlaneCalibTree.dzdzerror = process.hiEvtPlane.dzdzerror
process.evtPlaneCalibTree.d0d0error = process.hiEvtPlane.d0d0error
process.evtPlaneCalibTree.pterror = process.hiEvtPlane.pterror
process.evtPlaneCalibTree.chi2perlayer = process.hiEvtPlane.chi2perlayer
process.evtPlaneCalibTree.dzdzerror_pix = process.hiEvtPlane.dzdzerror_pix
process.evtPlaneCalibTree.chi2 = process.hiEvtPlane.chi2
process.evtPlaneCalibTree.nhitsValid = process.hiEvtPlane.nhitsValid
process.evtPlaneCalibTree.FlatOrder = process.hiEvtPlane.FlatOrder
process.evtPlaneCalibTree.NumFlatBins = process.hiEvtPlane.NumFlatBins
process.evtPlaneCalibTree.caloCentRef = process.hiEvtPlane.caloCentRef
process.evtPlaneCalibTree.caloCentRefWidth = process.hiEvtPlane.caloCentRefWidth
process.evtPlaneCalibTree.CentBinCompression = process.hiEvtPlane.CentBinCompression
process.evtPlaneCalibTree.cutEra = process.hiEvtPlane.cutEra

process.hiEvtPlane.loadDB = cms.bool(False)

if ivars.aodType == 'testMiniAOD' or ivars.aodType == 'MiniAOD':
    if ivars.inputType == 'Data':
        process.p = cms.Path(process.HLT_filters * process.event_filters * process.centralityBin * process.hiEvtPlane * process.evtPlaneCalibTree)
    else:
        process.p = cms.Path(process.event_filters * process.centralityBin * process.hiEvtPlane * process.evtPlaneCalibTree)


