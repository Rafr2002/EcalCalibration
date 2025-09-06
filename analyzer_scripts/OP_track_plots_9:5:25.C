#include <TFile.h>
#include <TTree.h>

#include <TH1I.h>
#include <TH2I.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TProfile.h>

#include <TCanvas.h>
#include <vector>
#include <map>
#include <set>
#include <utility>
#include <iostream>
#include <TVector3.h>
#include <TLorentzVector.h>
#include <TLegend.h>



// void OP_track_plots(const char *fname="10K_pi0_Ecal_cal.root") {
void OP_track_plots(const char *fname="/Users/rr/jlabHallASoftware/EcalCalibration/analyzer_scripts/10K_pi0_Ecal_cal.root") {

  // Open file and tree
  TFile *f = TFile::Open(fname);
  if (!f) {
    std::cerr << "Could not open file " << fname << std::endl;
    return;
  }
  TTree *T = (TTree*) f->Get("T");
  if (!T) {
    std::cerr << "Tree T not found!" << std::endl;
    return;
  }


//geometry from pi0_Ecal_Cal.mac
double z_calo   = 6.0;   // m (from your g4sbs config)
double z_target = -0.1003; // m (from targpos in macro)
TVector3 vertex(0,0,z_target);



//**Define containers
int ntracks;                       // number of sensitive detector tracks


std::vector<int>    *TID   = 0;    // track ID
std::vector<int>    *MID   = 0;    // mother (parent) track ID
std::vector<int>    *PID   = 0;    // particle ID (PDG code, e.g. 22 = γ, 111 = π0)
std::vector<int>    *MPID  = 0;    // parent particle ID (PDG code)

std::vector<double> *posx=0,*posy=0,*posz=0; //positions
std::vector<double> *momx=0,*momy=0,*momz=0; //momentums
std::vector<double> *polx=0,*poly=0,*polz=0; //polarizations
std::vector<double> *Etot=0,*Tkin=0; //total and kinetic energies GeV

//**Set container branches
T->SetBranchAddress("OTrack.ntracks", &ntracks);
T->SetBranchAddress("OTrack.TID", &TID);
T->SetBranchAddress("OTrack.MID", &MID);
T->SetBranchAddress("OTrack.PID", &PID);
T->SetBranchAddress("OTrack.MPID", &MPID);

T->SetBranchAddress("OTrack.posx", &posx);
T->SetBranchAddress("OTrack.posy", &posy);
T->SetBranchAddress("OTrack.posz", &posz);

T->SetBranchAddress("OTrack.momx", &momx);
T->SetBranchAddress("OTrack.momy", &momy);
T->SetBranchAddress("OTrack.momz", &momz);

T->SetBranchAddress("OTrack.polx", &polx);
T->SetBranchAddress("OTrack.poly", &poly);
T->SetBranchAddress("OTrack.polz", &polz);

T->SetBranchAddress("OTrack.Etot", &Etot);
T->SetBranchAddress("OTrack.T", &Tkin);

//**Initialize Histos
//PDG Particle Ids
TH1I *hPID  = new TH1I("hPID","Particle ID(photon=22) ;ID",20,10,30);  
TH1I *hMPID = new TH1I("hMPID","Parent PID(#pi-0=111) ;ID",140,-20,120);  
TH2I *hPIDvsMPID = new TH2I("hPIDvsMPID","Track PID vs Parent PID;Parent PID;Track PID",
170,-50,250, 170,-50,120);

//Photon Kinematics                             
TH1D *hPhE = new TH1D("hPhotonE","Photon Energy [GeV];Counts",50,0,7);

TH1D *hXPi     = new TH1D("hXPi","X Spread of Pi0 Photons;x [m];Counts",50,-2,2);
TH1D *hXNotPi  = new TH1D("hXNotPi","X Spread of Non-Pi0 Photons;x [m];Counts",50,-2,2);
TH1D *hYPi     = new TH1D("hYPi","Y Spread of Pi0 Photons;y [m];Counts",50,-2,2);
TH1D *hYNotPi  = new TH1D("hYNotPi","Y Spread of Non-Pi0 Photons;y [m];Counts",50,-2,2);

TH2D *hXY = new TH2D("hXY","Photon hit positions;x [m];y [m]",100,-4,4,100,-2,2);

// TH1D *hDR = new TH1D("hDR","Distance between photons dR [m];=Counts",100,0,8.0);

TH1D *hTheta = new TH1D("hTheta","Photon polar angle #theta [deg];#theta [deg];Counts",30,15,45);
TH1D *hPhi   = new TH1D("hPhi","Photon azimuthal angle #phi [deg];#phi [deg];Counts",90,-45,45);


TH1D *hPi0Mass = new TH1D("hPi0Mass","#pi^{0} invariant mass;M_{#gamma#gamma} [GeV];Counts",10,0,0.15);
// TH1D *hStrictPi0Mass = new TH1D("hPi0Mass","#pi^{0} invariant mass strictly of pairs that were produced by Pi0 ;M_{#gamma#gamma} [GeV];Counts",10,0,0.15);
TH1D *hOpeningAng = new TH1D("hOpenAngle","Opening angle between photons;#theta_{12} [#deg];Counts",20,0,20);
TH1D *hE1          = new TH1D("hE1","Photon 1 Energy;E1 [GeV];Counts",100,0,7);
TH1D *hE2          = new TH1D("hE2","Photon 2 Energy;E2 [GeV];Counts",100,0,7);
TH2D *hEvsE        = new TH2D("hEvsE","Photon 2 vs Photon 1 Energy;E1 [GeV];E2 [GeV]",100,0,7,100,0,7);


TH1D *hDE = new TH1D("hDE","Energy Difference between photons; |E1-E2| [GeV];Counts",100,0,7);

// TH2D *hAngVsDE = new TH2D("hAngVsDE","Opening angle vs Energy Difference between photons;#theta_{12} vs E_{12} [#deg];Counts", 15,0,15, 20,0,2);
TH2D *hAngVsDE = new TH2D("hAngVsDE","Opening angle vs Energy Difference between photons;#theta_{12} [#circ];|E_1-E_2| [GeV]", 
  // 40,0,40, //angles
  // 20,0,8.0);
  100,0,35, //angles
  70,0,7.0 //dEnergies
);

// TH2D *hDrVsDE = new TH2D("hDrVsDE",
//   "Distance between photon pair deposits vs Energy Difference between photons; r_{12} [m];|E_1-E_2| [GeV]",
//   50, -.2, .2, 20, 0, 8.0);

// Photon pair separation vs energy difference
TH2D *hDrVsDE = new TH2D("hDrVsDE", "Distance between photon pair deposits vs Energy Difference between photons; r_{12} [m];|E_1-E_2| [GeV]",
  100, 0.0, 3.0,    // r12: 0 to 3 m
  70, 0.0, 7.0);   // ΔE: 0 to 8 GeV

// And the 1D distribution
TH1D *hDR = new TH1D("hDR",
  "Distance between photons dR [m];dR [m];Counts",
  100, 0.0, 3.0);

//invariant mass correlations
TH2D *hAngVsPi0mass = new TH2D("hAngVsPi0mass","Opening angle vs #pi0 mass between photons;#theta_{12} [#circ];M_{#gamma#gamma} [GeV]", 
  15,0,15, 
  10,0.13,0.14);
  // 15,0,15, 
  // 20,0,0.3);

TH2D *hDrVsPi0mass = new TH2D("hDrVsPi0mass", "Distance between photon pair deposits vs #pi0 mass between photons; r_{12} [m];M_{#gamma#gamma} [GeV]", 
30, 0.0, 3.0,    // r12: 0 to 3 m
// 100, 0.0, 3.0,    // r12: 0 to 3 m
10, 0.13,0.14);   // ΔE: 0 to 8 GeV
// 100, 0.0, 0.3);   // ΔE: 0 to 8 GeV

TProfile *pAngVsPi0mass = new TProfile("pAngVsPi0mass",
  "Mean #pi^{0} mass vs opening angle;#theta_{12} [deg];M_{#Pi} [GeV]",
  100, 0, 20, 0.12, 0.14);

TProfile *pDrVsPi0mass = new TProfile("pDrVsPi0mass",
  "Mean #pi^{0} mass vs photon separation;r_{12} [m];M_{#Pi} [GeV]",
  100, 0, 3.0, 0.12, 0.14);



 std::map<int, std::vector<std::pair<TVector3, TLorentzVector>>> eventPhotonMap; //Mother(Pi0) index, <xyz, 4momentum of a photon>

//*Get the position and 4mom of all photons from a pi0 with id
Long64_t nentries = T->GetEntries();
for(Long64_t ie=0; ie<nentries; ie++){
  T->GetEntry(ie);
  // std::set<TVector3, TLorentzVector> photonKins; //xyz, 4momentum of a photon
  // std::map<int, std::vector<TLorentzVector>> eventPhotonMap;
  // std::map<int, std::vector<std::pair<TVector3, TLorentzVector>>> eventPhotonMap; //Mother(Pi0) index, <xyz, 4momentum of a photon>
  eventPhotonMap.clear();

  for(int j=0; j<ntracks; j++){

    if (PID->at(j) == 22 && MPID->at(j) == 111) {

      TVector3 pos(posx->at(j), posy->at(j), posz->at(j));
      TVector3 mom(momx->at(j), momy->at(j), momz->at(j));
      TLorentzVector ph(mom, Etot->at(j));  // px, py, pz, 

      int parentID = MID->at(j); //pi0 the photon originated from
      // eventPhotonMap[parentID].push_back(ph);

       eventPhotonMap[parentID].push_back(std::make_pair(pos, ph));
    }
  }


  //* Take first 2 photons from a pi0 and fill histograms with computed values
  for (auto &Mother : eventPhotonMap){
    auto &photons = Mother.second;

    //skip if no photon signal pair
    if (photons.size() < 2) continue;

    // Photon 1
    TVector3 mom1 = photons[0].second.Vect();
    double E1 = photons[0].second.E();
    double dz1 = z_calo - z_target;
    double scale1 = dz1 / mom1.Z();
    double x1 = 0 + mom1.X() * scale1;  // start from target (0,0,z_target)
    double y1 = 0 + mom1.Y() * scale1;
    TVector3 pos1(x1, y1, z_calo);
    TLorentzVector ph1(mom1.Unit() * E1, E1);

    // Photon 2
    TVector3 mom2 = photons[1].second.Vect();
    double E2 = photons[1].second.E();
    double dz2 = z_calo - z_target;
    double scale2 = dz2 / mom2.Z();
    double x2 = 0 + mom2.X() * scale2;
    double y2 = 0 + mom2.Y() * scale2;
    TVector3 pos2(x2, y2, z_calo);
    TLorentzVector ph2(mom2.Unit() * E2, E2);

    // Invariant mass
  double mgg = (ph1 + ph2).M();
  hPi0Mass->Fill(mgg);

  // Opening angle
  double OpeningAng = mom1.Angle(mom2) * 180.0 / TMath::Pi();
  hOpeningAng->Fill(OpeningAng);

  // Energy difference
  double dE = fabs(E1 - E2);
  hDE->Fill(dE);

  // Distance at ECAL
  double dR = (pos1 - pos2).Mag();
  hDR->Fill(dR);

  // Correlations
  hAngVsDE->Fill(OpeningAng, dE);
  hDrVsDE->Fill(dR, dE);
  hAngVsPi0mass->Fill(OpeningAng, mgg);
  hDrVsPi0mass->Fill(dR, mgg);
  pAngVsPi0mass->Fill(OpeningAng, mgg);
  pDrVsPi0mass->Fill(dR, mgg);

  }
}


  // Draw canvases
  // TCanvas *c1=new TCanvas("c1","Particle IDs(negative are antiparticles)",1000,600);
  // // c1->Divide(2,2);
  // c1->Divide(2,1);
  // c1->cd(1); hPID->Draw();
  // c1->cd(2)->GetLogy(); hMPID->Draw();
  // // c1->cd(3); hPIDvsMPID->Draw("COLZ");

  TCanvas *c2=new TCanvas("c2","Photon Kinematics",1000,600);
  c2->Divide(2,2);
  c2->cd(1); hPhE->Draw();
  c2->cd(2)->SetLogy(); hTheta->Draw();
  c2->cd(3); hPhi->Draw();
  c2->cd(4); hXY->Draw("COLZ");

  TCanvas *c3=new TCanvas("c3","Photon Positions and Distances",1000,600);
  // c3->Divide(2,1);
  // c3->cd(1); 
  hDR->Draw();
  // c3->cd(2); hXY->Draw("COLZ");

  // TCanvas *c4=new TCanvas("c4","Invariant Mass and Energies",1000,600);
  // c4->Divide(2,2);
  // c4->cd(1); 
  // hE1->SetLineColor(kBlue); hE1->Draw();
  // hE2->SetLineColor(kRed); hE2->Draw("SAME");
  // auto leg=new TLegend(0.6,0.7,0.9,0.9);
  // leg->AddEntry(hE1,"Photon 1","l");
  // leg->AddEntry(hE2,"Photon 2","l");
  // leg->Draw();
  // c4->cd(2); hEvsE->Draw("COLZ");
  // c4->cd(3); hPi0Mass->Draw();
  // c4->cd(4)->SetLogy(); hOpeningAng->Draw();

  TCanvas *c5 = new TCanvas("c5", "Metrics",1000,600);
  c5->Divide(2,2);
  c5->cd(1); hAngVsDE->Draw("COLZ");
  c5->cd(2); hDrVsDE->Draw("COLZ");
  c5->cd(3); hAngVsPi0mass->Draw("COLZ");
  c5->cd(4); hDrVsPi0mass->Draw("COLZ");
//   c5->cd(3); pAngVsPi0mass->Draw();
//   c5->cd(4); pDrVsPi0mass->Draw();
}
