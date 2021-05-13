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
#include<cstring>
#include<unordered_set>
#include<functional>
#include "ltl.h"
#include "fsm.h"

using namespace model::ltl;
using namespace model::fsm;
using namespace std;


// Получаем хеш для нашего объекта формула для добавления в сеты
template<>
struct std::hash<Formula> {
    int operator()(Formula const &formula) const {
        hash<string> hashFunc;
        return hashFunc(formula.toString());
    }
};

Formula getSimplifiedFormula(const Formula &formula);

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
void createEnclosure(const Formula &formula, unordered_set<Formula> &closure) {
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

unordered_set<Formula> getAtoms(unordered_set<Formula> &closure, bool &hasTrue) {
    unordered_set<Formula> atoms;
    for (const auto &formula : closure) {
        if (formula.kind() == Formula::ATOM && formula.prop() == "1") {
            hasTrue = true;
        } else if (formula.kind() == Formula::X || formula.kind() == Formula::ATOM) {
            atoms.insert(formula);
        }
    }
    return atoms;
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

unordered_set<Formula> getS(unordered_set<Formula> trueSet, unordered_set<Formula> &closure) {
    unordered_set<Formula> res;
    for (const auto &it : closure) {
        if (isTrue(it, trueSet)) {
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

vector<unordered_set<Formula>> makeAtomCombinations(unordered_set<Formula> atoms, unordered_set<Formula> &closure,
                                                    bool hasTrue) {
    int vectorCombinationsSize = pow(2, atoms.size());
    vector<unordered_set<Formula>> combinations;
    for (int i = 0; i < vectorCombinationsSize; i++) {
        unordered_set<Formula> trueSet = getTrueSet(atoms, i);

        cout << "True set" << endl;
        cout << trueSet << endl;

        combinations.push_back(getS(trueSet, closure));
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

void addStates(Automaton &res, Formula &formula, unordered_set<Formula> &untils,
               vector<unordered_set<Formula>> &states) {
    for (int i = 0; i < states.size(); i++) {
        bool init = false;
        bool final = false;

        if (states.at(i).find(formula) != states.at(i).end()) {
            init = true;
        }

        for (const auto &until : untils) {
            Formula psi = until.rhs();
            if ((states.at(i).find(until) == states.at(i).end()) || ((states.at(i).find(psi) != states.at(i).end()))) {
                final = true;
            }
        }
        res.add_state("s" + to_string(i), init, final);
    }
}

void addTransition(Automaton &res, int index, vector<unordered_set<Formula>> &S,
                   unordered_set<Formula> &untils, unordered_set<Formula> &closure) {
    unordered_set<Formula> XShould;
    unordered_set<Formula> XShouldnt;
    unordered_set<Formula> UShould;
    unordered_set<Formula> UShouldnt;

    unordered_set<Formula> Xes = getX(closure);
    set<string> atoms = {};

    for (const auto &it : S.at(index)) {
        if (it.kind() == Formula::ATOM) {
            atoms.insert(it.prop());
        }
    }

    for (const auto &Xe : Xes) {
        if (S.at(index).find(Xe) != S.at(index).end()) {
            XShould.insert(Xe.lhs());
        } else {
            XShouldnt.insert(Xe.lhs());
        }
    }

    for (const auto &until : untils) {
        Formula psi = until.rhs();
        Formula hi = until.lhs();
        if (S.at(index).find(until) != S.at(index).end()) {
            if (S.at(index).find(psi) == S.at(index).end()) {
                UShould.insert(until);
            }
        } else {
            if (S.at(index).find(hi) != S.at(index).end()) {
                UShouldnt.insert(until);
            }
        }
    }

    for (int i = 0; i < S.size(); i++) {
        bool isTransition = true;
        for (const auto &it : XShould) {
            if (S.at(i).find(it) == S.at(i).end()) {
                isTransition = false;
                break;
            }
        }
        if (isTransition) {
            for (const auto &it : XShouldnt) {
                if (S.at(i).find(it) != S.at(i).end()) {
                    isTransition = false;
                    break;
                }
            }
        }
        if (isTransition) {
            for (const auto &it : UShould) {
                if (S.at(i).find(it) == S.at(i).end()) {
                    isTransition = false;
                    break;
                }
            }
        }
        if (isTransition) {
            for (const auto &it : UShouldnt) {
                if (S.at(i).find(it) != S.at(i).end()) {
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

void addTransitions(Automaton &res, vector<unordered_set<Formula>> &S, unordered_set<Formula> &untils,
                    unordered_set<Formula> &closure) {
    for (int i = 0; i < S.size(); i++) {
        addTransition(res, i, S, untils, closure);
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
void simplifyKindX(vector<Formula> &formulaStack, const Formula &latestFormula) {
    if (latestFormula.lhs().kind() == Formula::OR) {
        simplifyKindX_Or(formulaStack, latestFormula);
    } else if (latestFormula.lhs().kind() == Formula::NOT) {
        simplifyKindX_Not(formulaStack, latestFormula);
    } else {
        simplifyLindX_Other(formulaStack, latestFormula);
    }
}

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

///// КОНЕЦ: Упрощение NEXT + вспомогаельные методы

/////// КОНЕЦ: УПРОЩЕНИЕ

Automaton createAutomatic(Formula &formula) {
//  Упрощение исходной формулы
    Formula simplifiedFormula = getSimplifiedFormula(formula);
    cout << "Simplified formula: " << endl << simplifiedFormula << endl;
//

//    Генерируем замыкание
    unordered_set<Formula> closure;
    createEnclosure(simplifiedFormula, closure);
    cout << "Closure: " << endl << closure << endl << endl;
//

//  Извлекаем атомы
    bool hasTrue = false;
    unordered_set<Formula> atoms = getAtoms(closure, hasTrue);
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
//

    return automatic;
}

Formula getSimplifiedFormula(const Formula &formula) {
    vector<Formula> simplifiedFormulas;
    simplifiedFormulas.push_back(formula);
    simplify(simplifiedFormulas);

    Formula simplifiedFormula = simplifiedFormulas.at(simplifiedFormulas.size() - 1);
    return simplifiedFormula;
}

int main() {
//    Formula formula = U(P("p") || P("q"), (P("p") && P("q")));
//    Formula formula = G(P("p") >> X(P("q")));
    Formula formula = X(!(P("q")));

//    Формула на входе
    cout << "Formula: " << endl;
    cout << formula << endl;
//

    Automaton automatic = createAutomatic(formula);
    cout << "Automatic Buchi: " << endl;
    cout << automatic << endl;
    return 0;
}
