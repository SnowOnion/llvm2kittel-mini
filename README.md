# llvm2kittel-mini
---

The condensing and slicing part of https://github.com/s-falke/llvm2kittel

# Install

[INSTALL](INSTALL)

# 主函数

主函数在 llvm2kittel.cpp 里，它：

+ 1/3 用 Kittel 的数据结构构造 5 个 rule 列表，并打印出来

+ 2/3 以 myKittelizedRulesWithMoreRule 为例，演示 condense。
    + 注意：必须手动指定在 condense 后留下的 state 集合（Condenser::purifiedGetCondensedRules() 的第二个参数）。

+ 3/3 以 myKittelizedRulesIsomorphismVarAndFunctionSymbol 为例，演示 slice。
    + 注意：这里假定第一条 rule 的左边的 FunctionSymbol 是初始 FunctionSymbol。

## 预期输出


    ====Outputting myKittelizedRules:
    eval_main_start(v_y.0, v_r.0, v_1) -> eval_main_bb0_in(v_y.0, v_r.0, v_1)
    eval_main_bb0_in(v_y.0, v_r.0, v_1) -> eval_main_bb1_in(nondef.0, 1, v_1)
    eval_main_bb1_in(v_y.0, v_r.0, v_1) -> eval_main_bb1_in(v_y.0 - 1, 0, v_y.0 - 1) [ v_y.0 > 0 ]
    eval_main_bb1_in(v_y.0, v_r.0, v_1) -> eval_main_bb2_in(v_y.0, v_r.0, v_y.0 - 1) [ v_y.0 <= 0 ]
    eval_main_bb2_in(v_y.0, v_r.0, v_1) -> eval_main_stop(v_y.0, v_r.0, v_1)
    ====Outputting myKittelizedRules done.
    ====Outputting myKittelizedRulesWithMoreRule:
    eval_main_start(v_y.0, v_r.0, v_1) -> eval_main_bb0_in(v_y.0, v_r.0, v_1)
    eval_main_bb0_in(v_y.0, v_r.0, v_1) -> weird_state(v_y.0, v_r.0, v_1)
    weird_state(v_y.0, v_r.0, v_1) -> eval_main_bb1_in(nondef.0, 1, v_1)
    eval_main_bb1_in(v_y.0, v_r.0, v_1) -> eval_main_bb1_in(v_y.0 - 1, 0, v_y.0 - 1) [ v_y.0 > 0 ]
    eval_main_bb1_in(v_y.0, v_r.0, v_1) -> eval_main_bb2_in(v_y.0, v_r.0, v_y.0 - 1) [ v_y.0 <= 0 ]
    eval_main_bb2_in(v_y.0, v_r.0, v_1) -> eval_main_stop(v_y.0, v_r.0, v_1)
    ====Outputting myKittelizedRulesWithMoreRule done.
    ====Outputting myKittelizedRulesWithMoreVar:
    eval_main_start(v_y.0, v_r.0, v_1, baka) -> eval_main_bb0_in(v_y.0, v_r.0, v_1, baka)
    eval_main_bb0_in(v_y.0, v_r.0, v_1, baka) -> eval_main_bb1_in(nondef.0, 1, v_1, baka)
    eval_main_bb1_in(v_y.0, v_r.0, v_1, baka) -> eval_main_bb1_in(v_y.0 - 1, 0, v_y.0 - 1, baka) [ v_y.0 > 0 ]
    eval_main_bb1_in(v_y.0, v_r.0, v_1, baka) -> eval_main_bb2_in(v_y.0, v_r.0, v_y.0 - 1, baka) [ v_y.0 <= 0 ]
    eval_main_bb2_in(v_y.0, v_r.0, v_1, baka) -> eval_main_stop(v_y.0, v_r.0, v_1, baka)
    ====Outputting myKittelizedRulesWithMoreVar done.
    ====Outputting myKittelizedRulesIsomorphismVar:
    eval_main_start(yyy, rrr, ichi) -> eval_main_bb0_in(yyy, rrr, ichi)
    eval_main_bb0_in(yyy, rrr, ichi) -> eval_main_bb1_in(nondef.0, 1, ichi)
    eval_main_bb1_in(yyy, rrr, ichi) -> eval_main_bb1_in(yyy - 1, 0, yyy - 1) [ yyy > 0 ]
    eval_main_bb1_in(yyy, rrr, ichi) -> eval_main_bb2_in(yyy, rrr, yyy - 1) [ yyy <= 0 ]
    eval_main_bb2_in(yyy, rrr, ichi) -> eval_main_stop(yyy, rrr, ichi)
    ====Outputting myKittelizedRulesIsomorphismVar done.
    ====Outputting myKittelizedRulesIsomorphismVarAndFunctionSymbol:
    state0(yyy, rrr, ichi) -> state1(yyy, rrr, ichi)
    state1(yyy, rrr, ichi) -> state2(nondef.0, 1, ichi)
    state2(yyy, rrr, ichi) -> state2(yyy - 1, 0, yyy - 1) [ yyy > 0 ]
    state2(yyy, rrr, ichi) -> state3(yyy, rrr, yyy - 1) [ yyy <= 0 ]
    state3(yyy, rrr, ichi) -> state4(yyy, rrr, ichi)
    ====Outputting myKittelizedRulesIsomorphismVarAndFunctionSymbol done.
    ====Outputting condensedRules:
    eval_main_start(v_y.0, v_r.0, v_1) -> eval_main_bb0_in(v_y.0, v_r.0, v_1)
    eval_main_bb0_in(v_y.0, v_r.0, v_1) -> weird_state(v_y.0, v_r.0, v_1)
    weird_state(v_y.0, v_r.0, v_1) -> eval_main_bb1_in(nondef.0, 1, v_1)
    eval_main_bb1_in(v_y.0, v_r.0, v_1) -> eval_main_bb1_in(v_y.0 - 1, 0, v_y.0 - 1) [ v_y.0 > 0 ]
    eval_main_bb1_in(v_y.0, v_r.0, v_1) -> eval_main_bb2_in(v_y.0, v_r.0, v_y.0 - 1) [ v_y.0 <= 0 ]
    eval_main_bb2_in(v_y.0, v_r.0, v_1) -> eval_main_stop(v_y.0, v_r.0, v_1)
    ====Outputting condensedRules done.
    ====Outputting [1/6] after sliceUsage:
    state0(yyy) -> state1(yyy)
    state1(yyy) -> state2(nondef.0)
    state2(yyy) -> state2(yyy - 1) [ yyy > 0 ]
    state2(yyy) -> state3(yyy) [ yyy <= 0 ]
    state3(yyy) -> state4(yyy)
    ====Outputting [1/6] after sliceUsage done.
    ====Outputting [2/6] after sliceConstraint:
    state0(yyy) -> state1(yyy)
    state1(yyy) -> state2(nondef.0)
    state2(yyy) -> state2(yyy - 1) [ yyy > 0 ]
    state2(yyy) -> state3(yyy) [ yyy <= 0 ]
    state3(yyy) -> state4(yyy)
    ====Outputting [2/6] after sliceConstraint done.
    ====Outputting [3/6] after sliceDefined:
    state0() -> state1()
    state1() -> state2(nondef.0)
    state2(yyy) -> state2(yyy - 1) [ yyy > 0 ]
    state2(yyy) -> state3() [ yyy <= 0 ]
    state3() -> state4()
    ====Outputting [3/6] after sliceDefined done.
    ====Outputting [4/6] after sliceStillUsed:
    state0() -> state1()
    state1() -> state2(nondef.0)
    state2(yyy) -> state2(yyy - 1) [ yyy > 0 ]
    state2(yyy) -> state3() [ yyy <= 0 ]
    state3() -> state4()
    ====Outputting [4/6] after sliceStillUsed done.
    ====Outputting [5/6] after sliceTrivialNondefConstraints:
    state0() -> state1()
    state1() -> state2(nondef.0)
    state2(yyy) -> state2(yyy - 1) [ yyy > 0 ]
    state2(yyy) -> state3() [ yyy <= 0 ]
    state3() -> state4()
    ====Outputting [5/6] after sliceTrivialNondefConstraints done.
    ====Outputting [6/6] after sliceDuplicates:
    state0() -> state1()
    state1() -> state2(nondef.0)
    state2(yyy) -> state2(yyy - 1) [ yyy > 0 ]
    state2(yyy) -> state3() [ yyy <= 0 ]
    state3() -> state4()
    ====Outputting [6/6] after sliceDuplicates done.

    Process finished with exit code 0

# 2017-03-26

- [] gmp，构造数儿
- [] 构造非原子 constraint
- [] kittelize, smtsolver