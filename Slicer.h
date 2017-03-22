// This file is part of llvm2KITTeL
//
// Copyright 2010-2014 Stephan Falke
//
// Licensed under the University of Illinois/NCSA Open Source License.
// See LICENSE for details.

#ifndef SLICER_H
#define SLICER_H

#include "Ref.h"

// C++ includes
#include <list>
#include <map>
#include <set>

class Rule;

class Slicer
{

public:

    static std::list<ref<Rule> > sliceUsage(std::list<ref<Rule> > rules);

    static std::list<ref<Rule> > sliceConstraint(std::list<ref<Rule> > rules);

    static std::list<ref<Rule> > sliceDefined(std::list<ref<Rule> > rules,std::string startFunctionSymbolName);

    static std::list<ref<Rule> > sliceStillUsed(std::list<ref<Rule> > rules, std::string startFunctionSymbolName,bool conservative=false);

    static std::list<ref<Rule> > sliceTrivialNondefConstraints(std::list<ref<Rule> > rules);

    static std::list<ref<Rule> > sliceDuplicates(std::list<ref<Rule> > rules);


//private:
public:

    static std::set<unsigned int> getSet(unsigned int size);

    static std::string getEval(std::string startstop);

    static unsigned int getIdxFunction(std::string f,std::map<std::string, unsigned int> m_functionIdx);
    static std::string getFunction(unsigned int idx,std::map<unsigned int, std::string>& m_idxFunction);

    static void setUpPreceeds(std::list<ref<Rule> > rules);
    static void makePreceedsTransitive(unsigned int m_numFunctions,bool* m_preceeds);
    static void setUpCalls(std::list<ref<Rule> > rules);
    static void makeCallsTransitive(unsigned int m_numFunctions,bool* m_calls);
    static bool* calc_m_calls(std::list<ref<Rule> > rules, unsigned int m_numFunctions,std::map<std::string, unsigned int>  m_functionIdx);

    static void makeDependsTransitive(unsigned int m_numVars, bool *m_depends);

    static std::set<std::string> getKnownVars(std::string f,std::map<std::string, std::set<std::string> >,unsigned int m_numFunctions,bool* m_proceeds,std::map<unsigned int, std::string>& m_idxFunction,std::map<std::string, unsigned int>  m_functionIdx);

    static std::set<std::string> getStillUsed(std::string f,
                                              std::map<std::string, std::set<std::string> > m_stillUsed,
                                              unsigned int m_numFunctions,
                                              bool* m_calls,
                                              std::map<unsigned int, std::string>& m_idxFunction,
                                              std::map<std::string, unsigned int> m_functionIdx);
    static std::set<unsigned int> getNotNeeded(std::string f, std::list<std::string> vars,std::map<std::string, std::set<std::string> >,unsigned int m_numFunctions,bool* m_proceeds,std::map<unsigned int, std::string>& m_idxFunction,std::map<std::string, unsigned int>  m_functionIdx);

    static std::set<std::string> computeReachableFuns(std::list<ref<Rule> > rules,std::string startFunctionSymbolName);

    static bool isRecursiveCall(std::string f);
    static bool isNondef(std::string v);

};

#endif // SLICER_H
