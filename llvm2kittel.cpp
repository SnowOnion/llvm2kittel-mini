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
#include "Condenser.h"

// C++ includes
#include <iomanip>
#include <iostream>
#include <sstream>

using std::cout;
using std::endl;


void printRules(std::list<ref<Rule> > rules, std::string message = "-") {
    std::cout << "====Outputting " << message << ":" << std::endl;
    for (auto i = rules.begin(), e = rules.end(); i != e; ++i) {
        std::cout << (*i)->toKittelString() << std::endl;
    }
    std::cout << "====Outputting " << message << " done." << std::endl;
}


int main(int argc, char *argv[]) {

    //// 1/3 用 Kittel 的数据结构构造 5 个 rule 列表，并打印出来

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


    std::list<ref<Rule> > myKittelizedRulesWithMoreVar({Rule::create(
            Term::create("eval_main_start",
                         std::list<ref<Polynomial> >(
                                 {Polynomial::create("v_y.0"),
                                  Polynomial::create("v_r.0"),
                                  Polynomial::create("v_1"),
                                  Polynomial::create("baka")})),
            Term::create("eval_main_bb0_in",
                         std::list<ref<Polynomial> >(
                                 {Polynomial::create("v_y.0"),
                                  Polynomial::create("v_r.0"),
                                  Polynomial::create("v_1"),
                                  Polynomial::create("baka")})),
            True::create()), Rule::create(
            Term::create("eval_main_bb0_in", std::list<ref<Polynomial> >(
                    {Polynomial::create("v_y.0"), Polynomial::create("v_r.0"), Polynomial::create("v_1"),
                     Polynomial::create("baka")})),
            Term::create("eval_main_bb1_in", std::list<ref<Polynomial> >(
                    {Polynomial::create("nondef.0"), Polynomial::create("1"), Polynomial::create("v_1"),
                     Polynomial::create("baka")})),
            True::create()), Rule::create(
            Term::create("eval_main_bb1_in", std::list<ref<Polynomial> >(
                    {Polynomial::create("v_y.0"), Polynomial::create("v_r.0"), Polynomial::create("v_1"),
                     Polynomial::create("baka")})),
            Term::create("eval_main_bb1_in", std::list<ref<Polynomial> >(
                    {Polynomial::create("v_y.0")->sub(Polynomial::one), Polynomial::create("0"),
                     Polynomial::create("v_y.0")->sub(Polynomial::one), Polynomial::create("baka")})),
            Atom::create(Polynomial::create("v_y.0"), Polynomial::null, Atom::AType::Gtr)), Rule::create(
            Term::create("eval_main_bb1_in", std::list<ref<Polynomial> >(
                    {Polynomial::create("v_y.0"), Polynomial::create("v_r.0"), Polynomial::create("v_1"),
                     Polynomial::create("baka")})),
            Term::create("eval_main_bb2_in", std::list<ref<Polynomial> >(
                    {Polynomial::create("v_y.0"), Polynomial::create("v_r.0"),
                     Polynomial::create("v_y.0")->sub(Polynomial::one), Polynomial::create("baka")})),
            Atom::create(Polynomial::create("v_y.0"), Polynomial::null, Atom::AType::Leq)), Rule::create(
            Term::create("eval_main_bb2_in", std::list<ref<Polynomial> >(
                    {Polynomial::create("v_y.0"), Polynomial::create("v_r.0"), Polynomial::create("v_1"),
                     Polynomial::create("baka")})),
            Term::create("eval_main_stop", std::list<ref<Polynomial> >(
                    {Polynomial::create("v_y.0"), Polynomial::create("v_r.0"), Polynomial::create("v_1"),
                     Polynomial::create("baka")})),
            True::create())
                                                       });


    std::list<ref<Rule> > myKittelizedRulesWithMoreRule({
                                                                Rule::create(
                                                                        Term::create("eval_main_start",
                                                                                     std::list<ref<Polynomial> >(
                                                                                             {Polynomial::create(
                                                                                                     "v_y.0"),
                                                                                              Polynomial::create(
                                                                                                      "v_r.0"),
                                                                                              Polynomial::create(
                                                                                                      "v_1")})),
                                                                        Term::create("eval_main_bb0_in",
                                                                                     std::list<ref<Polynomial> >(
                                                                                             {Polynomial::create(
                                                                                                     "v_y.0"),
                                                                                              Polynomial::create(
                                                                                                      "v_r.0"),
                                                                                              Polynomial::create(
                                                                                                      "v_1")})),
                                                                        True::create()), Rule::create(
                    Term::create("eval_main_bb0_in", std::list<ref<Polynomial> >(
                            {Polynomial::create("v_y.0"), Polynomial::create("v_r.0"), Polynomial::create("v_1")})),
                    Term::create("weird_state", std::list<ref<Polynomial> >(
                            {Polynomial::create("v_y.0"), Polynomial::create("v_r.0"), Polynomial::create("v_1")})),
                    True::create()), Rule::create(
                    Term::create("weird_state", std::list<ref<Polynomial> >(
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

    std::list<ref<Rule> > myKittelizedRulesIsomorphismVar({
                                                                  Rule::create(
                                                                          Term::create("eval_main_start",
                                                                                       std::list<ref<Polynomial> >(
                                                                                               {Polynomial::create(
                                                                                                       "yyy"),
                                                                                                Polynomial::create(
                                                                                                        "rrr"),
                                                                                                Polynomial::create(
                                                                                                        "ichi")})),
                                                                          Term::create("eval_main_bb0_in",
                                                                                       std::list<ref<Polynomial> >(
                                                                                               {Polynomial::create(
                                                                                                       "yyy"),
                                                                                                Polynomial::create(
                                                                                                        "rrr"),
                                                                                                Polynomial::create(
                                                                                                        "ichi")})),
                                                                          True::create()), Rule::create(
                    Term::create("eval_main_bb0_in", std::list<ref<Polynomial> >(
                            {Polynomial::create("yyy"), Polynomial::create("rrr"), Polynomial::create("ichi")})),
                    Term::create("eval_main_bb1_in", std::list<ref<Polynomial> >(
                            {Polynomial::create("nondef.0"), Polynomial::create("1"), Polynomial::create("ichi")})),
                    True::create()), Rule::create(
                    Term::create("eval_main_bb1_in", std::list<ref<Polynomial> >(
                            {Polynomial::create("yyy"), Polynomial::create("rrr"), Polynomial::create("ichi")})),
                    Term::create("eval_main_bb1_in", std::list<ref<Polynomial> >(
                            {Polynomial::create("yyy")->sub(Polynomial::one), Polynomial::create("0"),
                             Polynomial::create("yyy")->sub(Polynomial::one)})),
                    Atom::create(Polynomial::create("yyy"), Polynomial::null, Atom::AType::Gtr)), Rule::create(
                    Term::create("eval_main_bb1_in", std::list<ref<Polynomial> >(
                            {Polynomial::create("yyy"), Polynomial::create("rrr"), Polynomial::create("ichi")})),
                    Term::create("eval_main_bb2_in", std::list<ref<Polynomial> >(
                            {Polynomial::create("yyy"), Polynomial::create("rrr"),
                             Polynomial::create("yyy")->sub(Polynomial::one)})),
                    Atom::create(Polynomial::create("yyy"), Polynomial::null, Atom::AType::Leq)), Rule::create(
                    Term::create("eval_main_bb2_in", std::list<ref<Polynomial> >(
                            {Polynomial::create("yyy"), Polynomial::create("rrr"), Polynomial::create("ichi")})),
                    Term::create("eval_main_stop", std::list<ref<Polynomial> >(
                            {Polynomial::create("yyy"), Polynomial::create("rrr"), Polynomial::create("ichi")})),
                    True::create())
                                                          });

    std::list<ref<Rule> > myKittelizedRulesIsomorphismVarAndFunctionSymbol({
                                                                 Rule::create(
                                                                         Term::create("state0",
                                                                                      std::list<ref<Polynomial> >(
                                                                                              {Polynomial::create(
                                                                                                      "yyy"),
                                                                                               Polynomial::create(
                                                                                                       "rrr"),
                                                                                               Polynomial::create(
                                                                                                       "ichi")})),
                                                                         Term::create("state1",
                                                                                      std::list<ref<Polynomial> >(
                                                                                              {Polynomial::create(
                                                                                                      "yyy"),
                                                                                               Polynomial::create(
                                                                                                       "rrr"),
                                                                                               Polynomial::create(
                                                                                                       "ichi")})),
                                                                         True::create()), Rule::create(
                    Term::create("state1", std::list<ref<Polynomial> >(
                            {Polynomial::create("yyy"), Polynomial::create("rrr"), Polynomial::create("ichi")})),
                    Term::create("state2", std::list<ref<Polynomial> >(
                            {Polynomial::create("nondef.0"), Polynomial::create("1"), Polynomial::create("ichi")})),
                    True::create()), Rule::create(
                    Term::create("state2", std::list<ref<Polynomial> >(
                            {Polynomial::create("yyy"), Polynomial::create("rrr"), Polynomial::create("ichi")})),
                    Term::create("state2", std::list<ref<Polynomial> >(
                            {Polynomial::create("yyy")->sub(Polynomial::one), Polynomial::create("0"),
                             Polynomial::create("yyy")->sub(Polynomial::one)})),
                    Atom::create(Polynomial::create("yyy"), Polynomial::null, Atom::AType::Gtr)), Rule::create(
                    Term::create("state2", std::list<ref<Polynomial> >(
                            {Polynomial::create("yyy"), Polynomial::create("rrr"), Polynomial::create("ichi")})),
                    Term::create("state3", std::list<ref<Polynomial> >(
                            {Polynomial::create("yyy"), Polynomial::create("rrr"),
                             Polynomial::create("yyy")->sub(Polynomial::one)})),
                    Atom::create(Polynomial::create("yyy"), Polynomial::null, Atom::AType::Leq)), Rule::create(
                    Term::create("state3", std::list<ref<Polynomial> >(
                            {Polynomial::create("yyy"), Polynomial::create("rrr"), Polynomial::create("ichi")})),
                    Term::create("state4", std::list<ref<Polynomial> >(
                            {Polynomial::create("yyy"), Polynomial::create("rrr"), Polynomial::create("ichi")})),
                    True::create())
                                                         });


    printRules(myKittelizedRules, "myKittelizedRules");
    printRules(myKittelizedRulesWithMoreRule, "myKittelizedRulesWithMoreRule");
    printRules(myKittelizedRulesWithMoreVar, "myKittelizedRulesWithMoreVar");
    printRules(myKittelizedRulesIsomorphismVar, "myKittelizedRulesIsomorphismVar");
    printRules(myKittelizedRulesIsomorphismVarAndFunctionSymbol, "myKittelizedRulesIsomorphismVarAndFunctionSymbol");

    //// 2/3 以 myKittelizedRulesWithMoreRule 为例，演示 condense。
    //// 注意：必须手动指定在 condense 后留下的 FunctionSymbol 集合（Condenser::purifiedGetCondensedRules() 的第二个参数）。


    auto &rulesToCondense = myKittelizedRulesWithMoreRule; /// 配置位置 1/2 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    auto m_vars = Condenser::calc_vars_from_rules(rulesToCondense);
    auto condensedRules = Condenser::purifiedGetCondensedRules(
            rulesToCondense,
//            std::set<std::string>({"eval_main_bb0_in","eval_main_bb1_in","eval_main_bb2_in","eval_main_start","eval_main_stop"}),
            std::set<std::string>(
                    {"eval_main_bb0_in", "eval_main_bb1_in", "eval_main_bb2_in", "eval_main_start", "eval_main_stop",
                     "weird_state"}),
            m_vars
    );
    printRules(condensedRules, "condensedRules");

    //// 3/3 以 myKittelizedRulesIsomorphismVarAndFunctionSymbol 为例，演示 slice。
    //// 注意：这里假定第一条 rule 的左边的 FunctionSymbol 是初始 FunctionSymbol。

    auto &rulesToSlice = myKittelizedRulesIsomorphismVarAndFunctionSymbol; /// 配置位置 2/2 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // 这里假定第一条 rule 的左边的 FunctionSymbol 是初始 FunctionSymbol。
    std::string startFunctionSymbolName = rulesToSlice.front()->getLeft()->getFunctionSymbol();

    std::list<ref<Rule> > slicedRules;
    slicedRules = Slicer::sliceUsage(rulesToSlice);
    printRules(slicedRules, "[1/6] after sliceUsage");
    slicedRules = Slicer::sliceConstraint(slicedRules);
    printRules(slicedRules, "[2/6] after sliceConstraint");
    slicedRules = Slicer::sliceDefined(slicedRules, startFunctionSymbolName);
    printRules(slicedRules, "[3/6] after sliceDefined");
    slicedRules = Slicer::sliceStillUsed(slicedRules, startFunctionSymbolName);
    printRules(slicedRules, "[4/6] after sliceStillUsed");
    slicedRules = Slicer::sliceTrivialNondefConstraints(slicedRules);
    printRules(slicedRules, "[5/6] after sliceTrivialNondefConstraints");
    slicedRules = Slicer::sliceDuplicates(slicedRules);
    printRules(slicedRules, "[6/6] after sliceDuplicates");


    return 0;
}
