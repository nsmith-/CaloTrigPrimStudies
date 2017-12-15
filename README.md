## Installation
```bash
# Any 90X+ CMSSW should work ok
cmsrel CMSSW_9_4_1
cd CMSSW_9_4_1/src
cmsenv
git cms-init
mkdir -p L1Trigger && cd L1Trigger 
git clone git@github.com:nsmith-/CaloTrigPrimStudies.git
cd ..
scram b
cd L1Trigger/CaloTrigPrimStudies/test
```

## Running the geometry dumper
Just run `cmsRun dumpGeometry_cfg.py`
Edit the same file and stick the appropriate global tag in, as it will affect which HCAL geometry is used.

Produces `ttGeometry.root`

See also: https://docs.google.com/spreadsheets/d/10mGBiP377rHgFIRDRz668yWnjQ91JuZAZOIPVv6NHyw/edit#gid=841503941
