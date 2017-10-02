import FWCore.ParameterSet.Config as cms
from FWCore.ParameterSet.VarParsing import VarParsing

from Configuration.StandardSequences.Eras import eras

process = cms.Process('TEST',eras.Run2_2017)
options = VarParsing()
options.register('runNumber', 1, VarParsing.multiplicity.singleton, VarParsing.varType.int, 'Run to analyze')
options.register('outputFile', 'ttGeometry.root', VarParsing.multiplicity.singleton, VarParsing.varType.string, 'Output File')
options.parseArguments()

# import of standard configurations
process.load('Configuration.StandardSequences.Services_cff')
process.load('FWCore.MessageService.MessageLogger_cfi')
process.load('Configuration.StandardSequences.GeometryRecoDB_cff')
process.load('Configuration.Geometry.GeometrySimDB_cff')
process.load('Configuration.StandardSequences.MagneticField_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')

# Would have come from sim hcal tp imports
process.HcalTrigTowerGeometryESProducer = cms.ESProducer("HcalTrigTowerGeometryESProducer")

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(1)
)

# Input source
process.source = cms.Source("EmptySource",
    firstRun = cms.untracked.uint32(options.runNumber),
)

process.options = cms.untracked.PSet(
    wantSummary = cms.untracked.bool(False),
)

# Output definition


# Other statements
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, '80X_mcRun2_asymptotic_v14', '')

process.dump = cms.EDAnalyzer("DumpCaloGeometry")

# Path and EndPath definitions
process.p = cms.Path(process.dump)

process.TFileService = cms.Service(
    "TFileService", fileName = cms.string(options.outputFile),
    closeFileFast = cms.untracked.bool(True)
)

