// -*- C++ -*-
//
// Package:    Analysis/DumpCaloGeometry
// Class:      DumpCaloGeometry
// 
/**\class DumpCaloGeometry DumpCaloGeometry.cc Analysis/DumpCaloGeometry/plugins/DumpCaloGeometry.cc

 Description: 

 Implementation:
     
*/
//
// Original Author:  Nicholas Charles Smith
//         Created:  Thu, 09 Jun 2016 10:27:00 GMT
//
//


// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ESHandle.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DataFormats/CaloTowers/interface/CaloTowerDetId.h"
#include "DataFormats/DetId/interface/DetId.h"
#include "DataFormats/EcalDetId/interface/EBDetId.h"
#include "DataFormats/EcalDetId/interface/EEDetId.h"
#include "DataFormats/EcalDetId/interface/EcalTrigTowerDetId.h"
#include "DataFormats/EcalDetId/interface/EcalSubdetector.h"
#include "DataFormats/HcalDetId/interface/HcalDetId.h"
#include "DataFormats/HcalDetId/interface/HcalTrigTowerDetId.h"
#include "DataFormats/HcalDetId/interface/HcalSubdetector.h"

#include "Geometry/CaloEventSetup/interface/CaloTopologyRecord.h"
#include "Geometry/CaloGeometry/interface/CaloCellGeometry.h"
#include "Geometry/CaloGeometry/interface/CaloGeometry.h"
#include "Geometry/CaloGeometry/interface/CaloSubdetectorGeometry.h"
#include "Geometry/CaloTopology/interface/CaloSubdetectorTopology.h"
#include "Geometry/CaloTopology/interface/CaloTopology.h"
#include "Geometry/CaloTopology/interface/CaloTowerConstituentsMap.h"
#include "Geometry/CaloTopology/interface/EcalTrigTowerConstituentsMap.h"
#include "Geometry/HcalTowerAlgo/interface/HcalTrigTowerGeometry.h"
#include "Geometry/Records/interface/CaloGeometryRecord.h"
#include "Geometry/Records/interface/IdealGeometryRecord.h"

#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "TTree.h"

class DumpCaloGeometry : public edm::one::EDAnalyzer<edm::one::SharedResources, edm::one::WatchRuns>  {
  public:
    explicit DumpCaloGeometry(const edm::ParameterSet&);
    ~DumpCaloGeometry();

    static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);


  private:
    virtual void beginJob() override;
    virtual void beginRun(const edm::Run&, const edm::EventSetup&) override;
    virtual void endRun(edm::Run const&, edm::EventSetup const&) override {}
    virtual void analyze(const edm::Event&, const edm::EventSetup&) override;
    virtual void endJob() override;

    edm::ESHandle<CaloGeometry> caloGeometry_;
    edm::ESHandle<EcalTrigTowerConstituentsMap> ecalTriggerTowerMap_;
    edm::ESHandle<HcalTrigTowerGeometry> hcalTriggerTowerMap_;

    TTree * geoTree_;
    struct {
      UInt_t detId;
      UChar_t det; 
      Int_t ieta;
      Int_t iphi;
      Float_t area;
      Int_t nCaloTowers;
      std::vector<UInt_t> caloTower_detId;
      std::vector<Float_t> caloTower_rho;
      std::vector<Float_t> caloTower_z;
      std::vector<Float_t> caloTower_eta;
      std::vector<Float_t> caloTower_phi;
      std::vector<Float_t> caloTower_depth;
      std::vector<Float_t> caloTower_area;
    } geoRow_;

    void clearRow() {
      geoRow_.caloTower_detId.clear();
      geoRow_.caloTower_rho.clear();
      geoRow_.caloTower_z.clear();
      geoRow_.caloTower_eta.clear();
      geoRow_.caloTower_phi.clear();
      geoRow_.caloTower_depth.clear();
      geoRow_.caloTower_area.clear();
    };

    template<typename T>
    void fillFromMap(const std::map<T, std::vector<std::pair<const DetId, const CaloCellGeometry*>>>& ttMap) {
      for(const auto& tt : ttMap) {
        int absieta = std::abs(tt.first.ieta());
        double area{0.};
        for(const auto& detid_cell : tt.second) {
          auto&& cell = detid_cell.second;
          // Catch only first layer for HCAL
          if (
               tt.first.det() == DetId::Ecal
               or (absieta < 17 and cell->getPosition().perp() < 185.)
               or (absieta == 17 and cell->getPosition().perp() < 205.)
               or (absieta > 17 and absieta < 29 and std::fabs(cell->getPosition().z()) < 405.)
               or (absieta > 29 and absieta < 42 and std::fabs(cell->getPosition().z()) < 1120.)
              )
          {
            area += cell->etaSpan() * cell->phiSpan();
          }
        }
        geoRow_.detId = tt.first.rawId();
        geoRow_.det = tt.first.rawId()>>24;
        geoRow_.ieta = tt.first.ieta();
        geoRow_.iphi = tt.first.iphi();
        geoRow_.area = area;
        geoRow_.nCaloTowers = 0;
        for(const auto& detid_cell : tt.second) {
          geoRow_.caloTower_detId.push_back(detid_cell.first.rawId());
          auto&& cell = detid_cell.second;
          geoRow_.caloTower_rho.push_back(cell->getPosition().perp());
          geoRow_.caloTower_z.push_back(cell->getPosition().z());
          geoRow_.caloTower_eta.push_back(cell->etaPos());
          geoRow_.caloTower_phi.push_back(cell->phiPos());
          float depth = cell->getBackPoint().mag() - cell->getPosition().mag();
          geoRow_.caloTower_depth.push_back(depth);
          geoRow_.caloTower_area.push_back(cell->etaSpan() * cell->phiSpan());
          geoRow_.nCaloTowers++;
        }
        geoTree_->Fill();
        clearRow();
      }
    };
};

DumpCaloGeometry::DumpCaloGeometry(const edm::ParameterSet& iConfig)
{
  usesResource("TFileService");
  edm::Service<TFileService> fs;

  geoTree_ = fs->make<TTree>("geoTree", "Calo trigger tower geometry");
  geoTree_->Branch("detId", &geoRow_.detId, "detId/i");
  geoTree_->Branch("det", &geoRow_.det, "detId/b");
  geoTree_->Branch("ieta", &geoRow_.ieta, "ieta/I");
  geoTree_->Branch("iphi", &geoRow_.iphi, "iphi/I");
  geoTree_->Branch("area", &geoRow_.area, "area/F");
  geoTree_->Branch("nCaloTowers", &geoRow_.nCaloTowers, "nCaloTowers/i");
  geoTree_->Branch("caloTower_detId", &geoRow_.caloTower_detId);
  geoTree_->Branch("caloTower_rho", &geoRow_.caloTower_rho);
  geoTree_->Branch("caloTower_z", &geoRow_.caloTower_z);
  geoTree_->Branch("caloTower_eta", &geoRow_.caloTower_eta);
  geoTree_->Branch("caloTower_phi", &geoRow_.caloTower_phi);
  geoTree_->Branch("caloTower_depth", &geoRow_.caloTower_depth);
  geoTree_->Branch("caloTower_area", &geoRow_.caloTower_area);
}


DumpCaloGeometry::~DumpCaloGeometry()
{
}


void 
DumpCaloGeometry::beginJob()
{
}


void
DumpCaloGeometry::beginRun(const edm::Run& iRun, const edm::EventSetup& iSetup)
{
  iSetup.get<CaloGeometryRecord>().get(caloGeometry_);
  iSetup.get<IdealGeometryRecord>().get(ecalTriggerTowerMap_);
  iSetup.get<CaloGeometryRecord>().get(hcalTriggerTowerMap_);

  std::map<EcalTrigTowerDetId, std::vector<std::pair<const DetId, const CaloCellGeometry*>>> ebTTmap;
  auto ebGeometry = caloGeometry_->getSubdetectorGeometry(DetId::Ecal, EcalBarrel);
  for ( const auto& detId : ebGeometry->getValidDetIds() ) {
    const CaloCellGeometry * cell = ebGeometry->getGeometry(detId);
    EcalTrigTowerDetId towerDetId = ecalTriggerTowerMap_->towerOf(detId);
    ebTTmap[towerDetId].emplace_back(detId, cell);
  }


  std::map<EcalTrigTowerDetId, std::vector<std::pair<const DetId, const CaloCellGeometry*>>> eeTTmap;
  auto eeGeometry = caloGeometry_->getSubdetectorGeometry(DetId::Ecal, EcalEndcap);
  for ( const auto& detId : eeGeometry->getValidDetIds() ) {
    const CaloCellGeometry * cell = eeGeometry->getGeometry(detId);
    EcalTrigTowerDetId towerDetId = ecalTriggerTowerMap_->towerOf(detId);
    eeTTmap[towerDetId].emplace_back(detId, cell);
  }


  std::map<HcalTrigTowerDetId, std::vector<std::pair<const DetId, const CaloCellGeometry*>>> hbTTmap;
  auto hbGeometry = caloGeometry_->getSubdetectorGeometry(DetId::Hcal, HcalBarrel);
  for ( const auto& detId : hbGeometry->getValidDetIds() ) {
    const CaloCellGeometry * cell = hbGeometry->getGeometry(detId);
    auto towerDetIds = hcalTriggerTowerMap_->towerIds(detId);
    if ( towerDetIds.size() == 0 ) {
      std::cout << "No-tower hcal DetId" << std::endl;
      continue;
    }
    else if ( std::find(begin(towerDetIds), end(towerDetIds), detId) != end(towerDetIds) ) {
      continue;
    }
    HcalTrigTowerDetId towerDetId = * std::min_element(begin(towerDetIds), end(towerDetIds),
      [](auto a, auto b) {
        // a < b, prefer highest version lower iphi
        return a.version() > b.version() || ( a.version() == b.version() && a.iphi() < b.iphi() );
      }
    );
    hbTTmap[towerDetId].emplace_back(detId, cell);
  }

  fillFromMap(ebTTmap);
  fillFromMap(eeTTmap);
  fillFromMap(hbTTmap);

  // std::cout << "reading he" << std::endl;
  // auto heGeometry = caloGeometry_->getSubdetectorGeometry(DetId::Hcal, HcalEndcap);

  // std::cout << "reading hf" << std::endl;
  // auto hfGeometry = caloGeometry_->getSubdetectorGeometry(DetId::Hcal, HcalForward);

  // std::cout << "reading hcal tt" << std::endl;
  // auto hcalTriggerGeometry = caloGeometry_->getSubdetectorGeometry(DetId::Hcal, HcalTriggerTower);
}


void
DumpCaloGeometry::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  using namespace edm;

}


void 
DumpCaloGeometry::endJob() 
{
}


void
DumpCaloGeometry::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

DEFINE_FWK_MODULE(DumpCaloGeometry);
