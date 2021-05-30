/*
 * Copyright 2021 ISP RAS (http://www.ispras.ru)
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except
 * in compliance with the License. You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License
 * is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
 * or implied. See the License for the specific language governing permissions and limitations under
 * the License.
 */

#include<string>
#include<cmath>
#include<stack>
#include<unordered_set>
#include<functional>
#include "ltl.h"
#include "fsm.h"

using namespace model::ltl;
using namespace model::fsm;
using namespace std;


string printFormula(const Formula &formula) {
    switch (formula.kind()) {
        case Formula::ATOM:
            return "(ATOM" + formula.prop() + ")";
        case Formula::NOT:
            return "NOT(" + printFormula(formula.lhs()) + ")";
        case Formula::AND:
            return "(" + printFormula(formula.lhs()) + "AND" + printFormula(formula.rhs()) + ")";
        case Formula::OR:
            return "(" + printFormula(formula.lhs()) + "OR" + printFormula(formula.rhs()) + ")";
        case Formula::IMPL:
            return "(" + printFormula(formula.lhs()) + "IMPL" + printFormula(formula.rhs()) + ")";
        case Formula::X:
            return "X(" + printFormula(formula.lhs()) + ")";
        case Formula::G:
            return "G(" + printFormula(formula.lhs()) + ")";
        case Formula::F:
            return "F(" + printFormula(formula.lhs()) + ")";
        case Formula::U:
            return "(" + printFormula(formula.lhs()) + "U" + printFormula(formula.rhs()) + ")";
        case Formula::R:
            return "(" + printFormula(formula.lhs()) + "R" + printFormula(formula.rhs()) + ")";
    };
    return "";
}

// Получаем хеш для нашего объекта формула для добавления в сеты
template<>
struct std::hash<Formula> {
    int operator()(Formula const &formula) const {
        hash<string> hashFunc;
        return hashFunc(printFormula(formula));
    }
};

Formula getSimplifiedFormula(vector<Formula> &initFormulas);

void simplifyKindX(vector<Formula> &formulaStack, const Formula &latestFormula);

void simplifyKindX_Or(vector<Formula> &formulaStack, const Formula &latestFormula);

void simplifyKindX_Not(vector<Formula> &formulaStack, const Formula &latestFormula);

void simplifyLindX_Other(vector<Formula> &formulaStack, const Formula &latestFormula);

void simplifyKindF(vector<Formula> &formulaStack, const Formula &latestFormula);

void simplifyKindG(vector<Formula> &formulaStack, const Formula &latestFormula);

void simplifyKindNot(vector<Formula> &formulaStack, const Formula &latestFormula);

void simplifyKindAnd(vector<Formula> &formulaStack, const Formula &latestFormula);

void simplifyKindOr(vector<Formula> &formulaStack, const Formula &latestFormula);

void simplifyImpl(vector<Formula> &formulaStack, const Formula &latestFormula);

unordered_set<Formula> getTrueSet(unordered_set<Formula> &atoms, int i);

ostream &operator<<(ostream &out, vector<Formula> &formulas) {
    unsigned long sizeOfVector = formulas.size() - 1;
    for (int i = 0; i < sizeOfVector; i++) {
        out << formulas.at(i) << ", ";
    }

    out << formulas.at(sizeOfVector);
    return out;
}

ostream &operator<<(ostream &out, unordered_set<Formula> &formulas) {
    for (const auto &formula : formulas) {
        out << formula << ", ";
    }
    return out;
}

ostream &operator<<(ostream &out, vector<unordered_set<Formula>> &formulas) {
    for (auto &formula : formulas) {
        out << "[ " << formula << " ]" << endl;
    }
    return out;
}

// Перебор всех подформул исходной формулы
void createEnclosure(Formula formula, unordered_set<Formula> &closure) {
    closure.insert(formula);
    if (formula.kind() == Formula::ATOM) {
        return;
    } else if (formula.kind() == Formula::X) {
        createEnclosure(formula.arg(), closure);
    } else if (formula.kind() == Formula::F) {
        createEnclosure(formula.lhs(), closure);
    } else if (formula.kind() == Formula::G) {
        createEnclosure(formula.lhs(), closure);
    } else if (formula.kind() == Formula::NOT) {
        createEnclosure(formula.lhs(), closure);
    } else {
        createEnclosure(formula.lhs(), closure);
        createEnclosure(formula.rhs(), closure);
    }
}

void getAtoms(unordered_set<Formula> closure, bool &hasTrue, unordered_set<Formula> &atoms) {
    for (auto &formula: closure) {
        if (formula.kind() == Formula::ATOM && formula.prop() == "1") {
            hasTrue = true;
        } else if (formula.kind() == Formula::X || formula.kind() == Formula::ATOM) {
            atoms.insert(formula);
        }
    }
}


bool isTrue(Formula f, unordered_set<Formula> &trueSet) {
    if (f.kind() == Formula::AND) {
        return isTrue(f.lhs(), trueSet) && isTrue(f.rhs(), trueSet);
    } else if (f.kind() == Formula::OR) {
        return isTrue(f.lhs(), trueSet) || isTrue(f.rhs(), trueSet);
    } else if (f.kind() == Formula::NOT) {
        return !isTrue(f.lhs(), trueSet);
    } else if (f.kind() == Formula::ATOM || f.kind() == Formula::X) {
        return trueSet.find(f) != trueSet.end();
    } else {
        return false;
    }
}

bool hasAtomOrX(Formula f) {
    if (f.kind() == Formula::AND || f.kind() == Formula::OR) {
        return hasAtomOrX(f.lhs()) && hasAtomOrX(f.rhs());
    } else if (f.kind() == Formula::NOT) {
        return hasAtomOrX(f.lhs());
    } else if (f.kind() == Formula::ATOM || f.kind() == Formula::X) {
        return true;
    } else {
        return false;
    }
}

unordered_set<Formula> getS(unordered_set<Formula> trueSet, unordered_set<Formula> &closure) {
    unordered_set<Formula> res;
    for (const auto &it : closure) {
        if (hasAtomOrX(it) && isTrue(it, trueSet)) {
            res.insert(it);
        }
    }
    return res;
}

unordered_set<Formula> getUntil(unordered_set<Formula> &closure) {
    unordered_set<Formula> res;
    for (const auto &it : closure) {
        if (it.kind() == Formula::U) {
            res.insert(it);
        }
    }
    return res;
}

unordered_set<Formula> getX(unordered_set<Formula> &closure) {
    unordered_set<Formula> res;
    for (const auto &it : closure) {
        if (it.kind() == Formula::X) {
            res.insert(it);
        }
    }
    return res;
}

vector<unordered_set<Formula>> makeAtomCombinations(unordered_set<Formula> &atoms, unordered_set<Formula> &closure,
                                                    bool hasTrue) {
    vector<unordered_set<Formula>> combinations(pow(2, atoms.size()));
    for (int i = 0; i < combinations.size(); i++) {
        unordered_set<Formula> trueSet = getTrueSet(atoms, i);

        cout << "True set" << endl;
        cout << trueSet << endl;

        combinations.at(i) = getS(trueSet, closure);
        if (hasTrue) {
            combinations.at(i).insert(P("1"));
        }
    }
    return combinations;
}

unordered_set<Formula> getTrueSet(unordered_set<Formula> &atoms, int index) {
    unordered_set<Formula> trueSet;
    auto k = atoms.begin();
    while (index != 0) {
        if (index % 2 == 1) {
            trueSet.insert(*k);
        } else {
            trueSet.insert(!(*k));
        }
        index /= 2;
        k++;
    }

    while (k != atoms.end()) {
        trueSet.insert(!(*k));
        k++;
    }
    return trueSet;
}

void addUntil(unordered_set<Formula> &untils, vector<unordered_set<Formula>> &S) {
    vector<unordered_set<Formula>> result;
    for (const auto &until : untils) {
        Formula hi = until.lhs();
        Formula psi = until.rhs();
        for (auto &j : S) {
            if (j.find(psi) != j.end()) {
                j.insert(until);
            } else if (j.find(hi) != j.end()) {
                result.push_back(j);
                j.insert(until);
            }
        }
    }

    S.insert(S.end(), result.begin(), result.end());
}

bool isInitState(Formula f, unordered_set<Formula> &trueSet) {
    if (f.kind() == Formula::AND) {
        return isInitState(f.lhs(), trueSet) && isInitState(f.rhs(), trueSet);
    } else if (f.kind() == Formula::OR) {
        return isInitState(f.lhs(), trueSet) || isInitState(f.rhs(), trueSet);
    } else if (f.kind() == Formula::NOT) {
        return !isInitState(f.lhs(), trueSet);
    } else if (f.kind() == Formula::ATOM || f.kind() == Formula::X) {
        return trueSet.find(f) != trueSet.end();
    } else if (f.kind() == Formula::U) {
        return trueSet.find(f) != trueSet.end();
    } else {
        return false;
    }
}

void addStates(Automaton &res, Formula &formula, unordered_set<Formula> &untils,
               vector<unordered_set<Formula>> &states) {
    for (int i = 0; i < states.size(); i++) {
        bool final = false;

        for (const auto &until : untils) {
            Formula psi = until.rhs();
            if ((states.at(i).find(until) == states.at(i).end()) || ((states.at(i).find(psi) != states.at(i).end()))) {
                final = true;
                break;
            }
        }
        res.add_state("s" + to_string(i));

        if (isInitState(formula, states.at(i))) {
            res.set_initial("s" + to_string(i));
        }

        if (final) {
            res.set_final("s" + to_string(i), 1);
        }
    }
}

void addTransition(Automaton &res, int index, vector<unordered_set<Formula>> &states,
                   unordered_set<Formula> &untils, unordered_set<Formula> &closure) {
    unordered_set<Formula> XShould;
    unordered_set<Formula> XShouldnt;
    unordered_set<Formula> UShould;
    unordered_set<Formula> UShouldnt;

    unordered_set<Formula> Xes = getX(closure);
    set<string> atoms = {};

    for (const auto &it : states.at(index)) {
        if (it.kind() == Formula::ATOM) {
            atoms.insert(it.prop());
        }
    }

    for (const auto &Xe : Xes) {
        if (states.at(index).find(Xe) != states.at(index).end()) {
            XShould.insert(Xe.lhs());
        } else {
            XShouldnt.insert(Xe.lhs());
        }
    }

    for (const auto &until : untils) {
        Formula psi = until.rhs();
        Formula hi = until.lhs();
        if (states.at(index).find(until) != states.at(index).end()) {
            if (states.at(index).find(psi) == states.at(index).end()) {
                UShould.insert(until);
            }
        } else {
            if (states.at(index).find(hi) != states.at(index).end()) {
                UShouldnt.insert(until);
            }
        }
    }

    for (int i = 0; i < states.size(); i++) {
        bool isTransition = true;
        for (const auto &it : XShould) {
            if (states.at(i).find(it) == states.at(i).end()) {
                isTransition = false;
                break;
            }
        }
        if (isTransition) {
            for (const auto &it : XShouldnt) {
                if (states.at(i).find(it) != states.at(i).end()) {
                    isTransition = false;
                    break;
                }
            }
        }
        if (isTransition) {
            for (const auto &it : UShould) {
                if (states.at(i).find(it) == states.at(i).end()) {
                    isTransition = false;
                    break;
                }
            }
        }
        if (isTransition) {
            for (const auto &it : UShouldnt) {
                if (states.at(i).find(it) != states.at(i).end()) {
                    isTransition = false;
                    break;
                }
            }
        }
        if (isTransition) {
            res.add_trans("s" + to_string(index), atoms, "s" + to_string(i));
        }
    }
}

void addTransitions(Automaton &automatic, vector<unordered_set<Formula>> &states, unordered_set<Formula> &untils,
                    unordered_set<Formula> &closure) {
    for (int i = 0; i < states.size(); i++) {
        addTransition(automatic, i, states, untils, closure);
    }
}

/////// УПРОЩЕНИЕ
void simplify(vector<Formula> &formulaStack) {
    Formula latestFormula = formulaStack.at(formulaStack.size() - 1);
    if (latestFormula.kind() == Formula::ATOM) {
        return;
    } else if (latestFormula.kind() == Formula::X) {
        simplifyKindX(formulaStack, latestFormula);
    } else if (latestFormula.kind() == Formula::F) {
        simplifyKindF(formulaStack, latestFormula);
    } else if (latestFormula.kind() == Formula::G) {
        simplifyKindG(formulaStack, latestFormula);
    } else if (latestFormula.kind() == Formula::NOT) {
        simplifyKindNot(formulaStack, latestFormula);
    } else if (latestFormula.kind() == Formula::AND) {
        simplifyKindAnd(formulaStack, latestFormula);
    } else if (latestFormula.kind() == Formula::OR) {
        simplifyKindOr(formulaStack, latestFormula);
    } else if (latestFormula.kind() == Formula::IMPL) {
        simplifyImpl(formulaStack, latestFormula);
    }
}

void simplifyImpl(vector<Formula> &formulaStack, const Formula &latestFormula) {
    formulaStack.push_back(latestFormula.lhs());
    simplify(formulaStack);

    int index = formulaStack.size() - 1;
    formulaStack.push_back(latestFormula.rhs());
    simplify(formulaStack);

    formulaStack.push_back(!(formulaStack.at(index)));
    formulaStack.push_back(formulaStack.at(formulaStack.size() - 1) || formulaStack.at(formulaStack.size() - 2));
}

void simplifyKindOr(vector<Formula> &formulaStack, const Formula &latestFormula) {
    formulaStack.push_back(latestFormula.lhs());
    simplify(formulaStack);

    Formula leftFormula = formulaStack.at(formulaStack.size() - 1);
    formulaStack.push_back(latestFormula.rhs());
    simplify(formulaStack);

    formulaStack.push_back(leftFormula || formulaStack.at(formulaStack.size() - 1));
}

void simplifyKindAnd(vector<Formula> &formulaStack, const Formula &latestFormula) {
    formulaStack.push_back(latestFormula.lhs());
    simplify(formulaStack);

    Formula leftFormula = formulaStack.at(formulaStack.size() - 1);
    formulaStack.push_back(latestFormula.rhs());
    simplify(formulaStack);

    formulaStack.push_back(leftFormula && formulaStack.at(formulaStack.size() - 1));
}

void simplifyKindNot(vector<Formula> &formulaStack, const Formula &latestFormula) {
    formulaStack.push_back(latestFormula.lhs());
    simplify(formulaStack);

    formulaStack.push_back(!(formulaStack.at(formulaStack.size() - 1)));
}

void simplifyKindG(vector<Formula> &formulaStack, const Formula &latestFormula) {
    formulaStack.push_back(latestFormula.lhs());
    simplify(formulaStack);

    formulaStack.push_back(P("1"));
    formulaStack.push_back(!(formulaStack.at(formulaStack.size() - 2)));
    formulaStack.push_back(U(formulaStack.at(formulaStack.size() - 2), formulaStack.at(formulaStack.size() - 1)));
    formulaStack.push_back(!(formulaStack.at(formulaStack.size() - 1)));
}

void simplifyKindF(vector<Formula> &formulaStack, const Formula &latestFormula) {
    formulaStack.push_back(P("1"));
    simplify(formulaStack);

    formulaStack.push_back(latestFormula.lhs());
    simplify(formulaStack);

    formulaStack.push_back(U(formulaStack.at(formulaStack.size() - 2), formulaStack.at(formulaStack.size() - 1)));
}

///// Упрощение NEXT + вспомогаельные методы
void simplifyLindX_Other(vector<Formula> &formulaStack, const Formula &latestFormula) {
    formulaStack.push_back(latestFormula.lhs());
    simplify(formulaStack);
    formulaStack.push_back(X(formulaStack.at(formulaStack.size() - 1)));
}

void simplifyKindX_Not(vector<Formula> &formulaStack, const Formula &latestFormula) {
    formulaStack.push_back(latestFormula.lhs().lhs());
    simplify(formulaStack);

    formulaStack.push_back(X(formulaStack.at(formulaStack.size() - 1)));
    formulaStack.push_back(!(formulaStack.at(formulaStack.size() - 1)));
}

void simplifyKindX_Or(vector<Formula> &formulaStack, const Formula &latestFormula) {
    formulaStack.push_back(X(latestFormula.lhs().lhs()));
    simplify(formulaStack);

    int index = formulaStack.size() - 1;
    formulaStack.push_back(X(latestFormula.lhs().rhs()));
    simplify(formulaStack);

    formulaStack.push_back(formulaStack.at(index) || formulaStack.at(formulaStack.size() - 1));
    formulaStack.push_back(X(formulaStack.at(formulaStack.size() - 1)));
}

void simplifyKindX(vector<Formula> &formulaStack, const Formula &latestFormula) {
    if (latestFormula.lhs().kind() == Formula::OR) {
        simplifyKindX_Or(formulaStack, latestFormula);
    } else if (latestFormula.lhs().kind() == Formula::NOT) {
        simplifyKindX_Not(formulaStack, latestFormula);
    } else {
        simplifyLindX_Other(formulaStack, latestFormula);
    }
}

///// КОНЕЦ: Упрощение NEXT + вспомогаельные методы

/////// КОНЕЦ: УПРОЩЕНИЕ

Formula getSimplifiedFormula(vector<Formula> &initFormulas) {
    simplify(initFormulas);

    Formula simplifiedFormula = initFormulas.at(initFormulas.size() - 1);
    return simplifiedFormula;
}

Automaton createAutomatic(Formula &formula, vector<Formula> &initFormulas) {
    //  Упрощение исходной формулы
    Formula simplifiedFormula = getSimplifiedFormula(initFormulas);
    cout << "Simplified formula: " << endl << simplifiedFormula << endl;
//

//    Генерируем замыкание
    unordered_set<Formula> closure;
    createEnclosure(simplifiedFormula, closure);
    cout << "Closure: " << endl << closure << endl << endl;
//

//  Извлекаем атомы
    bool hasTrue = false;
    unordered_set<Formula> atoms;
    getAtoms(closure, hasTrue, atoms);
    cout << "Atoms: " << endl << atoms << endl << endl;
//

//  Получаем все выражения-комбинации атомов
    vector<unordered_set<Formula>> atomsCombinations = makeAtomCombinations(atoms, closure, hasTrue);
//

//  Получаем until-выражения
    unordered_set<Formula> untilExpressions = getUntil(closure);
    cout << "Until Expressions:" << endl << untilExpressions << endl;

    addUntil(untilExpressions, atomsCombinations);
    cout << "With until:" << endl << atomsCombinations << endl;
//

//    Заполнение автомата состояниями + связками
    Automaton automatic;
    addStates(automatic, simplifiedFormula, untilExpressions, atomsCombinations);
    addTransitions(automatic, atomsCombinations, untilExpressions, closure);

    return automatic;
}

int main() {
//    Будем использовать вектор, чтоб не было проблем с памятью и прочей сегментацией, будь она неладна
    vector<Formula> initFormula;
    initFormula.reserve(1000);
//    initFormula.push_back(F(P("p") >> (U(!P("q") && X(!P("p")), P("q")))));
//    initFormula.push_back(U(P("p") || P("q"), (P("p") && P("q"))));
//    initFormula.push_back(P("p") >> P("q"));
//    initFormula.push_back(G(P("p") >> X(P("q"))));
//    initFormula.push_back(X(!(P("q"))));
    initFormula.push_back(G(P("p") >> U((!P("q")) && (X(!(P("p")))), P("q"))));

    //    Формула на входе
    cout << "Formula: " << endl;
    cout << initFormula.at(0) << endl;
//

    Automaton automatic = createAutomatic(initFormula.at(0), initFormula);
    cout << "Automatic Buchi: " << endl;
    cout << automatic << endl;
    return 0;
}