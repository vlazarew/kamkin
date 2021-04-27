#include <iostream>

#include "ltl.h"
#include "fsm.h"


using namespace model::ltl;
using namespace model::fsm;
using namespace std;

void getAllFormulas(const Formula &source, std::vector<Formula> &result, std::vector<Formula> &atoms) {
    if (&source.lhs() != nullptr) {
        result.push_back(source);
        getAllFormulas(source.lhs(), result, atoms);
    }

    if (&source.rhs() != nullptr) {
        getAllFormulas(source.rhs(), result, atoms);
    }

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

    if (canAddSource) {
        result.push_back(source);
        if (source.kind() == model::ltl::Formula::ATOM) {
            atoms.push_back(source);
        }
    }
}

void updateFormulaMap(const Formula &formula, vector<pair<Formula::Kind, string>> &map) {
    if (&formula != nullptr) {
        map.push_back(pair<Formula::Kind, string>(formula.kind(), formula.prop()));
    }

    if (&formula.lhs() != nullptr) {
        updateFormulaMap(formula.lhs(), map);
    }

    if (&formula.rhs() != nullptr) {
        updateFormulaMap(formula.rhs(), map);
    }
}

void createCopyFormula(const Formula &formula) {
    vector<pair<Formula::Kind, string>> formulaMap;
    updateFormulaMap(formula, formulaMap);
    int i = 1;
//    return formula.kind();
}


int main() {
    setlocale(LC_ALL, "");
    const Formula &formula = G(P("p") >> F(P("q")));

    std::cout << "Formula: " << std::endl;
    std::cout << formula << std::endl << std::endl;

//    Formula simplifiedFormula = createCopyFormula(formula);
    createCopyFormula(formula);

    std::vector<Formula> allFormulas;
    std::vector<Formula> allAtoms;
    if (&formula.lhs() != nullptr) {
        getAllFormulas(formula.lhs(), allFormulas, allAtoms);
    }
    if (&formula.rhs() != nullptr) {
        getAllFormulas(formula.rhs(), allFormulas, allAtoms);
    }

    std::cout << "Все подвыражения: " << std::endl;
    for (int i = 0; i < allFormulas.size(); i++) {
        std::cout << allFormulas.at(i) << ((i + 1) == allFormulas.size() ? "" : ", ");
    }

    cout << endl;
    cout << endl;

    std::cout << "Атомы: " << std::endl;
    for (int i = 0; i < allAtoms.size(); i++) {
        std::cout << allAtoms.at(i) << ((i + 1) == allAtoms.size() ? "" : ", ");
    }

    cout << endl;
    cout << endl;

    int i = 1;

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


