// This file is part of llvm2KITTeL
//
// Copyright 2010-2014 Stephan Falke
//
// Licensed under the University of Illinois/NCSA Open Source License.
// See LICENSE for details.
//
// After cutting, what should the licence be? hahahaha

// Just to satisfy:
// /usr/lib/llvm-3.4/include/llvm/Support/DataTypes.h:48:3: error: #error "Must #define __STDC_LIMIT_MACROS before #including Support/DataTypes.h"
// # error "Must #define __STDC_LIMIT_MACROS before #including Support/DataTypes.h"
// /usr/lib/llvm-3.4/include/llvm/Support/DataTypes.h:52:3: error: #error "Must #define __STDC_CONSTANT_MACROS before " "#including Support/DataTypes.h"
// # error "Must #define __STDC_CONSTANT_MACROS before "
#define __STDC_LIMIT_MACROS 0
#define __STDC_CONSTANT_MACROS 0

#include "Ref.h"
#include "Term.h"
#include "Polynomial.h"
#include "Constraint.h"
#include "Rule.h"

#include "Slicer.h"

// C++ includes
#include <iomanip>
#include <iostream>
#include <sstream>
#include <cstdlib>

using std::cout;
using std::endl;



void printRules(std::list<ref<Rule> > rules,std::string message="-"){
    std::cout<<"====Outputting "<<message<<":"<<std::endl;
    for (auto i = rules.begin(), e = rules.end(); i != e; ++i) {
        std::cout << (*i)->toKittelString() << std::endl;
    }
    std::cout<<"====Outputting "<<message<<" done."<<std::endl;
}

std::list<std::string> calc_vars_from_rules(const std::list<ref<Rule> >& rulesToCondense){
    std::set<std::string> vars;
    (*rulesToCondense.front()).addVariablesToSet(vars);
    std::list<std::string> m_vars;
    for(auto i=vars.begin(),e=vars.end();i!=e;++i){
        m_vars.push_back(*i);
    }
    return m_vars;
}

std::list<ref<Rule> > purifiedGetCondensedRules(
        std::list<ref<Rule>> m_rules,
        std::set<std::string> m_controlPoints,
        std::list<std::string> m_vars
){
    std::list<ref<Rule> > good;
    std::list<ref<Rule> > junk;
    std::list<ref<Rule> > res;
    for (std::list<ref<Rule> >::iterator i = m_rules.begin(), e = m_rules.end(); i != e; ++i) {
        ref<Rule> rule = *i;
        std::string f = rule->getLeft()->getFunctionSymbol();
        if (m_controlPoints.find(f) != m_controlPoints.end()) {
            good.push_back(rule);
        } else {
            junk.push_back(rule);
        }
    }
    for (std::list<ref<Rule> >::iterator i = good.begin(), e = good.end(); i != e; ++i) {
        ref<Rule> rule = *i;
        std::vector<ref<Rule> > todo; // 当成stack用的呀。第一个塞前塞后无所谓，之后都塞到头里。
        todo.push_back(rule);
        while (!todo.empty()) {
            ref<Rule> r = *todo.begin();
            todo.erase(todo.begin());
            ref<Term> rhs = r->getRight();
            std::string f = rhs->getFunctionSymbol();
            if (m_controlPoints.find(f) != m_controlPoints.end()) {
                res.push_back(r);
            } else {
                std::list<ref<Rule> > newtodo;
                for (std::list<ref<Rule> >::iterator ii = junk.begin(), ee = junk.end(); ii != ee; ++ii) {
                    ref<Rule> junkrule = *ii;
                    if (junkrule->getLeft()->getFunctionSymbol() == f) {
                        std::map<std::string, ref<Polynomial> > subby;
                        std::list<ref<Polynomial> > rhsargs = rhs->getArgs();
                        std::list<ref<Polynomial> >::iterator ai = rhsargs.begin();
                        for (std::list<std::string>::iterator vi = m_vars.begin(), ve = m_vars.end(); vi != ve; ++vi, ++ai) {
                            subby.insert(std::make_pair(*vi, *ai));
                        }
                        ref<Rule> newRule = Rule::create(
                                r->getLeft(),
                                junkrule->getRight()->instantiate(&subby),
                                Operator::create(r->getConstraint(),
                                                 junkrule->getConstraint()->instantiate(&subby),
                                                 Operator::And));
                        newtodo.push_back(newRule);
                    }
                }
                todo.insert(todo.begin(), newtodo.begin(), newtodo.end());
            }
        }
    }
    return res;
}
/*
 list
 set
 string

 Ref.h
 Rule.h
 Term.h
 Polynomial.h
 Constraint.h
 Slicer.h

 */
int main(int argc, char *argv[])
{

    // 插入自己构造的 kittelized rule!
    std::list<ref<Rule> > myKittelizedRules({Rule::create(
            Term::create("eval_main_start", std::list<ref<Polynomial> >(
                    {Polynomial::create("v_y.0"),
                     Polynomial::create("v_r.0"),
                     Polynomial::create("v_1")})),
            Term::create("eval_main_bb0_in",
                         std::list<ref<Polynomial> >(
                                 {Polynomial::create("v_y.0"),
                                  Polynomial::create("v_r.0"),
                                  Polynomial::create("v_1")})),
            True::create()), Rule::create(
            Term::create("eval_main_bb0_in", std::list<ref<Polynomial> >(
                    {Polynomial::create("v_y.0"), Polynomial::create("v_r.0"), Polynomial::create("v_1")})),
            Term::create("eval_main_bb1_in", std::list<ref<Polynomial> >(
                    {Polynomial::create("nondef.0"), Polynomial::create("1"), Polynomial::create("v_1")})),
            True::create()), Rule::create(
            Term::create("eval_main_bb1_in", std::list<ref<Polynomial> >(
                    {Polynomial::create("v_y.0"), Polynomial::create("v_r.0"), Polynomial::create("v_1")})),
            Term::create("eval_main_bb1_in", std::list<ref<Polynomial> >(
                    {Polynomial::create("v_y.0")->sub(Polynomial::one), Polynomial::create("0"),
                     Polynomial::create("v_y.0")->sub(Polynomial::one)})),
            Atom::create(Polynomial::create("v_y.0"), Polynomial::null, Atom::AType::Gtr)), Rule::create(
            Term::create("eval_main_bb1_in", std::list<ref<Polynomial> >(
                    {Polynomial::create("v_y.0"), Polynomial::create("v_r.0"), Polynomial::create("v_1")})),
            Term::create("eval_main_bb2_in", std::list<ref<Polynomial> >(
                    {Polynomial::create("v_y.0"), Polynomial::create("v_r.0"),
                     Polynomial::create("v_y.0")->sub(Polynomial::one)})),
            Atom::create(Polynomial::create("v_y.0"), Polynomial::null, Atom::AType::Leq)), Rule::create(
            Term::create("eval_main_bb2_in", std::list<ref<Polynomial> >(
                    {Polynomial::create("v_y.0"), Polynomial::create("v_r.0"), Polynomial::create("v_1")})),
            Term::create("eval_main_stop", std::list<ref<Polynomial> >(
                    {Polynomial::create("v_y.0"), Polynomial::create("v_r.0"), Polynomial::create("v_1")})),
            True::create())
                                            });


        // slice 会去掉多余的变量
    std::list<ref<Rule> > myKittelizedRulesWithMoreVar({
                                                               Rule::create(
                                                                       Term::create("eval_main_start",std::list<ref<Polynomial> >({Polynomial::create("v_y.0"),Polynomial::create("v_r.0"),Polynomial::create("v_1"),Polynomial::create("baka")})),
                                                                       Term::create("eval_main_bb0_in",std::list<ref<Polynomial> >({Polynomial::create("v_y.0"),Polynomial::create("v_r.0"),Polynomial::create("v_1"),Polynomial::create("baka")})),
                                                                       True::create())
                                                               ,Rule::create(
                    Term::create("eval_main_bb0_in",std::list<ref<Polynomial> >({Polynomial::create("v_y.0"),Polynomial::create("v_r.0"),Polynomial::create("v_1"),Polynomial::create("baka")})),
                    Term::create("eval_main_bb1_in",std::list<ref<Polynomial> >({Polynomial::create("nondef.0"),Polynomial::create("1"),Polynomial::create("v_1"),Polynomial::create("baka")})),
                    True::create())
                                                               ,Rule::create(
                    Term::create("eval_main_bb1_in",std::list<ref<Polynomial> >({Polynomial::create("v_y.0"),Polynomial::create("v_r.0"),Polynomial::create("v_1"),Polynomial::create("baka")})),
                    Term::create("eval_main_bb1_in",std::list<ref<Polynomial> >({Polynomial::create("v_y.0")->sub(Polynomial::one),Polynomial::create("0"),Polynomial::create("v_y.0")->sub(Polynomial::one),Polynomial::create("baka")})),
                    Atom::create(Polynomial::create("v_y.0"),Polynomial::null,Atom::AType::Gtr))
                                                               ,Rule::create(
                    Term::create("eval_main_bb1_in",std::list<ref<Polynomial> >({Polynomial::create("v_y.0"),Polynomial::create("v_r.0"),Polynomial::create("v_1"),Polynomial::create("baka")})),
                    Term::create("eval_main_bb2_in",std::list<ref<Polynomial> >({Polynomial::create("v_y.0"),Polynomial::create("v_r.0"),Polynomial::create("v_y.0")->sub(Polynomial::one),Polynomial::create("baka")})),
                    Atom::create(Polynomial::create("v_y.0"),Polynomial::null,Atom::AType::Leq))
                                                               ,Rule::create(
                    Term::create("eval_main_bb2_in",std::list<ref<Polynomial> >({Polynomial::create("v_y.0"),Polynomial::create("v_r.0"),Polynomial::create("v_1"),Polynomial::create("baka")})),
                    Term::create("eval_main_stop",std::list<ref<Polynomial> >({Polynomial::create("v_y.0"),Polynomial::create("v_r.0"),Polynomial::create("v_1"),Polynomial::create("baka")})),
                    True::create())
                                                       });

    // slice 不会去掉 more rule 的。
    std::list<ref<Rule> > myKittelizedRulesWithMoreRule({
                                                                Rule::create(
                                                                        Term::create("eval_main_start",std::list<ref<Polynomial> >({Polynomial::create("v_y.0"),Polynomial::create("v_r.0"),Polynomial::create("v_1")})),
                                                                        Term::create("eval_main_bb0_in",std::list<ref<Polynomial> >({Polynomial::create("v_y.0"),Polynomial::create("v_r.0"),Polynomial::create("v_1")})),
                                                                        True::create())
                                                                ,Rule::create(
                    Term::create("eval_main_bb0_in",std::list<ref<Polynomial> >({Polynomial::create("v_y.0"),Polynomial::create("v_r.0"),Polynomial::create("v_1")})),
                    Term::create("weird_state",std::list<ref<Polynomial> >({Polynomial::create("v_y.0"),Polynomial::create("v_r.0"),Polynomial::create("v_1")})),
                    True::create())
                                                                ,Rule::create(
                    Term::create("weird_state",std::list<ref<Polynomial> >({Polynomial::create("v_y.0"),Polynomial::create("v_r.0"),Polynomial::create("v_1")})),
                    Term::create("eval_main_bb1_in",std::list<ref<Polynomial> >({Polynomial::create("nondef.0"),Polynomial::create("1"),Polynomial::create("v_1")})),
                    True::create())
                                                                ,Rule::create(
                    Term::create("eval_main_bb1_in",std::list<ref<Polynomial> >({Polynomial::create("v_y.0"),Polynomial::create("v_r.0"),Polynomial::create("v_1")})),
                    Term::create("eval_main_bb1_in",std::list<ref<Polynomial> >({Polynomial::create("v_y.0")->sub(Polynomial::one),Polynomial::create("0"),Polynomial::create("v_y.0")->sub(Polynomial::one)})),
                    Atom::create(Polynomial::create("v_y.0"),Polynomial::null,Atom::AType::Gtr))
                                                                ,Rule::create(
                    Term::create("eval_main_bb1_in",std::list<ref<Polynomial> >({Polynomial::create("v_y.0"),Polynomial::create("v_r.0"),Polynomial::create("v_1")})),
                    Term::create("eval_main_bb2_in",std::list<ref<Polynomial> >({Polynomial::create("v_y.0"),Polynomial::create("v_r.0"),Polynomial::create("v_y.0")->sub(Polynomial::one)})),
                    Atom::create(Polynomial::create("v_y.0"),Polynomial::null,Atom::AType::Leq))
                                                                ,Rule::create(
                    Term::create("eval_main_bb2_in",std::list<ref<Polynomial> >({Polynomial::create("v_y.0"),Polynomial::create("v_r.0"),Polynomial::create("v_1")})),
                    Term::create("eval_main_stop",std::list<ref<Polynomial> >({Polynomial::create("v_y.0"),Polynomial::create("v_r.0"),Polynomial::create("v_1")})),
                    True::create())
                                                        });
    // 变量同构地改名，仍然能slice…… 所以phi根本没用？ // 猜对了
    std::list<ref<Rule> > myKittelizedRulesIsomorphismVar({
                                                                  Rule::create(
                                                                          Term::create("eval_main_start",std::list<ref<Polynomial> >({Polynomial::create("yyy"),Polynomial::create("rrr"),Polynomial::create("ichi")})),
                                                                          Term::create("eval_main_bb0_in",std::list<ref<Polynomial> >({Polynomial::create("yyy"),Polynomial::create("rrr"),Polynomial::create("ichi")})),
                                                                          True::create())
                                                                  ,Rule::create(
                    Term::create("eval_main_bb0_in",std::list<ref<Polynomial> >({Polynomial::create("yyy"),Polynomial::create("rrr"),Polynomial::create("ichi")})),
                    Term::create("eval_main_bb1_in",std::list<ref<Polynomial> >({Polynomial::create("nondef.0"),Polynomial::create("1"),Polynomial::create("ichi")})),
                    True::create())
                                                                  ,Rule::create(
                    Term::create("eval_main_bb1_in",std::list<ref<Polynomial> >({Polynomial::create("yyy"),Polynomial::create("rrr"),Polynomial::create("ichi")})),
                    Term::create("eval_main_bb1_in",std::list<ref<Polynomial> >({Polynomial::create("yyy")->sub(Polynomial::one),Polynomial::create("0"),Polynomial::create("yyy")->sub(Polynomial::one)})),
                    Atom::create(Polynomial::create("yyy"),Polynomial::null,Atom::AType::Gtr))
                                                                  ,Rule::create(
                    Term::create("eval_main_bb1_in",std::list<ref<Polynomial> >({Polynomial::create("yyy"),Polynomial::create("rrr"),Polynomial::create("ichi")})),
                    Term::create("eval_main_bb2_in",std::list<ref<Polynomial> >({Polynomial::create("yyy"),Polynomial::create("rrr"),Polynomial::create("yyy")->sub(Polynomial::one)})),
                    Atom::create(Polynomial::create("yyy"),Polynomial::null,Atom::AType::Leq))
                                                                  ,Rule::create(
                    Term::create("eval_main_bb2_in",std::list<ref<Polynomial> >({Polynomial::create("yyy"),Polynomial::create("rrr"),Polynomial::create("ichi")})),
                    Term::create("eval_main_stop",std::list<ref<Polynomial> >({Polynomial::create("yyy"),Polynomial::create("rrr"),Polynomial::create("ichi")})),
                    True::create())
                                                          });
    // 状态(还是叫函数？)同构地改名，不能slice了！得到slicedRules为空
    std::list<ref<Rule> > myKittelizedRulesIsoVarAndFunc({
                                                                 Rule::create(
                                                                         Term::create("state0",std::list<ref<Polynomial> >({Polynomial::create("yyy"),Polynomial::create("rrr"),Polynomial::create("ichi")})),
                                                                         Term::create("state1",std::list<ref<Polynomial> >({Polynomial::create("yyy"),Polynomial::create("rrr"),Polynomial::create("ichi")})),
                                                                         True::create())
                                                                 ,Rule::create(
                    Term::create("state1",std::list<ref<Polynomial> >({Polynomial::create("yyy"),Polynomial::create("rrr"),Polynomial::create("ichi")})),
                    Term::create("state2",std::list<ref<Polynomial> >({Polynomial::create("nondef.0"),Polynomial::create("1"),Polynomial::create("ichi")})),
                    True::create())
                                                                 ,Rule::create(
                    Term::create("state2",std::list<ref<Polynomial> >({Polynomial::create("yyy"),Polynomial::create("rrr"),Polynomial::create("ichi")})),
                    Term::create("state2",std::list<ref<Polynomial> >({Polynomial::create("yyy")->sub(Polynomial::one),Polynomial::create("0"),Polynomial::create("yyy")->sub(Polynomial::one)})),
                    Atom::create(Polynomial::create("yyy"),Polynomial::null,Atom::AType::Gtr))
                                                                 ,Rule::create(
                    Term::create("state2",std::list<ref<Polynomial> >({Polynomial::create("yyy"),Polynomial::create("rrr"),Polynomial::create("ichi")})),
                    Term::create("state3",std::list<ref<Polynomial> >({Polynomial::create("yyy"),Polynomial::create("rrr"),Polynomial::create("yyy")->sub(Polynomial::one)})),
                    Atom::create(Polynomial::create("yyy"),Polynomial::null,Atom::AType::Leq))
                                                                 ,Rule::create(
                    Term::create("state3",std::list<ref<Polynomial> >({Polynomial::create("yyy"),Polynomial::create("rrr"),Polynomial::create("ichi")})),
                    Term::create("state4",std::list<ref<Polynomial> >({Polynomial::create("yyy"),Polynomial::create("rrr"),Polynomial::create("ichi")})),
                    True::create())
                                                         });

    printRules(myKittelizedRules,"myKittelizedRules");
            printRules(myKittelizedRulesWithMoreRule,"myKittelizedRulesWithMoreRule");
            printRules(myKittelizedRulesWithMoreVar,"myKittelizedRulesWithMoreVar");
            printRules(myKittelizedRulesIsomorphismVar,"myKittelizedRulesIsomorphismVar");
            printRules(myKittelizedRulesIsoVarAndFunc,"myKittelizedRulesIsoVarAndFunc");

    auto& rulesToCondense=myKittelizedRulesWithMoreRule; /// 配置位置 1/2 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    auto m_vars=calc_vars_from_rules(rulesToCondense);
    auto condensedRules = purifiedGetCondensedRules(
            rulesToCondense,
//            std::set<std::string>({"eval_main_bb0_in","eval_main_bb1_in","eval_main_bb2_in","eval_main_start","eval_main_stop"}),
            std::set<std::string>({"eval_main_bb0_in","eval_main_bb1_in","eval_main_bb2_in","eval_main_start","eval_main_stop","weird_state"}),
            m_vars
            );
    printRules(condensedRules,"condensedRules");

    auto& rulesToSlice = myKittelizedRulesIsoVarAndFunc; /// 配置位置 2/2 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // 这里假定第一条rule的左边是最开始。
    std::string startFunctionSymbolName = rulesToSlice.front()->getLeft()->getFunctionSymbol();

    std::list<ref<Rule> > slicedRules;
    slicedRules=Slicer::sliceUsage(rulesToSlice);
    printRules(slicedRules,"[1/6] after sliceUsage");
    slicedRules = Slicer::sliceConstraint(slicedRules);
    printRules(slicedRules,"[2/6] after sliceConstraint");
    slicedRules = Slicer::sliceDefined(slicedRules,startFunctionSymbolName);
    printRules(slicedRules,"[3/6] after sliceDefined");
    slicedRules = Slicer::sliceStillUsed(slicedRules, startFunctionSymbolName);
    printRules(slicedRules,"[4/6] after sliceStillUsed");
    slicedRules = Slicer::sliceTrivialNondefConstraints(slicedRules);
    printRules(slicedRules,"[5/6] after sliceTrivialNondefConstraints");
    slicedRules = Slicer::sliceDuplicates(slicedRules);
    printRules(slicedRules,"[6/6] after sliceDuplicates");


    return 0;
}
