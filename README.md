Installation:
-------------
```bash
cmsrel CMSSW_9_2_12
cd CMSSW_9_2_12/src
cmsenv
git cms-init
mkdir Analysis && cd Analysis
git clone git@github.com:nsmith-/DumpCaloGeometry.git
cd ..
scram b
cd Analysis/DumpCaloGeometry/test
```
