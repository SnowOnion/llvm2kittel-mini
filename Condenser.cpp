//
// Created by lee on 17-3-22.
//

#include "Condenser.h"
#include "Term.h"
#include "Constraint.h"

// F! ganniniang include bao

#include "Polynomial.h"

#include "Slicer.h"

// C++ includes
#include <iomanip>
#include <iostream>
#include <sstream>

using std::cout;
using std::endl;

std::list<std::string> Condenser::calc_vars_from_rules(const std::list<ref<Rule> > &rulesToCondense) {
    std::set<std::string> vars;
    (*rulesToCondense.front()).addVariablesToSet(vars);
    std::list<std::string> m_vars;
    for (auto i = vars.begin(), e = vars.end(); i != e; ++i) {
        m_vars.push_back(*i);
    }
    return m_vars;
}

std::list<ref<Rule> > Condenser::purifiedGetCondensedRules(
        std::list<ref<Rule>> m_rules,
        std::set<std::string> m_controlPoints,
        std::list<std::string> m_vars
) {
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
                        for (std::list<std::string>::iterator vi = m_vars.begin(), ve = m_vars.end();
                             vi != ve; ++vi, ++ai) {
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
