//
// Created by lee on 17-3-22.
//

#ifndef LLVM2KITTEL_MINI_CONDENSER_H
#define LLVM2KITTEL_MINI_CONDENSER_H

#include<list>
#include<string>
#include "Ref.h"
#include "Rule.h"


class Condenser {
public:
    static std::list<std::string> calc_vars_from_rules(const std::list<ref<Rule> > &rulesToCondense);

    static std::list<ref<Rule> > purifiedGetCondensedRules(
            std::list<ref<Rule>> m_rules,
            std::set<std::string> m_controlPoints,
            std::list<std::string> m_vars
    );
};


#endif //LLVM2KITTEL_MINI_CONDENSER_H
