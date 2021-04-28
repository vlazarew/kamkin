#include <iostream>

#include "ltl.h"
#include "fsm.h"


void checkNextFormulaNode(std::vector<std::pair<const model::ltl::Formula &, int>> &fixedFormula, int level,
                          bool needToCheckOnCurrentLevel);

bool canAddFormulaToVector(const model::ltl::Formula &source, std::vector<model::ltl::Formula> &result);

using namespace model::ltl;
using namespace model::fsm;
using namespace std;

void getClosure(const Formula &source, std::vector<Formula> &result, std::vector<Formula> &atoms) {
    if (&source.lhs() != nullptr) {
        result.push_back(source);
        getClosure(source.lhs(), result, atoms);
    }

    if (&source.rhs() != nullptr) {
        getClosure(source.rhs(), result, atoms);
    }

    bool canAddSource = canAddFormulaToVector(source, result);

    if (canAddSource) {
        result.push_back(source);
        if (source.kind() == model::ltl::Formula::ATOM) {
            atoms.push_back(source);
        }
    }
}

bool canAddFormulaToVector(const Formula &source, vector<Formula> &result) {
    bool canAddSource = true;
    for (const auto &tempFormula : result) {
        if (&tempFormula.lhs() == &source.lhs()
            && &tempFormula.rhs() == &source.rhs()
            && &tempFormula.arg() == &source.arg()
            && tempFormula.kind() == source.kind()
            && tempFormula.prop() == source.prop()) {
            canAddSource = false;
            break;
        }
    }
    return canAddSource;
}

bool canAddFormulaToVector(const Formula &source, int level, vector<pair<const Formula &, int>> &result) {
    bool canAddSource = true;
    for (const auto &tempFormula : result) {
        if (&tempFormula.first == &source
            && &tempFormula.first.lhs() == &source.lhs()
            && &tempFormula.first.rhs() == &source.rhs()
            && &tempFormula.first.arg() == &source.arg()
            && tempFormula.first.kind() == source.kind()
            && tempFormula.first.prop() == source.prop()
            && level == tempFormula.second) {
            canAddSource = false;
            break;
        }
    }
    return canAddSource;
}

//void updateFormulaMap(const Formula &formula, vector<pair<Formula::Kind, string>> &map) {
//    if (&formula != nullptr) {
//        map.push_back(pair<Formula::Kind, string>(formula.kind(), formula.prop()));
//    }
//
//    if (&formula.lhs() != nullptr) {
//        updateFormulaMap(formula.lhs(), map);
//    }
//
//    if (&formula.rhs() != nullptr) {
//        updateFormulaMap(formula.rhs(), map);
//    }
//}


void simplifyFormula(const Formula &formula, vector<pair<const Formula &, int>> &fixedFormula, int level = 0) {
    if (&formula == nullptr) {
        return;
    }

    bool edited = false;
    if (formula.kind() == Formula::IMPL && !edited) {
        Formula a = formula.lhs().operator!();
        Formula b = a.operator||(formula.rhs());
        edited = true;
        if (canAddFormulaToVector(b, level, fixedFormula)) {
            fixedFormula.push_back(pair<Formula, int>(b, level));
            checkNextFormulaNode(fixedFormula, level, true);
        }
    }

    if (formula.kind() == Formula::G && !edited) {
        const Formula &a = !F(!formula.lhs());
        edited = true;
        if (canAddFormulaToVector(a, level, fixedFormula)) {
            fixedFormula.push_back(pair<const Formula &, int>(a, level));
            checkNextFormulaNode(fixedFormula, level, true);
        }
    }

    if (formula.kind() == Formula::F && !edited) {
        const Formula &a = U(P("1"), formula.lhs());
        edited = true;
        if (canAddFormulaToVector(a, level, fixedFormula)) {
            fixedFormula.push_back(pair<const Formula &, int>(a, level));
            checkNextFormulaNode(fixedFormula, level, true);
        }
    }

    if (!edited) {
        if (canAddFormulaToVector(formula, level, fixedFormula)) {
            fixedFormula.push_back(pair<const Formula &, int>(formula, level));
            checkNextFormulaNode(fixedFormula, level, false);
        }
    }

}

void checkNextFormulaNode(vector<pair<const Formula &, int>> &fixedFormula, int level, bool needToCheckOnCurrentLevel) {
    const Formula &tempFormula = fixedFormula.at(fixedFormula.size() - 1).first;

    if (needToCheckOnCurrentLevel && &tempFormula != nullptr) {
        simplifyFormula(tempFormula, fixedFormula, level);
    }

    if (&tempFormula.lhs() != nullptr) {
        simplifyFormula(tempFormula.lhs(), fixedFormula, level + 1);
    }

    if (&tempFormula.rhs() != nullptr) {
        simplifyFormula(tempFormula.rhs(), fixedFormula, level + 1);
    }
}

void createCopyFormula(const Formula &formula) {
    vector<pair<const Formula &, int>> fixedFormula;
//    fixedFormula.resize(1);
//    vector<Formula> fixedFormula;
//    updateFormulaMap(formula, formulaMap);
    simplifyFormula(formula, fixedFormula, 0);
    int i = 1;
//    return formula.kind();
}

vector<Formula> getElem(vector<Formula> closure) {
    int i = 0;
    vector<Formula> result;
    while (i < closure.size()) {
        Formula formula = closure.at(i);
        if (formula.kind() == Formula::X) {
            if (formula.arg().kind() == model::ltl::Formula::NOT) {
                closure.push_back(X(formula.arg().arg()));
            } else if (formula.arg().kind() == model::ltl::Formula::OR) {
                closure.push_back(X(formula.arg().lhs()));
                closure.push_back(X(formula.arg().rhs()));
            } else {
                result.push_back(formula);
            }
        }

        if (formula.kind() == model::ltl::Formula::ATOM) {
            result.push_back(formula);
        }
        i++;
    }

    return result;
}

Formula simplify(Formula formula, vector<Formula> &temp) {
    if (formula.kind() == model::ltl::Formula::F) {
        Formula a = U(P("1"), formula.lhs());
        temp.push_back(a);
        return U(P("1"), simplify(formula.lhs(), temp));
    } else if (formula.kind() == model::ltl::Formula::G) {
        Formula a = F(!(formula.lhs()));
        temp.push_back(a);
        return simplify(F(!(formula.lhs())), temp);
    } else if (formula.kind() == model::ltl::Formula::X) {
        Formula a = X(formula.lhs());
        temp.push_back(a);
        return X(simplify(formula.lhs(), temp));
    } else if (formula.kind() == model::ltl::Formula::ATOM) {
        switch (formula.kind()) {
            case model::ltl::Formula::NOT: {
                Formula a = !formula.lhs();
                temp.push_back(a);
                return !(simplify(formula.lhs(), temp));
            }
            case model::ltl::Formula::AND: {
                Formula a = formula.lhs() && formula.rhs();
                temp.push_back(a);
                return simplify(formula.lhs(), temp) && simplify(formula.rhs(), temp);
            }
            case model::ltl::Formula::OR: {
                Formula a = formula.lhs() || formula.rhs();
                temp.push_back(a);
                return simplify(formula.lhs(), temp) || simplify(formula.rhs(), temp);
            }
            default:
                return formula;
        }
    } else if (formula.kind() == model::ltl::Formula::R) {
        Formula a = !U(!formula.lhs(), !formula.rhs());
        temp.push_back(a);
        return !U(!simplify(formula.lhs(), temp), !simplify(formula.rhs(), temp));
    } else if (formula.kind() == model::ltl::Formula::NOT) {
        Formula a = !(formula.lhs());
        temp.push_back(a);
        return !(simplify(formula.lhs(), temp));
    } else if (formula.kind() == model::ltl::Formula::IMPL) {
        Formula a = (formula.lhs()).operator!().operator||(formula.rhs());
        temp.push_back(a);
        return (simplify(formula.lhs(), temp)).operator!().operator||(simplify(formula.rhs(), temp));
    }
    return formula;
}


int main() {
    setlocale(LC_ALL, "");
//    const Formula &formula = G(P("p") >> F(P("q")));

    vector<Formula> temp;
    Formula formula = simplify(G(P("p") >> F(P("q"))), temp);

    std::cout << "Formula: " << std::endl;
    std::cout << formula << std::endl << std::endl;

//    Formula simplifiedFormula = createCopyFormula(formula);
//    createCopyFormula(formula);

    std::vector<Formula> allFormulas;
    std::vector<Formula> allAtoms;
//    if (&formula.lhs() != nullptr) {
//        getClosure(formula.lhs(), allFormulas, allAtoms);
//    }
//    if (&formula.rhs() != nullptr) {
//        getClosure(formula.rhs(), allFormulas, allAtoms);
//    }
//
//    std::cout << "Все подвыражения: " << std::endl;
//    for (int i = 0; i < allFormulas.size(); i++) {
//        std::cout << allFormulas.at(i) << ((i + 1) == allFormulas.size() ? "" : ", ");
//    }
//
//    cout << endl;
//    cout << endl;
//
//    std::cout << "Атомы: " << std::endl;
//    for (int i = 0; i < allAtoms.size(); i++) {
//        std::cout << allAtoms.at(i) << ((i + 1) == allAtoms.size() ? "" : ", ");
//    }
//
//    cout << endl;
//    cout << endl;
//
//
//    std::vector<Formula> elem = getElem(allFormulas);
//    std::cout << "getElem: " << std::endl;
//    for (int i = 0; i < elem.size(); i++) {
//        std::cout << elem.at(i) << ((i + 1) == elem.size() ? "" : ", ");
//    }
//
//    cout << endl;
//    cout << endl;
//
//    int i = 1;

    /*Automaton automaton;
    automaton.add_state("s0");
    automaton.add_state("s1");
    automaton.add_state("s2");
    automaton.set_initial("s0");
    automaton.set_final("s1", 0);
    automaton.set_final("s2", 1);
    automaton.add_trans("s0", { "p" }, "s1");
    automaton.add_trans("s0", { "q" }, "s2");
    automaton.add_trans("s1", {}, "s0");
    automaton.add_trans("s1", { "p", "q" }, "s2");
    automaton.add_trans("s2", {}, "s2");

    std::cout << "Automaton: " << std::endl;
    std::cout << automaton << std::endl;*/

    return 0;
}


