// This file is part of llvm2KITTeL
//
// Copyright 2010-2014 Stephan Falke
//
// Licensed under the University of Illinois/NCSA Open Source License.
// See LICENSE for details.


// just to satisfy:
// /usr/lib/llvm-3.4/include/llvm/Support/DataTypes.h:48:3: error: #error "Must #define __STDC_LIMIT_MACROS before #including Support/DataTypes.h"
// # error "Must #define __STDC_LIMIT_MACROS before #including Support/DataTypes.h"
// /usr/lib/llvm-3.4/include/llvm/Support/DataTypes.h:52:3: error: #error "Must #define __STDC_CONSTANT_MACROS before " "#including Support/DataTypes.h"
// # error "Must #define __STDC_CONSTANT_MACROS before "
#define __STDC_LIMIT_MACROS 0
#define __STDC_CONSTANT_MACROS 0

#include "Slicer.h"
#include "Constraint.h"
#include "Polynomial.h"
#include "Rule.h"
#include "Term.h"

// C++ includes
#include <iostream>
#include <sstream>
#include <queue>

using std::cout;
using std::endl;


// Usage
// 产生集合 {0,1,...,size-1}
std::set<unsigned int> Slicer::getSet(unsigned int size)
{
    std::set<unsigned int> res;
    for (unsigned int i = 0; i < size; ++i) {
        res.insert(i);
    }
    return res;
}

/**
 * [1/6]
 * 还有输入：
 * m_F->arg_begin() // Keep all inputs of integer type
 * llvm::Module *module = m_F->getParent()->global_begin();  // keep all globals of integer type
 *
 * @param rules
 * @return
 */
std::list<ref<Rule> > Slicer::sliceUsage(std::list<ref<Rule> > rules)
{
    if (rules.empty()) {
        return rules;
    }
    std::list<ref<Rule> > res;
    std::vector<std::string> vars;
    std::list<ref<Polynomial> > var_args = (*rules.begin())->getLeft()->getArgs();
    for (std::list<ref<Polynomial> >::iterator i = var_args.begin(), e = var_args.end(); i != e; ++i) {
        ref<Polynomial> tmp = *i;
        std::set<std::string> tmpVars;
        tmp->addVariablesToSet(tmpVars);
        vars.push_back(*(tmpVars.begin()));
    }
    unsigned int arity = static_cast<unsigned int>(vars.size());
    std::set<unsigned int> notNeeded = getSet(arity);
    for (std::list<ref<Rule> >::iterator it = rules.begin(), et = rules.end(); it != et; ++it) {
        ref<Rule> tmp = *it;
        std::set<std::string> c_vars;
        tmp->getConstraint()->addVariablesToSet(c_vars);
        std::vector<std::set<std::string> > rhsVars;
        std::set<std::string> allRhsVars;
        if (isRecursiveCall(tmp->getRight()->getFunctionSymbol())) {
            tmp->getRight()->addVariablesToSet(allRhsVars);
        } else {
            for (unsigned int i = 0; i < arity; ++i) {
                std::set<std::string> tmpVars;
                tmp->getRight()->addVariablesToSet(i, tmpVars);
                rhsVars.push_back(tmpVars);
            }
        }
        unsigned int argc = 0;
        for (std::vector<std::string>::iterator vi = vars.begin(), ve = vars.end(); vi != ve; ++vi, ++argc) {
            std::string var = *vi;
            if (c_vars.find(var) != c_vars.end()) {
                // needed because it occurs in the constraint
                notNeeded.erase(argc);
            } else {
                if (isRecursiveCall(tmp->getRight()->getFunctionSymbol())) {
                    if (allRhsVars.find(var) != allRhsVars.end()) {
                        // needed because it occurs in a recursive call
                        notNeeded.erase(argc);
                    }
                } else {
                    unsigned int rhsc = 0;
                    for (std::vector<std::set<std::string> >::iterator ri = rhsVars.begin(), re = rhsVars.end(); ri != re; ++ri, ++rhsc) {
                        if (rhsc != argc) {
                            std::set<std::string> &r_vars = *ri;
                            if (r_vars.find(var) != r_vars.end()) {
                                // needed because it occurs in different position in rhs
                                notNeeded.erase(argc);
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    // Keep all inputs of integer type
    unsigned int intarg = 0;


    for (std::list<ref<Rule> >::iterator i = rules.begin(), e = rules.end(); i != e; ++i) {
        ref<Rule> rule = *i;
        if (!isRecursiveCall(rule->getRight()->getFunctionSymbol())) {
            res.push_back(rule->dropArgs(notNeeded));
        } else {
            res.push_back(Rule::create(rule->getLeft()->dropArgs(notNeeded), rule->getRight(), rule->getConstraint()));
        }
    }
    return res;
}

// Constraint
/**
 * [2/6]
 * 还有输入：
 * m_F    ->arg_begin()
 *
 *
 * 以下已局部化：
 * m_varIdx 在且仅在 Slicer::sliceConstraint(std::list<ref<Rule> > rules) 和 Slicer::makeDependsTransitive(void) 使用
 * m_idxVar 在且仅在 Slicer::sliceConstraint(std::list<ref<Rule> > rules) 和 Slicer::makeDependsTransitive(void) 使用
 * * 好像体会到了作者的心情： 为了在 slicer.h L85 声明 unsigned int getIdxVar(std::string v);，也把数据写在旁边，于是把这个局部使用的数据写成了类成员。
 * 1. 唉 不能在函数里定义函数，真不优雅；
 * 2. 包个 getIdxVar(std::string v); 干吊？在里面写了蜜汁 exit code，我怀疑作者不会 C++ Exception。
 * 2017-2-16 19:12:29 取消 getIdxVar
 * m_numVars 并不需要。是现场算出来的。
 * m_depends 是现场算的。在且仅在 Slicer::sliceConstraint(std::list<ref<Rule> > rules) 和 Slicer::makeDependsTransitive(void) 使用
 * @param rules
 * @return
 */
std::list<ref<Rule> > Slicer::sliceConstraint(std::list<ref<Rule> > rules)
{
    // 局部化了的 前 类成员
    std::map<std::string, unsigned int> m_varIdx;
    std::map<unsigned int, std::string> m_idxVar;
    unsigned int m_numVars;
    bool *m_depends;

    if (rules.empty()) {
        return rules;
    }
    std::list<ref<Rule> > res;
    std::vector<std::string> vars;
    std::list<ref<Polynomial> > var_args = (*rules.begin())->getLeft()->getArgs();
    for (std::list<ref<Polynomial> >::iterator i = var_args.begin(), e = var_args.end(); i != e; ++i) {
        ref<Polynomial> tmp = *i;
        std::set<std::string> tmpVars;
        tmp->addVariablesToSet(tmpVars);
        vars.push_back(*(tmpVars.begin()));
    }
    m_numVars = static_cast<unsigned int>(vars.size());
    std::set<unsigned int> notNeeded = getSet(m_numVars);
    // Keep all inputs of integer type
    unsigned int intarg = 0;

    // prepare
    std::set<std::string> c_vars;
    m_depends = new bool[m_numVars * m_numVars];
    for (unsigned int i = 0; i < m_numVars; ++i) {
        for (unsigned int j = 0; j < m_numVars; ++j) {
            m_depends[i + m_numVars * j] = false;
        }
    }
    unsigned int idx = 0;
    for (std::vector<std::string>::iterator i = vars.begin(), e = vars.end(); i != e; ++i) {
        std::string v = *i;
        m_varIdx.insert(std::make_pair(v, idx));
        m_idxVar.insert(std::make_pair(idx, v));
        ++idx;
    }
    for (std::list<ref<Rule> >::iterator i = rules.begin(), e = rules.end(); i != e; ++i) {
        ref<Rule> rule = *i;
        rule->getConstraint()->addVariablesToSet(c_vars);
        if (isRecursiveCall(rule->getRight()->getFunctionSymbol())) {
            rule->getRight()->addVariablesToSet(c_vars);
            continue;
        }
        std::list<ref<Polynomial> > rhsArgs = rule->getRight()->getArgs();
        std::list<ref<Polynomial> >::iterator ri = rhsArgs.begin();
        for (std::vector<std::string>::iterator it = vars.begin(), et = vars.end(); it != et; ++it, ++ri) {
            std::string lvar = *it;
            unsigned int lvarIdx = m_varIdx.find(lvar)->second;
            ref<Polynomial> inRhs = *ri;
            std::set<std::string> tmp;
            inRhs->addVariablesToSet(tmp);
            for (std::set<std::string>::iterator ii = tmp.begin(), ee = tmp.end(); ii != ee; ++ii) {
                if (!isNondef(*ii)) {
                    m_depends[lvarIdx + m_numVars * m_varIdx.find(*ii)->second] = true;
                }
            }
        }
    }
    /**
 * m_depends 在且仅在
     * Slicer::makeDependsTransitive(void)
     * Slicer::sliceConstraint
     * 使用
 */
    makeDependsTransitive(m_numVars,m_depends);


    for (std::set<std::string>::iterator i = c_vars.begin(), e = c_vars.end(); i != e; ++i) {
        std::string v = *i;
        if (isNondef(v)) {
            continue;
        }
        unsigned int vidx = m_varIdx.find(v)->second;
        for (unsigned int ii = 0; ii < m_numVars; ++ii) {
            if (m_depends[vidx + m_numVars * ii]) {
                notNeeded.erase(ii);
            }
        }
    }

    for (std::list<ref<Rule> >::iterator i = rules.begin(), e = rules.end(); i != e; ++i) {
        ref<Rule> rule = *i;
        if (!isRecursiveCall(rule->getRight()->getFunctionSymbol())) {
            res.push_back(rule->dropArgs(notNeeded));
        } else {
          res.push_back(Rule::create(rule->getLeft()->dropArgs(notNeeded), rule->getRight(), rule->getConstraint()));
        }
    }
    return res;
}


std::string Slicer::getEval(std::string startstop)
{
    std::ostringstream tmp;
//    tmp << "eval_" << m_F->getName().str() << "_" << startstop;
    // 20160226
    // TODO 千万别忘了改啊 // 多个函数可咋整
    tmp << "eval_" << "main" << "_" << startstop;
    return tmp.str();
}


std::set<std::string> calc_m_functions(std::list<ref<Rule>> rules){
    std::set<std::string> m_functions;
    for (std::list<ref<Rule> >::iterator i = rules.begin(), e = rules.end(); i != e; ++i) {
        m_functions.insert((*i)->getLeft()->getFunctionSymbol());
        m_functions.insert((*i)->getRight()->getFunctionSymbol());
    }
    return m_functions;
}

bool* calc_m_preceeds(unsigned int m_numFunctions){
    bool* m_preceeds = new bool[m_numFunctions * m_numFunctions];
    for (unsigned int i = 0; i < m_numFunctions; ++i) {
        for (unsigned int j = 0; j < m_numFunctions; ++j) {
            m_preceeds[i + m_numFunctions * j] = false;
        }
    }
    return m_preceeds;
}

void init_m_functionIdx_m_idxFunction(std::set<std::string> m_functions,std::map<std::string, unsigned int>& m_functionIdx,std::map<unsigned int, std::string>& m_idxFunction){
    unsigned int idx = 0;
    for (std::set<std::string>::iterator i = m_functions.begin(), e = m_functions.end(); i != e; ++i) {
        std::string f = *i;
        m_functionIdx.insert(std::make_pair(f, idx));
        m_idxFunction.insert(std::make_pair(idx, f));
        ++idx;
    }
}

void Slicer::setUpPreceeds(std::list<ref<Rule> > rules)
{
    std::set<std::string> m_functions=calc_m_functions(rules); // 局部化
    unsigned int m_numFunctions = static_cast<unsigned int>(m_functions.size());; // 局部化
    std::map<std::string, unsigned int> m_functionIdx;
    std::map<unsigned int, std::string> m_idxFunction;
    init_m_functionIdx_m_idxFunction(m_functions,m_functionIdx,m_idxFunction);
    bool *m_preceeds=calc_m_preceeds(m_numFunctions);

    std::set<std::string> visited;
    std::queue<std::string> todo;
    todo.push(getEval("start"));

    do {
        std::string v = todo.front();
        todo.pop();
        visited.insert(v);
        std::list<std::string> succs;
        for (std::list<ref<Rule> >::iterator i = rules.begin(), e = rules.end(); i != e; ++i) {
            ref<Rule> rule = *i;
            if (rule->getLeft()->getFunctionSymbol() == v) {
                bool have = false;
                std::string succ = rule->getRight()->getFunctionSymbol();
                for (std::list<std::string>::iterator si = succs.begin(), se = succs.end(); si != se; ++si) {
                    if (*si == succ) {
                        have = true;
                    }
                }
                if (!have) {
                    succs.push_back(succ);
                }
            }
        }
        for (std::list<std::string>::iterator i = succs.begin(), e = succs.end(); i != e; ++i) {
            std::string child = *i;
            if (visited.find(child) == visited.end()) {
                // not yet visited
                m_preceeds[getIdxFunction(v,m_functionIdx) + m_numFunctions * getIdxFunction(child,m_functionIdx)] = true;
                todo.push(child);
            }
        }
    } while (!todo.empty());

    makePreceedsTransitive(m_numFunctions,m_preceeds);
}

std::set<std::string> Slicer::computeReachableFuns(std::list<ref<Rule> > rules,std::string startFunctionSymbolName)
{
    std::set<std::string> res;
    std::queue<std::string> todo;
//    todo.push(getEval("start"));
    todo.push(startFunctionSymbolName);

    do {
        std::string v = todo.front();
        todo.pop();
        res.insert(v);
        std::list<std::string> succs;
        for (std::list<ref<Rule> >::iterator i = rules.begin(), e = rules.end(); i != e; ++i) {
            ref<Rule> rule = *i;
            if (rule->getLeft()->getFunctionSymbol() == v) {
                bool have = false;
                std::string succ = rule->getRight()->getFunctionSymbol();
                for (std::list<std::string>::iterator si = succs.begin(), se = succs.end(); si != se; ++si) {
                    if (*si == succ) {
                        have = true;
                    }
                }
                if (!have) {
                    succs.push_back(succ);
                }
            }
        }
        for (std::list<std::string>::iterator i = succs.begin(), e = succs.end(); i != e; ++i) {
            std::string child = *i;
            if (res.find(child) == res.end()) {
                // not yet visited
                todo.push(child);
            }
        }
    } while (!todo.empty());

    return res;
}

/**
 *[3/6]
 * 还需要：
 * m_F           ->arg_begin()
 * m_F           ->getParent()->global_begin()
 * m_defined 但是当场算 已局部化
 *
 * setUpPreceeds()
 *
 * @param rules
 * @return
 */
std::list<ref<Rule> > Slicer::sliceDefined(std::list<ref<Rule> > rules,std::string startFunctionSymbolName)
{
    std::map<std::string, std::set<std::string> > m_defined;
    auto m_functions=calc_m_functions(rules);
    unsigned int m_numFunctions= (unsigned int) m_functions.size();
    bool* m_preceeds=calc_m_preceeds(m_numFunctions);
    std::map<std::string, unsigned int> m_functionIdx;
    std::map<unsigned int, std::string> m_idxFunction;
    init_m_functionIdx_m_idxFunction(m_functions,m_functionIdx,m_idxFunction);

    std::set<std::string> reachableFuns = computeReachableFuns(rules,startFunctionSymbolName);
    std::list<ref<Rule> > reachable;
    for (std::list<ref<Rule> >::iterator i = rules.begin(), e = rules.end(); i != e; ++i) {
        if (reachableFuns.find((*i)->getLeft()->getFunctionSymbol()) != reachableFuns.end()) {
            reachable.push_back(*i);
        }
    }
    if (reachable.empty()) {
        return reachable;
    }
    setUpPreceeds(reachable); // 只用一次
    std::set<std::string> initial;

    m_defined.insert(std::make_pair(getEval("start"), initial));
    for (std::list<ref<Rule> >::iterator i = reachable.begin(), e = reachable.end(); i != e; ++i) {
        ref<Rule> tmp = *i;
        ref<Term> left = tmp->getLeft();
        ref<Term> right = tmp->getRight();
        if (m_defined.find(right->getFunctionSymbol()) != m_defined.end() || isRecursiveCall(right->getFunctionSymbol())) {
            // already have this one
            continue;
        }
        std::list<ref<Polynomial> > largs = left->getArgs();
        std::list<ref<Polynomial> > rargs = right->getArgs();
        std::set<std::string> defs;
        for (std::list<ref<Polynomial> >::iterator li = largs.begin(), le = largs.end(), ri = rargs.begin(); li != le; ++li, ++ri) {
            ref<Polynomial> lpol = *li;
            ref<Polynomial> rpol = *ri;
            std::set<std::string> ltmpVars;
            lpol->addVariablesToSet(ltmpVars);
            std::string lvar = *ltmpVars.begin();
            if (!rpol->isVar()) {
                defs.insert(lvar);
            } else {
                std::set<std::string> rtmpVars;
                rpol->addVariablesToSet(rtmpVars);
                std::string rvar = *rtmpVars.begin();
                if (lvar != rvar) {
                    defs.insert(lvar);
                }
            }
        }
        m_defined.insert(std::make_pair(right->getFunctionSymbol(), defs));
    }

    std::list<ref<Rule> > res;
    std::list<std::string> vars;
    std::list<ref<Polynomial> > polys = (*reachable.begin())->getLeft()->getArgs();
    for (std::list<ref<Polynomial> >::iterator i = polys.begin(), e = polys.end(); i != e; ++i) {
        std::set<std::string> tmpVars;
        (*i)->addVariablesToSet(tmpVars);
        vars.push_back(*tmpVars.begin());
    }
    for (std::list<ref<Rule> >::iterator i = reachable.begin(), e = reachable.end(); i != e; ++i) {
        ref<Rule> rule = *i;
        std::set<unsigned int> lnotneeded = getNotNeeded(rule->getLeft()->getFunctionSymbol(), vars,m_defined,m_numFunctions,m_preceeds,m_idxFunction, m_functionIdx);
        std::set<unsigned int> rnotneeded;
        if (rule->getRight()->getFunctionSymbol() == getEval("stop")) {
            rnotneeded = getSet(static_cast<unsigned int>(rule->getRight()->getArgs().size()));
        } else if (isRecursiveCall(rule->getRight()->getFunctionSymbol())) {
            // keep everything
        } else {
            rnotneeded = getNotNeeded(rule->getRight()->getFunctionSymbol(), vars,m_defined,m_numFunctions,m_preceeds,m_idxFunction, m_functionIdx);
        }
        ref<Rule> newRule = Rule::create(rule->getLeft()->dropArgs(lnotneeded), rule->getRight()->dropArgs(rnotneeded), rule->getConstraint());
        res.push_back(newRule);
    }

    return res;
}

// 这个倒是自己写得很pure
std::set<unsigned int> Slicer::getNotNeeded(std::string f, std::list<std::string> vars,std::map<std::string, std::set<std::string> > m_defined,unsigned int m_numFunctions,bool* m_preceeds,std::map<unsigned int, std::string>& m_idxFunction,std::map<std::string, unsigned int>  m_functionIdx)
{
    std::set<unsigned int> res;
    unsigned int tmp = 0;
    std::set<std::string> known = getKnownVars(f,m_defined,m_numFunctions,m_preceeds,m_idxFunction,m_functionIdx);
    for (std::list<std::string>::iterator vi = vars.begin(), ve = vars.end(); vi != ve; ++vi) {
        if (known.find(*vi) == known.end()) {
            res.insert(tmp);
        }
        ++tmp;
    }
    return res;
}

std::set<std::string> Slicer::getKnownVars(std::string f,std::map<std::string, std::set<std::string> > m_defined,unsigned int m_numFunctions,bool* m_preceeds,std::map<unsigned int, std::string>& m_idxFunction,std::map<std::string, unsigned int>  m_functionIdx)
{
    std::set<std::string> res;
    std::set<std::string> fdefines = m_defined.find(f)->second;
    res.insert(fdefines.begin(), fdefines.end());
    unsigned int fidx = getIdxFunction(f,m_functionIdx) * m_numFunctions;
    for (unsigned int i = 0; i < m_numFunctions; ++i) {
        if (m_preceeds[i + fidx]) {
            std::set<std::string> pdefines = m_defined.find(getFunction(i,m_idxFunction))->second;
            res.insert(pdefines.begin(), pdefines.end());
        }
    }
    return res;
}


bool* Slicer::calc_m_calls(std::list<ref<Rule> > rules, unsigned int m_numFunctions,std::map<std::string, unsigned int>  m_functionIdx){
    bool *m_calls= new bool[m_numFunctions * m_numFunctions];

    for (unsigned int i = 0; i < m_numFunctions; ++i) {
        for (unsigned int j = 0; j < m_numFunctions; ++j) {
            m_calls[i + m_numFunctions * j] = false;
        }
    }
    std::set<std::string> visited;
    std::queue<std::string> todo;
    todo.push(getEval("start"));

    do {
        std::string v = todo.front();
        todo.pop();
        visited.insert(v);
        std::list<std::string> succs;
        for (std::list<ref<Rule> >::iterator i = rules.begin(), e = rules.end(); i != e; ++i) {
            ref<Rule> rule = *i;
            if (rule->getLeft()->getFunctionSymbol() == v) {
                bool have = false;
                std::string succ = rule->getRight()->getFunctionSymbol();
                for (std::list<std::string>::iterator si = succs.begin(), se = succs.end(); si != se; ++si) {
                    if (*si == succ) {
                        have = true;
                    }
                }
                if (!have) {
                    succs.push_back(succ);
                }
            }
        }
        for (std::list<std::string>::iterator i = succs.begin(), e = succs.end(); i != e; ++i) {
            std::string child = *i;
            m_calls[getIdxFunction(v, m_functionIdx) + m_numFunctions * getIdxFunction(child, m_functionIdx)] = true;
            if (visited.find(child) == visited.end()) {
                // not yet visited
                todo.push(child);
            }
        }
    } while (!todo.empty());
    return m_calls;
}

// Still Used
void Slicer::setUpCalls(std::list<ref<Rule> > rules)
{
    std::set<std::string> m_functions=calc_m_functions(rules); // 局部化
    unsigned int m_numFunctions = static_cast<unsigned int>(m_functions.size());; // 局部化
    std::map<std::string, unsigned int> m_functionIdx;
    std::map<unsigned int, std::string> m_idxFunction;
    init_m_functionIdx_m_idxFunction(m_functions,m_functionIdx,m_idxFunction);
    bool *m_preceeds;
    bool *m_calls= calc_m_calls(rules,m_numFunctions,m_functionIdx);

    for (std::list<ref<Rule> >::iterator i = rules.begin(), e = rules.end(); i != e; ++i) {
        m_functions.insert((*i)->getLeft()->getFunctionSymbol());
        m_functions.insert((*i)->getRight()->getFunctionSymbol());
    }

    unsigned int idx = 0;
    for (std::set<std::string>::iterator i = m_functions.begin(), e = m_functions.end(); i != e; ++i) {
        std::string f = *i;
        m_functionIdx.insert(std::make_pair(f, idx));
        m_idxFunction.insert(std::make_pair(idx, f));
        ++idx;
    }

    makeCallsTransitive(m_numFunctions,m_calls);
}

/**
 * [4/6]
 * 还需要：
 * m_F->arg_begin()
 *
 * m_stillUsed 但是当场算
 * m_phiVars 但是用不上 因为 conservative总是false
 *
 * 大问题：getStillUsed 里还用到了:
 * m_numFunctions
 * m_calls
 *
 * @param rules
 * @param conservative 按默认配置 总是false
 * @return
 */
std::list<ref<Rule> > Slicer::sliceStillUsed(std::list<ref<Rule> > rules,std::string startFunctionSymbolName, bool conservative)
{
    // 局部化
    std::map<std::string, std::set<std::string> > m_stillUsed;
    std::set<std::string> m_phiVars; // 悍然

    auto m_functions=calc_m_functions(rules);
    unsigned int m_numFunctions= (unsigned int) m_functions.size();
    std::map<std::string, unsigned int> m_functionIdx;
    std::map<unsigned int, std::string> m_idxFunction;
    init_m_functionIdx_m_idxFunction(m_functions,m_functionIdx,m_idxFunction);
    bool *m_calls= calc_m_calls(rules,m_numFunctions,m_functionIdx);

    std::set<std::string> reachableFuns = computeReachableFuns(rules,startFunctionSymbolName);
    std::list<ref<Rule> > reachable;
    for (std::list<ref<Rule> >::iterator i = rules.begin(), e = rules.end(); i != e; ++i) {
        if (reachableFuns.find((*i)->getLeft()->getFunctionSymbol()) != reachableFuns.end()) {
            reachable.push_back(*i);
        }
    }
    if (reachable.empty()) {
        return reachable;
    }
    setUpCalls(reachable);

    // simple cases first
    m_stillUsed.insert(std::make_pair(getEval("stop"), std::set<std::string>()));
    // 20170226
    std::set<std::string> initial;

    m_stillUsed.insert(std::make_pair(getEval("start"), initial));

    // rules
    for (std::list<ref<Rule> >::iterator i = reachable.begin(), e = reachable.end(); i != e; ++i) {
        ref<Rule> tmp = *i;
        ref<Term> left = tmp->getLeft();
        ref<Term> right = tmp->getRight();
        std::set<std::string> c_vars;
        tmp->getConstraint()->addVariablesToSet(c_vars);
        std::list<ref<Polynomial> > largs = left->getArgs();
        size_t largsSize = largs.size();
        std::list<ref<Polynomial> > rargs = right->getArgs();
        std::set<std::string> used;
        std::set<std::string> interestingVars;
        std::set<std::string> seenVars;
        size_t counter = 0;
        if (isRecursiveCall(right->getFunctionSymbol())) {
            right->addVariablesToSet(interestingVars);
        } else {
            for (std::list<ref<Polynomial> >::iterator ri = rargs.begin(), re = rargs.end(), li = largs.begin(); ri != re; ++ri, ++li, ++counter) {
                ref<Polynomial> rpol = *ri;
                if (rpol->isVar()) {
                    std::set<std::string> rvarsTmp;
                    rpol->addVariablesToSet(rvarsTmp);
                    std::string rvar = *rvarsTmp.begin();
                    if (counter >= largsSize) {
                        // "new"
                        interestingVars.insert(rvar);
                    } else {
                        std::set<std::string> lvarsTmp;
                        (*li)->addVariablesToSet(lvarsTmp);
                        std::string lvar = *lvarsTmp.begin();
                        if (lvar != rvar) {
                            // in different position
                            interestingVars.insert(rvar);
                        } else if (conservative && m_phiVars.find(rvar) != m_phiVars.end()) {
                            // it's a phi and thus always interesting
                            interestingVars.insert(rvar);
                        } else if (seenVars.find(rvar) != seenVars.end()) {
                            // more than once
                            interestingVars.insert(rvar);
                        } else {
                            // seen it once now
                            seenVars.insert(rvar);
                        }
                    }
                } else {
                    rpol->addVariablesToSet(interestingVars);
                }
            }
        }
        for (std::list<ref<Polynomial> >::iterator li = largs.begin(), le = largs.end(); li != le; ++li) {
            ref<Polynomial> lpol = *li;
            std::set<std::string> tmpVars;
            lpol->addVariablesToSet(tmpVars);
            std::string lvar = *tmpVars.begin();
            if (c_vars.find(lvar) != c_vars.end() || interestingVars.find(lvar) != interestingVars.end()) {
                used.insert(lvar);
            }
        }
        std::map<std::string, std::set<std::string> >::iterator found = m_stillUsed.find(left->getFunctionSymbol());
        if (found == m_stillUsed.end()) {
            m_stillUsed.insert(std::make_pair(left->getFunctionSymbol(), used));
        } else {
            used.insert(found->second.begin(), found->second.end());
            m_stillUsed.erase(found);
            m_stillUsed.insert(std::make_pair(left->getFunctionSymbol(), used));
        }
    }

    std::list<ref<Rule> > res;
    std::map<std::string, std::set<std::string> > stillusedMap;
    std::map<std::string, std::set<unsigned int> > notneededMap;
    std::map<std::string, std::list<std::string> > varsMap;
    for (std::list<ref<Rule> >::iterator i = reachable.begin(), e = reachable.end(); i != e; ++i) {
        ref<Rule> rule = *i;
        std::string leftF = rule->getLeft()->getFunctionSymbol();
        if (stillusedMap.find(leftF) == stillusedMap.end()) {
            stillusedMap.insert(std::make_pair(leftF, getStillUsed(leftF,m_stillUsed, m_numFunctions, m_calls,m_idxFunction,m_functionIdx)));
        }
        if (varsMap.find(leftF) == varsMap.end()) {
            std::list<ref<Polynomial> > polys = rule->getLeft()->getArgs();
            std::list<std::string> vars;
            for (std::list<ref<Polynomial> >::iterator it = polys.begin(), et = polys.end(); it != et; ++it) {
                std::set<std::string> tmpVars;
                (*it)->addVariablesToSet(tmpVars);
                vars.push_back(*tmpVars.begin());
            }
            varsMap.insert(std::make_pair(leftF, vars));
        }
    }
    for (std::list<ref<Rule> >::iterator i = reachable.begin(), e = reachable.end(); i != e; ++i) {
        ref<Rule> rule = *i;
        std::string rightF = rule->getRight()->getFunctionSymbol();
        if (stillusedMap.find(rightF) == stillusedMap.end()) {
            if (isRecursiveCall(rightF) && notneededMap.find(rightF) == notneededMap.end()) {
                // keep everything
                notneededMap.insert(std::make_pair(rightF, std::set<unsigned int>()));
            } else {
                stillusedMap.insert(std::make_pair(rightF, getStillUsed(rightF,m_stillUsed, m_numFunctions, m_calls,m_idxFunction,m_functionIdx)));
            }
        }
    }
    for (std::map<std::string, std::set<std::string> >::iterator it = stillusedMap.begin(), et = stillusedMap.end(); it != et; ++it) {
        std::string f = it->first;
        if (notneededMap.find(f) == notneededMap.end()) {
            std::set<std::string> stillused = it->second;
            std::map<std::string, std::list<std::string> >::iterator varsi = varsMap.find(f);
            std::set<unsigned int> notneeded;
            if (varsi != varsMap.end()) {
                unsigned int i = 0;
                std::list<std::string> vars = varsi->second;
                for (std::list<std::string>::iterator vi = vars.begin(), ve = vars.end(); vi != ve; ++vi, ++i) {
                    std::string var = *vi;
                    if (stillused.find(var) == stillused.end()) {
                        notneeded.insert(i);
                    }
                }
            }
            notneededMap.insert(std::make_pair(f, notneeded));
        }
    }
    for (std::list<ref<Rule> >::iterator i = reachable.begin(), e = reachable.end(); i != e; ++i) {
        ref<Rule> rule = *i;
        std::set<unsigned int> lnotneeded = notneededMap.find(rule->getLeft()->getFunctionSymbol())->second;
        std::set<unsigned int> rnotneeded = notneededMap.find(rule->getRight()->getFunctionSymbol())->second;
        ref<Rule> newRule = Rule::create(rule->getLeft()->dropArgs(lnotneeded), rule->getRight()->dropArgs(rnotneeded), rule->getConstraint());
        res.push_back(newRule);
    }

    return res;
}

/**
 * side input:
 *
 * m_stillUsed
 * m_numFunctions
 * m_calls
 *
 * @param f
 * @return
 */
std::set<std::string> Slicer::getStillUsed(std::string f,
                                           std::map<std::string, std::set<std::string> > m_stillUsed,
                                           unsigned int m_numFunctions,
                                           bool* m_calls,
                                           std::map<unsigned int, std::string>& m_idxFunction,
                                           std::map<std::string, unsigned int> m_functionIdx
)
{
    std::set<std::string> res;
    std::set<std::string> fstillused = m_stillUsed.find(f)->second;
    res.insert(fstillused.begin(), fstillused.end());
    unsigned int fidx = getIdxFunction(f,m_functionIdx);
    for (unsigned int i = 0; i < m_numFunctions; ++i) {
        if (m_calls[fidx + i * m_numFunctions] && !isRecursiveCall(getFunction(i,m_idxFunction))) {
            std::set<std::string> sstillused = m_stillUsed.find(getFunction(i,m_idxFunction))->second;
            res.insert(sstillused.begin(), sstillused.end());
        }
    }
    return res;
}

// Helpers
void Slicer::makePreceedsTransitive(unsigned int m_numFunctions,bool* m_preceeds)
{
    for (unsigned int y = 0; y < m_numFunctions; ++y) {
        for (unsigned int x = 0; x < m_numFunctions; ++x) {
            if (m_preceeds[x + m_numFunctions * y]) {
                // x calls y
                for (unsigned int j = 0; j < m_numFunctions; ++j) {
                    if (m_preceeds[y + m_numFunctions * j]) {
                        // y calls j --> x calls j
                        m_preceeds[x + m_numFunctions * j] = true;
                    }
                }
            }
        }
    }
}

/**
 * 传递closure
 * 于是、call graph?
 * @param m_numFunctions
 * @param m_calls  SIDE EFFECT!
 *
 */
void Slicer::makeCallsTransitive(unsigned int m_numFunctions,bool* m_calls)
{
    for (unsigned int y = 0; y < m_numFunctions; ++y) {
        for (unsigned int x = 0; x < m_numFunctions; ++x) {
            if (m_calls[x + m_numFunctions * y]) {
                // x calls y
                for (unsigned int j = 0; j < m_numFunctions; ++j) {
                    if (m_calls[y + m_numFunctions * j]) {
                        // y calls j --> x calls j
                        m_calls[x + m_numFunctions * j] = true;
                    }
                }
            }
        }
    }
}

/**
* m_depends 在且仅在
 * Slicer::makeDependsTransitive(void)
 * Slicer::sliceConstraint
 * 使用
 * 传递闭包吗
*/
void Slicer::makeDependsTransitive(unsigned int m_numVars, bool *m_depends)
{
    for (unsigned int y = 0; y < m_numVars; ++y) {
        for (unsigned int x = 0; x < m_numVars; ++x) {
            if (m_depends[x + m_numVars * y]) {
                // x depends on y
                for (unsigned int j = 0; j < m_numVars; ++j) {
                    if (m_depends[y + m_numVars * j]) {
                        // y depends on j --> x depends on j
                        m_depends[x + m_numVars * j] = true;
                    }
                }
            }
        }
    }
}

std::string Slicer::getFunction(unsigned int idx,std::map<unsigned int, std::string>& m_idxFunction)
{
    std::map<unsigned int, std::string>::iterator found = m_idxFunction.find(idx);
    if (found == m_idxFunction.end()) {
        std::cerr << "Internal error in Slicer::getFunction (" << __FILE__ << ":" << __LINE__ << ")!" << std::endl;
        exit(123);
    } else {
        return found->second;
    }
}

unsigned int Slicer::getIdxFunction(std::string f,std::map<std::string, unsigned int> m_functionIdx)
{
    std::map<std::string, unsigned int>::iterator found = m_functionIdx.find(f);
    if (found == m_functionIdx.end()) {
        std::cerr << "Internal error in Slicer::getIdxFunction (" << __FILE__ << ":" << __LINE__ << ")!" << std::endl;
        exit(123);
    } else {
        return found->second;
    }
}

bool Slicer::isRecursiveCall(std::string f)
{
    std::string end = f.substr(f.length() - 5);
    return (end == "start");
}

bool Slicer::isNondef(std::string v)
{
    return (v.substr(0, 7) == "nondef.");
}

/**
 * [5/6]
 * 没有任何side输入！
 * @param rules
 * @return
 */
std::list<ref<Rule> > Slicer::sliceTrivialNondefConstraints(std::list<ref<Rule> > rules)
{
    // Remove constraints of the form 'Polynomial Operator nondef' if nondef is not
    // used anywhere else, i.e. on a rhs or in another constraint. A constraint of
    // this form can always be satisfied and, hence, and safely be removed.

    std::list<ref<Rule> > res;

    for (std::list<ref<Rule> >::iterator i = rules.begin(), e = rules.end(); i != e; ++i) {
        ref<Rule> rule = *i;
        std::list<ref<Polynomial> > rhsArgs = rule->getRight()->getArgs();
        std::set<std::string> rhsVars;
        rule->getRight()->addVariablesToSet(rhsVars);

        // Map from a nondef variable name to its atomic. If the nondef variable name is
        // used more than once in a constraint, or used on the right-hand side of a rewrite
        // rule we map the variable name to NULL.
        std::map<std::string, ref<Constraint> > nondefToAtomic;

        for (std::set<std::string>::iterator vi = rhsVars.begin(), ve = rhsVars.end(); vi != ve; ++vi) {
            if (isNondef(*vi)) {
                nondefToAtomic[*vi] = NULL;
            }
        }

        std::list<ref<Constraint> > atomics;
        rule->getConstraint()->addAtomicsToList(atomics);
        for (std::list<ref<Constraint> >::iterator ai = atomics.begin(), ae = atomics.end(); ai != ae; ++ai) {
            std::set<std::string> tmp;
            (*ai)->addVariablesToSet(tmp);
            for (std::set<std::string>::iterator vi = tmp.begin(), ve = tmp.end(); vi != ve; ++vi) {
                if (isNondef(*vi)) {
                    if (nondefToAtomic.find(*vi) != nondefToAtomic.end()) {
                        nondefToAtomic[*vi] = NULL;
                    } else {
                        nondefToAtomic[*vi] = *ai;
                    }
                }
            }
        }

        ref<Constraint> newConstraint = rule->getConstraint();
        for (std::map<std::string, ref<Constraint> >::iterator ai = nondefToAtomic.begin(), ae = nondefToAtomic.end(); ai != ae; ++ai) {
            if (ai->second.isNull()) {
                continue;
            }
            ref<Atom> atomic = static_cast<Atom*>(ai->second.get());
            std::set<std::string> tmpLeft;
            std::set<std::string> tmpRight;
            atomic->getLeft()->addVariablesToSet(tmpLeft);
            atomic->getRight()->addVariablesToSet(tmpRight);

            if (atomic->getLeft()->isVar() && tmpLeft.find(ai->first) != tmpLeft.end()) {
              newConstraint = newConstraint->setAtomToTrue(atomic);
            } else if (atomic->getRight()->isVar() && tmpRight.find(ai->first) != tmpRight.end()) {
              newConstraint = newConstraint->setAtomToTrue(atomic);
            }
        }

        ref<Rule> newRule = Rule::create(rule->getLeft(), rule->getRight(), newConstraint);
        res.push_back(newRule);
    }

    return res;
}

/**
 * [6/6]
 * 太好了！纯函数
 * @param rules
 * @return
 */
std::list<ref<Rule> > Slicer::sliceDuplicates(std::list<ref<Rule> > rules)
{
    std::list<ref<Rule> > res;

    for (std::list<ref<Rule> >::iterator i = rules.begin(), e = rules.end(); i != e; ++i) {
        bool newRule = true;
        for (std::list<ref<Rule> >::iterator ri = res.begin(), re = res.end(); ri != re; ++ri) {
            if ((*i)->equals(*ri)) {
                newRule = false;
            }
        }
        if (newRule) {
            res.push_back(*i);
        }
    }

    return res;
}

