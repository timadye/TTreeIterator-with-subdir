#include <gtest/gtest.h>

#include <string>

#include "TError.h"
#include "TFile.h"
#include "TTree.h"

#include "TTreeIterator/TTreeIterator.h"

// ==========================================================================================
// Test configuration
// ==========================================================================================

#define DO_TEST1
#define DO_TEST2
#define DO_TEST3
#define DO_ITER
#define DO_ADDR
#define DO_FILL
#define DO_GET
//#define FULL_CHECKS

const Long64_t nfill1 = 500000;
const Long64_t nfill2 = 100000;
const Long64_t nfill3 = 100000;
constexpr size_t nx1 = 20;
constexpr size_t nx2 = 100;
constexpr size_t nx3 = 100;
const double vinit = 42.3;  // fill each element with a different value starting from here
const int verbose = 0;

// ==========================================================================================
// Global definitions
// ==========================================================================================

// Disable tests not selected above
#ifndef DO_TEST1
#define timingTests1 DISABLED_timingTests1
#endif
#ifndef DO_TEST2
#define timingTests2 DISABLED_timingTests2
#endif
#ifndef DO_TEST3
#define timingTests3 DISABLED_timingTests3
#endif
#ifndef DO_ITER
#define FillIter DISABLED_FillIter
#define GetIter  DISABLED_GetIter
#endif
#ifndef DO_ADDR
#define FillAddr DISABLED_FillAddr
#define GetAddr  DISABLED_GetAddr
#endif
#ifndef DO_FILL
#define FillIter DISABLED_FillIter
#define FillAddr DISABLED_FillAddr
#endif
#ifndef DO_GET
#define GetIter  DISABLED_GetIter
#define GetAddr  DISABLED_GetAddr
#endif

Int_t ShowBranches (const TFile& f, TTree* tree, const std::string& branch_type, const char* op = "tree has") {
  Int_t nbranches = tree->GetListOfBranches()->GetEntries();
  const testing::TestInfo* const test_info = testing::UnitTest::GetInstance()->current_test_info();
  ::Info (Form("%s.%s",test_info->test_case_name(),test_info->name()),
          "%s:%s: %s %lld entries, %d branches of type %s",
          f.GetName(), tree->GetName(), op, tree->GetEntries(), nbranches, branch_type.c_str());
  return nbranches;
}

// ==========================================================================================
// TTreeIterator test 1: write/read a bunch of doubles
// ==========================================================================================

const std::string branch_type1 = "double";

TEST(timingTests1, FillIter) {
  TFile f ("test_timing1.root", "recreate");
  ASSERT_FALSE(f.IsZombie()) << "no file";

  std::vector<std::string> bnames;
  bnames.reserve(nx1);
  for (int i=0; i<nx1; i++) bnames.push_back (Form("x%03d",i));

  TTreeIterator titer ("test", verbose);
  double v = vinit;
  for (auto& e : titer.FillEntries(nfill1)) {
    for (auto& b : bnames) e[b.c_str()] = v++;
  }
  Int_t nbranches = ShowBranches (f, titer.GetTree(), branch_type1, "filled");
  EXPECT_FLOAT_EQ (vinit+double(nbranches*nfill1), v);
}

TEST(timingTests1, GetIter) {
  TFile f ("test_timing1.root");
  ASSERT_FALSE(f.IsZombie()) << "no file";

  TTreeIterator titer ("test", &f, verbose);
  ASSERT_TRUE(titer.GetTree()) << "no tree";
  EXPECT_EQ(titer.GetEntries(), nfill1);
  Int_t nbranches = ShowBranches (f, titer.GetTree(), branch_type1);
  EXPECT_EQ(nbranches, nx1);

  std::vector<std::string> bnames;
  bnames.reserve(nx1);
  for (int i=0; i<nx1; i++) bnames.push_back (Form("x%03d",i));

  double v = vinit;
  for (auto& e : titer) {
    for (auto& b : bnames) {
      double x = e[b.c_str()];
#ifdef FULL_CHECKS
      EXPECT_EQ (x, v++) << Form("entry %lld, branch %d",titer.index(),b.c_str());
#endif
    }
  }
}

// ==========================================================================================
// TTree test 1: write/read a bunch of doubles
// ==========================================================================================

TEST(timingTests1, FillAddr) {
  TFile f ("test_timing1.root", "recreate");
  ASSERT_FALSE(f.IsZombie()) << "no file";

  auto tree = new TTree("test","");
  std::vector<double> vals(nx1);
  for (int i=0; i<nx1; i++) tree->Branch (Form("x%03d",i), &vals[i]);

  double v = vinit;
  for (Long64_t i = 0; i < nfill1; i++) {
    for (auto& x : vals) x = v++;
    tree->Fill();
  }
  f.Write();
  tree->ResetBranchAddresses();

  Int_t nbranches = ShowBranches (f, tree, branch_type1, "filled");
  EXPECT_FLOAT_EQ (vinit+double(nbranches*nfill1), v);
}

TEST(timingTests1, GetAddr) {
  TFile f ("test_timing1.root");
  ASSERT_FALSE(f.IsZombie()) << "no file";

  auto tree = f.Get<TTree>("test");
  ASSERT_TRUE(tree) << "no tree";
  EXPECT_EQ(tree->GetEntries(), nfill1);
  Int_t nbranches = ShowBranches (f, tree, branch_type1);
  EXPECT_EQ(nbranches, nx1);

  std::vector<double> vals(nx1);
  for (int i=0; i<nx1; i++) tree->SetBranchAddress (Form("x%03d",i), &vals[i]);

  double v = vinit;
  Long64_t n = tree->GetEntries();
  for (Long64_t i=0; i<n; i++) {
    tree->GetEntry(i);
#ifdef FULL_CHECKS
    for (auto& x : vals) EXPECT_EQ (x, v++) << Form("entry %lld, element %d",i,&x-vals.data());
#endif
  }
  tree->ResetBranchAddresses();
}

// ==========================================================================================
// TTreeIterator test 2: write/read a bunch of doubles in a POD class
// ==========================================================================================

#define _NUM(x) #x
#define NUM(x) _NUM(x)
//constexpr size_t nx2 = NX2;

// A simple user-defined POD class
struct MyStruct {
  double x[nx2];
};
template<> const char* TTreeIterator::GetLeaflist<MyStruct>() { return Form("x[%d]/D",int(nx2)); }
const std::string branch_type2 = TTreeIterator::GetLeaflist<MyStruct>();

TEST(timingTests2, FillIter) {
  ASSERT_EQ (sizeof(MyStruct::x)/sizeof(MyStruct::x[0]), nx2);
  TFile f ("test_timing2.root", "recreate");
  ASSERT_FALSE(f.IsZombie()) << "no file";

  TTreeIterator titer ("test", verbose);
  double v = vinit;
  for (auto& e : titer.FillEntries(nfill2)) {
    MyStruct M;
    for (auto& x : M.x) x = v++;
    e["M"] = M;
  }
  Int_t nbranches = ShowBranches (f, titer.GetTree(), branch_type2, "filled");
  EXPECT_FLOAT_EQ (vinit+double(nbranches*nfill2*nx2), v);
}

TEST(timingTests2, GetIter) {
  ASSERT_EQ (sizeof(MyStruct::x)/sizeof(MyStruct::x[0]), nx2);
  TFile f ("test_timing2.root");
  ASSERT_FALSE(f.IsZombie()) << "no file";

  TTreeIterator titer ("test", &f, verbose);
  ASSERT_TRUE(titer.GetTree()) << "no tree";
  EXPECT_EQ(titer.GetEntries(), nfill2);
  Int_t nbranches = ShowBranches (f, titer.GetTree(), branch_type2);
  EXPECT_EQ(nbranches, 1);

  double v = vinit;
  for (auto& e : titer) {
    const MyStruct& M = e["M"];
#ifdef FULL_CHECKS
    for (auto& x : M.x) EXPECT_EQ (x, v++) << Form("entry %lld, element %d",titer.index(),&x-&M.x[0]);
#endif
  }
}

// ==========================================================================================
// TTree test 2: write/read a bunch of doubles in a POD class
// ==========================================================================================

TEST(timingTests2, FillAddr) {
  ASSERT_EQ (sizeof(MyStruct::x)/sizeof(MyStruct::x[0]), nx2);
  TFile f ("test_timing2.root", "recreate");
  ASSERT_FALSE(f.IsZombie()) << "no file";

  auto tree = new TTree("test","");
  MyStruct M;
  tree->Branch ("M", &M, TTreeIterator::GetLeaflist<MyStruct>());
  double v = vinit;
  for (Long64_t i = 0; i < nfill2; i++) {
    for (auto& x : M.x) x = v++;
    tree->Fill();
  }
  f.Write();
  tree->ResetBranchAddresses();

  Int_t nbranches = ShowBranches (f, tree, branch_type2, "filled");
  EXPECT_FLOAT_EQ (vinit+double(nbranches*nfill2*nx2), v);
}

TEST(timingTests2, GetAddr) {
  ASSERT_EQ (sizeof(MyStruct::x)/sizeof(MyStruct::x[0]), nx2);
  TFile f ("test_timing2.root");
  ASSERT_FALSE(f.IsZombie()) << "no file";

  auto tree = f.Get<TTree>("test");
  ASSERT_TRUE(tree) << "no tree";
  EXPECT_EQ(tree->GetEntries(), nfill2);
  Int_t nbranches = ShowBranches (f, tree, branch_type2);
  EXPECT_EQ(nbranches, 1);

  MyStruct M;
  tree->SetBranchAddress ("M", &M);
  double v = vinit;
  Long64_t n = tree->GetEntries();
  for (Long64_t i=0; i<n; i++) {
    tree->GetEntry(i);
#ifdef FULL_CHECKS
    for (auto& x : M.x) EXPECT_EQ (x, v++) << Form("entry %lld, element %d",i,&x-&M.x[0]);
#endif
  }
  tree->ResetBranchAddresses();
}

// ==========================================================================================
// TTreeIterator test 3: write/read a vector of doubles
// ==========================================================================================

const std::string branch_type3 = Form("std::vector<double>(%d)",int(nx3));

TEST(timingTests3, FillIter) {
  TFile f ("test_timing3.root", "recreate");
  ASSERT_FALSE(f.IsZombie()) << "no file";

  TTreeIterator titer ("test", verbose);
  double v = vinit;
  for (auto& e : titer.FillEntries(nfill3)) {
    std::vector<double> vx;
    vx.reserve(nx3);
    for (size_t i=0; i<nx3; i++) vx.push_back(v++);
    e["vx"] = vx;
  }
  Int_t nbranches = ShowBranches (f, titer.GetTree(), branch_type3, "filled");
  EXPECT_FLOAT_EQ (vinit+double(nbranches*nfill3*nx3), v);
}

TEST(timingTests3, GetIter) {
  TFile f ("test_timing3.root");
  ASSERT_FALSE(f.IsZombie()) << "no file";

  TTreeIterator titer ("test", &f, verbose);
  ASSERT_TRUE(titer.GetTree()) << "no tree";
  EXPECT_EQ(titer.GetEntries(), nfill3);
  Int_t nbranches = ShowBranches (f, titer.GetTree(), branch_type3);
  EXPECT_EQ(nbranches, 1);

  double v = vinit;
  for (auto& e : titer) {
    const std::vector<double>& vx = e["vx"];
    EXPECT_EQ (vx.size(), nx3);
#ifdef FULL_CHECKS
    for (auto& x : vx) EXPECT_EQ (x, v++) << Form("entry %lld, element %d",titer.index(),&x-vx.data());
#endif
  }
}

// ==========================================================================================
// TTree test 3: write/read a vector of doubles
// ==========================================================================================


TEST(timingTests3, FillAddr) {
  TFile f ("test_timing3.root", "recreate");
  ASSERT_FALSE(f.IsZombie()) << "no file";

  auto tree = new TTree("test","");
  std::vector<double>* vx = 0;
  tree->Branch ("vx", &vx);
  double v = vinit;
  for (Long64_t i = 0; i < nfill3; i++) {
    ASSERT_TRUE(vx);
    vx->clear();
    for (size_t i=0; i<nx3; i++) vx->push_back(v++);
    tree->Fill();
  }
  f.Write();
  tree->ResetBranchAddresses();

  Int_t nbranches = ShowBranches (f, tree, branch_type3, "filled");
  EXPECT_FLOAT_EQ (vinit+double(nbranches*nfill3*nx3), v);
}

TEST(timingTests3, GetAddr) {
  TFile f ("test_timing3.root");
  ASSERT_FALSE(f.IsZombie()) << "no file";

  auto tree = f.Get<TTree>("test");
  ASSERT_TRUE(tree) << "no tree";
  EXPECT_EQ(tree->GetEntries(), nfill3);
  Int_t nbranches = ShowBranches (f, tree, branch_type3);
  EXPECT_EQ(nbranches, 1);

  std::vector<double>* vx = 0;
  tree->SetBranchAddress ("vx", &vx);
  double v = vinit;
  Long64_t n = tree->GetEntries();
  for (Long64_t i=0; i<n; i++) {
    tree->GetEntry(i);
    EXPECT_EQ (vx->size(), nx3);
#ifdef FULL_CHECKS
    for (auto& x : *vx) EXPECT_EQ (x, v++) << Form("entry %lld, element %d",i,&x-vx->data());
#endif
  }
  tree->ResetBranchAddresses();
}
