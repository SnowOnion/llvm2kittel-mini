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

#include "Term.h"
#include "Polynomial.h"

// C++ includes
#include <sstream>

Term::Term(std::string f, std::list<ref<Polynomial> > args)
  : refCount(0),
    m_f(f),
    m_args(args),
    m_vars()
{}

ref<Term> Term::create(std::string f, std::list<ref<Polynomial> > args)
{
    return new Term(f, args);
}

Term::~Term()
{}

std::string Term::toString()
{
    std::ostringstream res;
    res << m_f << '(';
    for (std::list<ref<Polynomial> >::iterator i = m_args.begin(), e = m_args.end(); i != e; ) {
        ref<Polynomial> tmp = *i;
        res << tmp->toString();
        if (++i != e) {
            res << ", ";
        }
    }
    res << ')';
    return res.str();
}

std::string Term::getFunctionSymbol()
{
    return m_f;
}

std::list<ref<Polynomial> > Term::getArgs()
{
    return m_args;
}

ref<Polynomial> Term::getArg(unsigned int i)
{
    unsigned int c = 0;
    for (std::list<ref<Polynomial> >::iterator it = m_args.begin(), et = m_args.end(); it != et; ++it) {
        if (c == i) {
            return *it;
        }
        ++c;
    }
    return ref<Polynomial>();
}

ref<Term> Term::instantiate(std::map<std::string, ref<Polynomial> > *bindings)
{
    std::list<ref<Polynomial> > newargs;
    for (std::list<ref<Polynomial> >::iterator i = m_args.begin(), e = m_args.end(); i != e; ++i) {
        ref<Polynomial> pol = *i;
        newargs.push_back(pol->instantiate(bindings));
    }
    return create(m_f, newargs);
}

void Term::addVariablesToSet(std::set<std::string> &res)
{
    setupVars();
    for (std::vector<std::set<std::string> >::iterator i = m_vars.begin(), e = m_vars.end(); i != e; ++i) {
        res.insert(i->begin(), i->end());
    }
}

void Term::addVariablesToSet(unsigned int argpos, std::set<std::string> &res)
{
    setupVars();
    std::set<std::string> &tmp = m_vars[argpos];
    res.insert(tmp.begin(), tmp.end());
}

void Term::setupVars(void)
{
    if (m_args.size() != m_vars.size()) {
        m_vars.clear();
        for (std::list<ref<Polynomial> >::iterator i = m_args.begin(), e = m_args.end(); i != e; ++i) {
            ref<Polynomial> tmp = *i;
            std::set<std::string> vars;
            tmp->addVariablesToSet(vars);
            m_vars.push_back(vars);
        }
    }
}

ref<Term> Term::dropArgs(std::set<unsigned int> drop)
{
    std::list<ref<Polynomial> > newargs;
    unsigned int argc = 0;
    for (std::list<ref<Polynomial> >::iterator i = m_args.begin(), e = m_args.end(); i != e; ++i, ++argc) {
        if (drop.find(argc) == drop.end()) {
            ref<Polynomial> pol = *i;
            newargs.push_back(pol);
        }
    }
    return create(m_f, newargs);
}

bool Term::equals(ref<Term> term)
{
    if (m_f != term->m_f) {
        return false;
    }

    if (m_args.size() != term->m_args.size()) {
        return false;
    }

    for (std::list<ref<Polynomial> >::iterator i = m_args.begin(), e = m_args.end(), ti = term->m_args.begin(); i != e; ++i, ++ti) {
        if (!(*i)->equals(*ti)) {
            return false;
        }
    }

    return true;
}
