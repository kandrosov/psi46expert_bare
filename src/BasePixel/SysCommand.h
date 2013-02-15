/*!
 * \file SysCommand.h
 * \brief Definition of SysCommand class.
 *
 * \b Changelog
 * 15-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using boost::units::quantity to represent physical values.
 *      - Switching to use GNU readline library instead getline.c
 */

#pragma once

#include <iostream>
#include <fstream>
#include "GlobalConstants.h"
/*!
 * \brief Class provides functionalities to run commands from a file or the command line.
 *
 * SysCommand class representing system commands
 * a system consists of multiple modules, with tbms and rocs
 *
 * A single command is contained in the following public fields
 * int type;                 // what kind of target kTB,kTBM,kROC
 * int module;               // module id
 * int roc;                  // roc id (when type=kROC)
 * int narg;                 // number of arguments
 * char* carg[nArgMax];      // keyword or NULL (`\0` terminated)
 * int*  iarg[nArgMax];      // integer list    (-1 terminated)
 */
class SysCommand {
public:
    static const psi::ElectricPotential VOLTAGE_FACTOR;

  enum {kTB, kTBM, kROC, kSYS};
  static const int nModuleMax=32;
  static const int nRocMax=16;
  static const int nCNMax=3;
  static const int nArgMax=10;


  // visible part of a single instruction
  int type;                 // what kind of target kTB,kTBM,kROC
  int CN;                   // control network
  int module;               // module id
  int roc;                  // roc id (when type=kROC)
  int narg;                 // number of arguments
  char* carg[nArgMax]    ;  // keyword or NULL
  int* iarg[nArgMax];       // integer list
  int verbose;

  // methods
  SysCommand();
  int Parse(const char* line);
  int Next();
  int Read(const char* fileName);
  void Print();
  char* TargetPrompt(int mode, const char* sep);
  int GetTargetRoc(int *pModule, int *pRoc);
  char* toString();
  void RocsDone();
  bool IncludesRoc(int rocID);
  
  bool TargetIsTB();
  bool TargetIsTBM();
  bool TargetIsROC();
  bool Exit();
  bool Keyword(const char* keyword);
  bool Keyword(const char* keyword, int** value);
  bool Keyword(const char* keyword, int** value1, int** value2);
  bool Keyword(const char* keyword, const char* keyword1);

 private:

  static const int ibufsize=1000;
  int ibuf[ibufsize]; // holds all integers 
  static const int cbufsize=200; 
  char cbuf[cbufsize]; //holds keywords + terminating '\0's

  struct target_t{
    int type;       // kTB,kTBM,kROC
    int nCN;        // number of control networks
    int CN[nCNMax]; // control network id (0,1,2,...)
    int nModule;    // number of target modules
    int hub[nModuleMax];  // list of hub ID's
    int nRoc;       // length of target list
    int id[16];     // id's of targets (used for ROCs only)
  };
  target_t target, defaultTarget;

  int isIterated[nArgMax];
  int temp[nArgMax];
  int iHub;
  int iRoc;

  // file stack
  int nOpen;
  static const int nFileMax=3;
  std::ifstream* fileStack[nFileMax];
  std::ifstream* inputFile;
  int Getline(char* line, int n);

  bool exitFlag;
  const char* GetWord(const char *s, int* l);
  int StrToI(const char* word, const int len, int* v);
  int PrintList(char* buf, int n, int v[]);
  int PrintTarget(char* buf, target_t* t, int mode);
};
