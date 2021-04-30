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

unordered_set<Formula> getAtoms(unordered_set<Formula> &closure, bool &has_true) {
    unordered_set<Formula> atoms;
    for (const auto &formula : closure) {
        if (formula.kind() == Formula::ATOM && formula.prop() == "1") {
            has_true = true;
        } else if (formula.kind() == Formula::X || formula.kind() == Formula::ATOM) {
            atoms.insert(formula);
        }
    }
    return atoms;
}


bool is_true(Formula f, unordered_set<Formula> &true_set) {
    if (f.kind() == Formula::AND) {
        return is_true(f.lhs(), true_set) && is_true(f.rhs(), true_set);
    } else if (f.kind() == Formula::OR) {
        return is_true(f.lhs(), true_set) || is_true(f.rhs(), true_set);
    } else if (f.kind() == Formula::NOT) {
        return !is_true(f.lhs(), true_set);
    } else if (f.kind() == Formula::ATOM || f.kind() == Formula::X) {
        return true_set.find(f) != true_set.end();
    } else {
        return false;
    }
}

unordered_set<Formula> get_s(unordered_set<Formula> true_set, unordered_set<Formula> &closure) {
    unordered_set<Formula> res;
    for (auto it = closure.begin(); it != closure.end(); it++) {
        if (is_true(*it, true_set)) {
            res.insert(*it);
        }
    }
    return res;
}

unordered_set<Formula> get_until(unordered_set<Formula> &closure) {
    unordered_set<Formula> res;
    for (auto it = closure.begin(); it != closure.end(); it++) {
        if (it->kind() == Formula::U) {
            res.insert(*it);
        }
    }
    return res;
}

unordered_set<Formula> get_X(unordered_set<Formula> &closure) {
    unordered_set<Formula> res;
    for (auto it = closure.begin(); it != closure.end(); it++) {
        if (it->kind() == Formula::X) {
            res.insert(*it);
        }
    }
    return res;
}

vector<unordered_set<Formula>> makeAtomCombinations(unordered_set<Formula> atoms, unordered_set<Formula> &closure,
                                                    bool has_true) {
    int vectorCombinationsSize = pow(2, atoms.size());
    vector<unordered_set<Formula>> combinations;
    for (int i = 0; i < vectorCombinationsSize; i++) {
        unordered_set<Formula> true_set;
        auto k = atoms.begin();
        int num = i;
        while (num != 0) {
            if (num % 2 == 1) {
                true_set.insert(*k);
            } else {
                true_set.insert(!(*k));
            }
            num /= 2;
            k++;
        }
        while (k != atoms.end()) {
            true_set.insert(!(*k));
            k++;
        }
        cout << "True set" << endl;
        cout << true_set << endl;
        combinations.push_back(get_s(true_set, closure));
        if (has_true) {
            combinations.at(i).insert(P("1"));
        }
    }
    return combinations;
}

void add_until(unordered_set<Formula> &untils, vector<unordered_set<Formula>> &S) {
    vector<unordered_set<Formula>> to_add;
    for (auto until = untils.begin(); until != untils.end(); until++) {
        Formula hi = until->lhs();
        Formula psi = until->rhs();
        for (size_t j = 0; j < S.size(); j++) {
            if (S[j].find(psi) != S[j].end()) {
                S[j].insert(*until);
            } else if (S[j].find(hi) != S[j].end()) {
                to_add.push_back(S[j]);
                S[j].insert(*until);
            }
        }
    }

    S.insert(S.end(), to_add.begin(), to_add.end());
}

void add_states(Automaton &res, Formula &formula, unordered_set<Formula> &untils,
                vector<unordered_set<Formula>> &S) {
    for (int i = 0; i < S.size(); i++) {
        bool init = false;
        bool fin = false;
        if (S[i].find(formula) != S[i].end()) {
            init = true;
        }
        for (auto until = untils.begin(); until != untils.end(); until++) {
            Formula psi = until->rhs();
            if ((S[i].find(*until) == S[i].end()) || ((S[i].find(psi) != S[i].end()))) {
                fin = true;
            }
        }
        res.add_state("s" + to_string(i), init, fin);
    }
}

void add_transition(Automaton &res, int num, vector<unordered_set<Formula>> &S,
                    unordered_set<Formula> &untils, unordered_set<Formula> &closure) {
    unordered_set<Formula> X_should;
    unordered_set<Formula> X_shouldnt;
    unordered_set<Formula> U_should;
    unordered_set<Formula> U_shouldnt;

    unordered_set<Formula> Xes = get_X(closure);
    set<string> atoms = {};

    for (auto it = S[num].begin(); it != S[num].end(); it++) {
        if (it->kind() == Formula::ATOM) {
            atoms.insert(it->prop());
        }
    }

    for (auto next = Xes.begin(); next != Xes.end(); next++) {
        if (S[num].find(*next) != S[num].end()) {
            X_should.insert(next->lhs());
        } else {
            X_shouldnt.insert(next->lhs());
        }
    }

    for (auto until = untils.begin(); until != untils.end(); until++) {
        Formula psi = until->rhs();
        Formula hi = until->lhs();
        if (S[num].find(*until) != S[num].end()) {
            if (S[num].find(psi) == S[num].end()) {
                U_should.insert(*until);
            }
        } else {
            if (S[num].find(hi) != S[num].end()) {
                U_shouldnt.insert(*until);
            }
        }
    }

    for (size_t i = 0; i < S.size(); i++) {
        bool is_transition = true;
        for (auto it = X_should.begin(); it != X_should.end(); it++) {
            if (S[i].find(*it) == S[i].end()) {
                is_transition = false;
                break;
            }
        }
        if (is_transition)
            for (auto it = X_shouldnt.begin(); it != X_shouldnt.end(); it++) {
                if (S[i].find(*it) != S[i].end()) {
                    is_transition = false;
                    break;
                }
            }
        if (is_transition)
            for (auto it = U_should.begin(); it != U_should.end(); it++) {
                if (S[i].find(*it) == S[i].end()) {
                    is_transition = false;
                    break;
                }
            }
        if (is_transition)
            for (auto it = U_shouldnt.begin(); it != U_shouldnt.end(); it++) {
                if (S[i].find(*it) != S[i].end()) {
                    is_transition = false;
                    break;
                }
            }
        if (is_transition) {
            res.add_trans("s" + to_string(num), atoms, "s" + to_string(i));
        }
    }
}

void add_transitions(Automaton &res, vector<unordered_set<Formula>> &S, unordered_set<Formula> &untils,
                     unordered_set<Formula> &closure) {
    for (int i = 0; i < S.size(); i++) {
        add_transition(res, i, S, untils, closure);
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
    unordered_set<Formula> untilExpressions = get_until(closure);
    cout << "Until Expressions:" << endl << untilExpressions << endl;

    add_until(untilExpressions, atomsCombinations);
    cout << "With until:" << endl << atomsCombinations << endl;
//

//    Заполнение автомата состояниями + связками
    Automaton automatic;
    add_states(automatic, simplifiedFormula, untilExpressions, atomsCombinations);
    add_transitions(automatic, atomsCombinations, untilExpressions, closure);
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
    Formula formula = U(P("p") || P("q"), (P("p") && P("q")));
//    Formula formula = G(P("p") >> X(P("q")));
//    Formula formula = X(!(P("q")));

//    Формула на входе
    cout << "Formula: " << endl;
    cout << formula << endl;
//

    Automaton automatic = createAutomatic(formula);
    cout << "Automatic Buchi: " << endl;
    cout << automatic << endl;
    return 0;
}
