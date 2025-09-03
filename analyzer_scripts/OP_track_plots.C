#include <TFile.h>
#include <TTree.h>

#include <TH1I.h>
#include <TH2I.h>
#include <TH1F.h>
#include <TH2F.h>

#include <TCanvas.h>
#include <vector>
#include <map>
#include <iostream>
#include <TVector3.h>
#include <TLorentzVector.h>
#include <TLegend.h>



void OP_track_plots(const char *fname="10K_pi0_Ecal_cal.root") {

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
std::vector<double> *vx=0,*vy=0,*vz=0; //production vertices
std::vector<double> *vnx=0,*vny=0,*vnz=0; //trajectories(unit velocities)
std::vector<double> *vTkin=0; //vertex kinetic energy GeV


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

T->SetBranchAddress("OTrack.vx", &vx);
T->SetBranchAddress("OTrack.vy", &vy);
T->SetBranchAddress("OTrack.vz", &vz);

T->SetBranchAddress("OTrack.vnx", &vnx);
T->SetBranchAddress("OTrack.vny", &vny);
T->SetBranchAddress("OTrack.vnz", &vnz);

T->SetBranchAddress("OTrack.vEkin", &vTkin);

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

TH1D *hDist = new TH1D("hDist","Distance between photons #dX [m];#theta [deg];Counts",30,15,45);

TH1D *hTheta = new TH1D("hTheta","Photon polar angle #theta [deg];#theta [deg];Counts",30,15,45);
TH1D *hPhi   = new TH1D("hPhi","Photon azimuthal angle #phi [deg];#phi [deg];Counts",90,-45,45);


TH1D *hPi0Mass = new TH1D("hPi0Mass","#pi^{0} invariant mass;M_{#gamma#gamma} [GeV];Counts",10,0,0.15);
// TH1D *hStrictPi0Mass = new TH1D("hPi0Mass","#pi^{0} invariant mass strictly of pairs that were produced by Pi0 ;M_{#gamma#gamma} [GeV];Counts",10,0,0.15);
TH1D *hOpeningAng = new TH1D("hOpenAngle","Opening angle between photons;#theta_{12} [#deg];Counts",20,0,20);
TH1D *hE1          = new TH1D("hE1","Photon 1 Energy;E1 [GeV];Counts",100,0,7);
TH1D *hE2          = new TH1D("hE2","Photon 2 Energy;E2 [GeV];Counts",100,0,7);
TH2D *hEvsE        = new TH2D("hEvsE","Photon 2 vs Photon 1 Energy;E1 [GeV];E2 [GeV]",100,0,7,100,0,7);


TH1D *hDE = new TH1D("hDE","Energy Difference between photons; |E1-E2| [GeV];Counts",100,0,7);

TH2D *hAngVsDE = new TH2D("hAngVsDE","Opening angle vs Energy Difference between photons;#theta_{12} vs E_{12} [#deg];Counts", 20,-45,45, 20,0,7);

TH2D *hDistVsDE = new TH2D("hAngVsDE","Distance between photon pair deposits vs Energy Difference between photons;#dX_{12} vs E_{12};Counts", 100,-8,8, 20,0,7);



Long64_t nentries = T->GetEntries();
for(Long64_t ie=0; ie<nentries; ie++){
  T->GetEntry(ie);
  

  std::map<int, std::vector<TLorentzVector>> eventPhotonMap;

  for(int j=0; j<ntracks; j++){

    if (PID->at(j) == 22 && MPID->at(j) == 111) {

      TVector3 mom(momx->at(j), momy->at(j), momz->at(j));
      TLorentzVector ph(mom, Etot->at(j));  // px, py, pz, 

      int parentID = MID->at(j); //pi0 the photon originated from
      eventPhotonMap[parentID].push_back(ph);
    }
    
  for (auto &Mother : eventPhotonMap){
    auto &photons = Mother.second;

    TLorentzVector ph1 = photons[0];
    TLorentzVector ph2 = photons[1];

    double mgg = (ph1+ph2).M(); //calculated pi0 invariant mass
    hPi0Mass->Fill(mgg);
    
    double OpeningAng = ph1.Vect().Angle(ph2.Vect())*180.0/TMath::Pi();
    hOpeningAng->Fill(OpeningAng);

    double dE = fabs(ph1.E()-ph2.E());
    hDE->Fill(dE);
    


  }

  }

  // Photon from pi0
        hPhE->Fill(Etot->at(j));
        hTheta->Fill(mom.Theta() * 180.0 / TMath::Pi());
        hPhi->Fill(mom.Phi() * 180.0 / TMath::Pi());

        hXPi->Fill(posx->at(j));
        hYPi->Fill(posy->at(j));

        hXY->Fill(posx->at(j), posy->at(j));

        eventPhotons.push_back(ph);

    // Reconstruct invariant mass if >=2 pi0 photons
    if(eventPhotons.size()>=2){

    // take top-2 by energy
    // std::sort(eventPhotons.begin(),eventPhotons.end(),
    // [](const TLorentzVector&a,const TLorentzVector&b){return a.E()>b.E();});
    
    TLorentzVector ph1=eventPhotons[0];
    TLorentzVector ph2=eventPhotons[1];

    hE1->Fill(ph1.E());
    hE2->Fill(ph2.E());
    hEvsE->Fill(ph1.E(),ph2.E());

    // if (MPID[i]==111){
    //   hStrictPi0Mass->Fill(ph1.E(),ph2.E());
    // }

    double mgg=(ph1+ph2).M();
    hPi0Mass->Fill(mgg);

    double opening=ph1.Vect().Angle(ph2.Vect())*180.0/TMath::Pi();
    hOpeningAng->Fill(opening);
  }
}


  // Draw canvases
  TCanvas *c1=new TCanvas("c1","Particle IDs(negative are antiparticles)",1000,600);
  // c1->Divide(2,2);
  c1->Divide(2,1);
  c1->cd(1); hPID->Draw();
  c1->cd(2)->GetLogy(); hMPID->Draw();
  // c1->cd(3); hPIDvsMPID->Draw("COLZ");

  TCanvas *c2=new TCanvas("c2","Photon Kinematics",1000,600);
  c2->Divide(2,2);
  c2->cd(1); hPhE->Draw();
  c2->cd(2)->SetLogy(); hTheta->Draw();
  c2->cd(3); hPhi->Draw();
  c2->cd(4); hXY->Draw("COLZ");

  TCanvas *c3=new TCanvas("c3","Invariant Mass and Energies",1000,600);
  c3->Divide(2,2);
  c3->cd(1); 
  hE1->SetLineColor(kBlue); hE1->Draw();
  hE2->SetLineColor(kRed); hE2->Draw("SAME");
  auto leg=new TLegend(0.6,0.7,0.9,0.9);
  leg->AddEntry(hE1,"Photon 1","l");
  leg->AddEntry(hE2,"Photon 2","l");
  leg->Draw();
  c3->cd(2); hEvsE->Draw("COLZ");
  c3->cd(3); hPi0Mass->Draw();
  c3->cd(4)->SetLogy(); hOpeningAng->Draw();
}
