import FWCore.ParameterSet.VarParsing as VarParsing
import string
import subprocess
import os

if os.access("flatparms_new.db",os.F_OK)==True:
    ret = subprocess.Popen(['rm','flatparms_new.db'])
    ret.wait()
    
ivars = VarParsing.VarParsing('standard')

ivars.register ('outputFile',
                mult=ivars.multiplicity.singleton,
                mytype=ivars.varType.string,
                info="for testing")

ivars.register ('outtag',
                mult=ivars.multiplicity.singleton,
                mytype=ivars.varType.string,
                info="for testing")

ivars.register ('begin',
                mult=ivars.multiplicity.singleton,
                mytype=ivars.varType.int,
                info="for testing")

ivars.register ('end',
                mult=ivars.multiplicity.singleton,
                mytype=ivars.varType.int,
                info="for testing")

ivars.register ('rescorloc',
                mult=ivars.multiplicity.singleton,
                mytype=ivars.varType.string,
                info="loc of resolution corrections")

ivars.register ('infile',
                mult=ivars.multiplicity.singleton,
                mytype=ivars.varType.string,
                info="input file")

ivars.parseArguments()
import FWCore.ParameterSet.Config as cms

process = cms.Process("Demo")
process.load("CondCore.CondDB.CondDB_cfi")
process.load("FWCore.MessageService.MessageLogger_cfi")
process.load("HeavyIonsAnalysis.HiEvtPlaneCalib.moveparmstodb_cfi")

process.CondDB.connect = "sqlite_file:" + ivars.outputFile
process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(1) )

process.source = cms.Source("EmptyIOVSource",
    firstValue = cms.uint64(ivars.begin),
    lastValue = cms.uint64(ivars.end),
    timetype = cms.string('runnumber'),
    interval = cms.uint64(1)
)

process.PoolDBOutputService = cms.Service("PoolDBOutputService",
                                          process.CondDB,
                                          timetype = cms.untracked.string("runnumber"),
                                          toPut = cms.VPSet(cms.PSet(record = cms.string('HeavyIonRPRcd'),
                                                                     tag = cms.string(ivars.outtag)
                                                                     )
                                                            )
                                          )


process.moveparmstodb.rescorloc = ivars.rescorloc
process.moveparmstodb.infile = ivars.infile
process.moveparmstodb.outtag = ivars.outtag

process.p = cms.Path(process.moveparmstodb)
