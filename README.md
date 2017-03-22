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
