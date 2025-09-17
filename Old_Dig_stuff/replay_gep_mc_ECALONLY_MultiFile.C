#include <iostream>
#include <unordered_map>
#include <vector>

#include "TSystem.h"
#include "THaGlobals.h"
#include "TString.h"
#include "TFile.h"
#include "TChain.h"

#include "THaEvData.h"
#include "THaRun.h"
#include "THaAnalyzer.h"
#include "THaVarList.h"
#include "THaInterface.h"
#include "THaGoldenTrack.h"

#include "SBSGEPEArm.h"
#include "SBSCDet.h"
#include "SBSECal.h"
#include "SBSHCal.h"
#include "SBSGEMSpectrometerTracker.h"
#include "SBSGEMPolarimeterTracker.h"
#include "SBSGEPRegionOfInterestModule.h"
#include "SBSGEPHeepCoinModule.h"
#include "SBSSimDecoder.h"

TDatime get_datime(uint gepconfig) {
  std::unordered_map<uint,TDatime> m = {
    {1, "2024-10-01 00:00:00"},
    {2, "2024-10-01 00:00:00"},
    {3, "2025-05-04 12:00:00"},
    {4, "2024-10-01 00:00:00"}};
  if (m.find(gepconfig)==m.end())
    throw std::invalid_argument("Invalid SBS config!! Valid options are: 1,2,3");
  return m[gepconfig];
}

void replay_gep_mc_ECALONLY(const std::vector<std::string>& filebases,
                            uint gepconfig, uint nev=-1, TString experiment="gep")
{
  // --- Setup detectors ---
  SBSGEPEArm* earm = new SBSGEPEArm("earm", "GEP electron arm");
  earm->AddDetector(new SBSECal("ecal", "ECal"));
  earm->AddDetector(new SBSCDet("cdet", "coordinate detector"));
  gHaApps->Add(earm);

  SBSEArm *harm = new SBSEArm("sbs","Hadron Arm with HCal");
  harm->AddDetector(new SBSHCal("hcal","HCAL"));
  harm->AddDetector(new SBSGEMSpectrometerTracker("gemFT","Front tracker"));
  harm->AddDetector(new SBSGEMPolarimeterTracker("gemFPP","Focal Plane Polarimeter"));
  gHaApps->Add(harm);

  THaAnalyzer* analyzer = new THaAnalyzer;
  analyzer->AddInterStage(new SBSGEPRegionOfInterestModule("FTROI","ROI calc",THaAnalyzer::kCoarseRecon));

  gHaPhysics->Add(new THaGoldenTrack("SBS.gold","golden track","sbs"));
  gHaPhysics->Add(new SBSGEPHeepCoinModule("heep","H(e,e'p)","earm","sbs"));

  THaInterface::SetDecoder(SBSSimDecoder::Class());

  // --- Input handling: build chain ---
  TChain* chain = new TChain("T");
  for (auto& fbase : filebases) {
    TString run_file = Form("%s.root", fbase.c_str());
    if (std::getenv("DATA_DIR")) {
      TString tmp = Form("%s/%s.root", std::string(std::getenv("DATA_DIR")).c_str(), fbase.c_str());
      if (!gSystem->AccessPathName(tmp)) run_file = tmp; // prefer DATA_DIR if it exists
    }
    if (gSystem->AccessPathName(run_file)) {
      Error("replay.C","Input file does not exist: %s", run_file.Data());
      exit(-1);
    }
    chain->Add(run_file);
  }

  // Use the chain inside SBSSimFile
  THaRunBase* run = new SBSSimFile(chain, experiment.Data(), "");
  run->SetFirstEvent(0);
  run->SetLastEvent(nev);
  run->SetDataRequired(0);
  run->SetDate(get_datime(gepconfig));
  run->SetDatabaseName("db_gep3_conf_ECALONLY.dat");

  TString out_dir = gSystem->Getenv("OUT_DIR");
  if(out_dir.IsNull()) out_dir = ".";
  TString out_file = out_dir + "/replayed_output.root";
  analyzer->SetOutFile(out_file);

  analyzer->SetSummaryFile("sbs_hcal_test.log");
  analyzer->SetCrateMapFileName("sbssim_cratemap");

  TString prefix = gSystem->Getenv("SBS_REPLAY");
  prefix += "/replay/";
  analyzer->SetOdefFile(prefix+"replay_gep_mc.odef");
  analyzer->SetCutFile(prefix+"replay_gep_mc.cdef");

  analyzer->SetVerbosity(2);
  analyzer->EnableBenchmarks();

  analyzer->Process(run);

  analyzer->Close();
  delete analyzer;
  gHaVars->Clear();
  gHaPhysics->Delete();
  gHaApps->Delete();
}

int main(int argc, char* argv[]) {
  new THaInterface("Hall A analyzer",&argc,argv,0,0,1);

  if(argc < 3) {
    std::cout << "Usage: replay_gep_mc_ECALONLY file1 [file2 ...] nev\n";
    return -1;
  }

  uint nev = atoi(argv[argc-1]); // last arg = nev
  std::vector<std::string> files;
  for(int i=1;i<argc-1;i++) files.emplace_back(argv[i]);

  replay_gep_mc_ECALONLY(files,1,nev);
  return 0;
}
