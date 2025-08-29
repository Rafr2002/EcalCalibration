// ROOT macro to analyze pi0 -> gamma gamma events from G4SBS simulation

#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TCanvas.h>
#include <vector>
#include <cmath>
#include <iostream>
#include <TVector3.h>
#include <TLorentzVector.h>
#include <TLegend.h>

void Hit_based_analyze_pi0(const char *fname="pi0_Ecal_cal.root") {
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

  // ECal hit containers
  int nhits = 0;
  std::vector<double> *sumedep=0, *xhit=0, *yhit=0, *zhit=0;

  T->SetBranchAddress("Earm.ECalTF1.hit.nhits", &nhits);
  T->SetBranchAddress("Earm.ECalTF1.hit.sumedep", &sumedep);
  T->SetBranchAddress("Earm.ECalTF1.hit.xhit", &xhit);
  T->SetBranchAddress("Earm.ECalTF1.hit.yhit", &yhit);
  T->SetBranchAddress("Earm.ECalTF1.hit.zhit", &zhit);

  // Histograms
  TH1D *hE1   = new TH1D("hE1","Photon 1 Energy;E1 [GeV];Counts",100,0,6.5);
  TH1D *hE2   = new TH1D("hE2","Photon 2 Energy;E2 [GeV];Counts",100,0,6.5);
  TH1D *h_pi0_mass   = new TH1D("h_pi0_mass","Reconstructed #pi^{0} Invariant Mass;m_{#pi 0} [GeV];Events",100,0,0.3);
  TH2D *hEvsE = new TH2D("hEvsE",
    "Photon 2 Energy vs Photon 1 Energy;E_{1} [GeV];E_{2} [GeV]",
    200, 0, 7, 200, 0, 7);

  TH2D *hXY   = new TH2D("hXY","ECal Hit Positions;x [m];y [m]",200,-2,2,200,-1,1);
  TH2D *hMassVsX = new TH2D("hMassVsX","Invariant Mass vs X;x [m];m_{#pi 0} [GeV]",200,-2,2,100,0,0.3);
  TH2D *hMassVsY = new TH2D("hMassVsY","Invariant Mass vs Y;y [m];m_{#pi 0} [GeV]",200,-1,1,100,0,0.3);

    // Detector and target geometry
  const double z_calo   = 6.0;   // m, calorimeter face
  const double z_target = 0.09;  // m, target z
  TVector3 vertex(0,0,z_target);

  Long64_t nentries = T->GetEntries();
  for(Long64_t i=0;i<nentries;i++){
    T->GetEntry(i);

    if (nhits < 2) continue; // need at least two hits

    // pick top-2 energies
    std::vector<std::pair<double,int>> energies;
    for(int j=0;j<nhits;j++) {
      energies.push_back({sumedep->at(j), j});
    }
    std::sort(energies.begin(), energies.end(),
              [](auto &a, auto &b){ return a.first > b.first; });

    if(energies.size()<2) continue;
    int i1 = energies[0].second;
    int i2 = energies[1].second;

    double E1 = energies[0].first;
    double E2 = energies[1].first;
    hE1->Fill(E1);
    hE2->Fill(E2);

    hEvsE->Fill(E1, E2);
    hEvsE->Fill(E2, E1);

    // Positions at ECal
    TVector3 pos1(xhit->at(i1), yhit->at(i1), zhit->at(i1));
    TVector3 pos2(xhit->at(i2), yhit->at(i2), zhit->at(i2));

    hXY->Fill(pos1.X(),pos1.Y());
    hXY->Fill(pos2.X(),pos2.Y());

    // Photon directions
    TVector3 dir1 = (pos1 - vertex).Unit();
    TVector3 dir2 = (pos2 - vertex).Unit();

    // Build photon 4-vectors
    TLorentzVector ph1(dir1.X()*E1, dir1.Y()*E1, dir1.Z()*E1, E1);
    TLorentzVector ph2(dir2.X()*E2, dir2.Y()*E2, dir2.Z()*E2, E2);

    // Invariant mass
    double pi0_mass = (ph1 + ph2).M();
    h_pi0_mass->Fill(pi0_mass);

    double x1 = pos1.X() / pos1.Z() * z_calo;  // projected to calorimeter plane
    double y1 = pos1.Y() / pos1.Z() * z_calo;
    double x2 = pos2.X() / pos2.Z() * z_calo;
    double y2 = pos2.Y() / pos2.Z() * z_calo;
    
    // Fill invariant mass vs position for each photon
hMassVsX->Fill(x1, pi0_mass);
hMassVsX->Fill(x2, pi0_mass);

hMassVsY->Fill(y1, pi0_mass);
hMassVsY->Fill(y2, pi0_mass);


  // Hit position plot
  hXY->Fill(xhit->at(i1), yhit->at(i1));
  hXY->Fill(xhit->at(i2), yhit->at(i2));
  }

  // Draw plots
  // TCanvas *c1 = new TCanvas("c1","ECal Pi0 Analysis",1200,800);
  // c1->Divide(2,2);
  // c1->cd(1); hE1->Draw();
  // c1->cd(2); hE2->Draw();
  // c1->cd(3); h_pi0_mass->Draw();
  // c1->cd(4); hXY->Draw("COLZ");
  // TCanvas *c1 = new TCanvas("c1","2 cluster energies",1200,800);
  // hE1->SetLineColor(kBlue);
  // hE1->SetLineWidth(2);
  // hE1->Draw();

  // hE2->SetLineColor(kRed);
  // hE2->SetLineWidth(2);
  // hE2->Draw("SAME");

  // auto leg1 = new TLegend(0.6,0.7,0.9,0.9);
  // leg1->AddEntry(hE1,"Photon 1 Energy","l");
  // leg1->AddEntry(hE2,"Photon 2 Energy","l");
  // leg1->Draw();

  TCanvas *c1 = new TCanvas("c1","Photon Energy Correlation",1000,800);
  hEvsE->Draw("COLZ");

  TCanvas *c2 = new TCanvas("c2","Pi0 invariant mass",1200,800);
  h_pi0_mass->Draw();

  TCanvas *c3 = new TCanvas("c3","Mass vs Position",1200,600);
  c3->Divide(2,1);
  c3->cd(1); hMassVsX->Draw("COLZ");
  c3->cd(2); hMassVsY->Draw("COLZ");
}



void analyze_pi0(const char *fname="pi0_Ecal_cal.root") {
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

    // Cluster branches
    const int MAXCLUS = 100;
    Double_t ecal_e[MAXCLUS];
    Double_t ecal_x[MAXCLUS];
    Double_t ecal_y[MAXCLUS];
    Double_t ecal_t[MAXCLUS];
    Double_t clus_nblk[MAXCLUS];
    Int_t nclus;

    T->SetBranchAddress("earm.ecal.nclus", &nclus);
    T->SetBranchAddress("earm.ecal.clus.e", ecal_e);
    T->SetBranchAddress("earm.ecal.clus.x", ecal_x);
    T->SetBranchAddress("earm.ecal.clus.y", ecal_y);
    T->SetBranchAddress("earm.ecal.clus.atimeblk", ecal_t);
    T->SetBranchAddress("earm.ecal.clus.nblk", clus_nblk);



  // Histograms
// Histograms
  TH1D *hE1   = new TH1D("hE1","Photon 1 Energy;E1 [GeV];Counts",100,0,7);
  TH1D *hE2   = new TH1D("hE2","Photon 2 Energy;E2 [GeV];Counts",100,0,7);
  TH1D *h_pi0_mass = new TH1D("h_pi0_mass","Reconstructed #pi^{0} Invariant Mass;m_{#pi-0} [GeV];Events",100,0,0.3);
  TH2D *hEvsE = new TH2D("hEvsE","Photon 2 Energy vs Photon 1 Energy;E1 [GeV];E2 [GeV]",100,0,7,100,0,7);


  // Detector and target geometry
  const double z_calo   = 6.0;   // m, calorimeter face
  const double z_target = 0.09;  // m, target z
  TVector3 vertex(0,0,z_target);

  Long64_t nentries = T->GetEntries();
  for(Long64_t i=0;i<nentries;i++){
    T->GetEntry(i);

    if  (nclus < 2) continue;

    // collect photon candidates
    std::vector<int> good;
    for(int j=0;j<nclus;j++) {
      if (ecal_e[j] < 0.2) continue;          // energy cut
      if (clus_nblk[j] < 2) continue;         // require multi-block
      if (ecal_t[j] < 80 || ecal_t[j] > 120) continue; // timing cut
      good.push_back(j);
    }
    if (good.size() < 2) continue;

    // sort by energy
    std::sort(good.begin(), good.end(),
              [&](int a, int b){ return ecal_e[a] > ecal_e[b]; });

    int i1 = good[0];
    int i2 = good[1];

    double E1 = ecal_e[i1];
    double E2 = ecal_e[i2];
    hE1->Fill(E1);
    hE2->Fill(E2);
    hEvsE->Fill(E1,E2);

    // build photon 4-vectors
    TVector3 pos1(ecal_x[i1], ecal_y[i1], z_calo);
    TVector3 pos2(ecal_x[i2], ecal_y[i2], z_calo);
    TVector3 dir1 = (pos1 - vertex).Unit();
    TVector3 dir2 = (pos2 - vertex).Unit();

    TLorentzVector ph1(dir1.X()*E1, dir1.Y()*E1, dir1.Z()*E1, E1);
    TLorentzVector ph2(dir2.X()*E2, dir2.Y()*E2, dir2.Z()*E2, E2);

    double mgg = (ph1 + ph2).M();
    h_pi0_mass->Fill(mgg);
  }

  // Draw plots
  TCanvas *c1 = new TCanvas("c1","Photon Energies",800,600);
  hE1->SetLineColor(kBlue);
  hE2->SetLineColor(kRed);
  hE1->Draw();
  hE2->Draw("SAME");
  auto leg = new TLegend(0.6,0.7,0.9,0.9);
  leg->AddEntry(hE1,"Photon 1","l");
  leg->AddEntry(hE2,"Photon 2","l");
  leg->Draw();

  TCanvas *c2 = new TCanvas("c2","Photon Correlation",800,600);
  hEvsE->Draw("COLZ");

  TCanvas *c3 = new TCanvas("c3","Pi0 Invariant Mass",800,600);
  h_pi0_mass->Draw();
}