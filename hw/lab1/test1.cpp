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


string ftostr(const Formula &formula) {
    switch (formula.kind()) {
        case Formula::ATOM:
            return "(ATOM" + formula.prop() + ")";
        case Formula::NOT:
            return "NOT(" + ftostr(formula.lhs()) + ")";
        case Formula::AND:
            return "(" + ftostr(formula.lhs()) + "AND" + ftostr(formula.rhs()) + ")";
        case Formula::OR:
            return "(" + ftostr(formula.lhs()) + "OR" + ftostr(formula.rhs()) + ")";
        case Formula::IMPL:
            return "(" + ftostr(formula.lhs()) + "IMPL" + ftostr(formula.rhs()) + ")";
        case Formula::X:
            return "X(" + ftostr(formula.lhs()) + ")";
        case Formula::G:
            return "G(" + ftostr(formula.lhs()) + ")";
        case Formula::F:
            return "F(" + ftostr(formula.lhs()) + ")";
        case Formula::U:
            return "(" + ftostr(formula.lhs()) + "U" + ftostr(formula.rhs()) + ")";
        case Formula::R:
            return "(" + ftostr(formula.lhs()) + "R" + ftostr(formula.rhs()) + ")";
    };
    return "";
}

template<>
struct std::hash<Formula> {
    size_t operator()(Formula const &key) const {
        hash<string> hasher;
        return hasher(ftostr(key));
    }
};

ostream &operator<<(ostream &out, vector<Formula> &formula_vector) {
    for (int i = 0; i < formula_vector.size() - 1; i++) {
        out << formula_vector[i] << ", ";
    }
    out << formula_vector[formula_vector.size() - 1];
    return out;
}

ostream &operator<<(ostream &out, unordered_set<Formula> &formula_vector) {
    for (auto it = formula_vector.begin(); it != formula_vector.end(); it++) {
        out << *it << ", ";
    }
    return out;
}

ostream &operator<<(ostream &out, vector<unordered_set<Formula>> &S) {
    for (int i = 0; i < S.size(); i++) {
        out << "[ " << S[i] << " ]" << endl;
    }
    return out;
}

void build_enclosure(Formula formula, unordered_set<Formula> &closure) {
    closure.insert(formula);
    if (formula.kind() == Formula::ATOM) {
        return;
    } else if (formula.kind() == Formula::X) {
        build_enclosure(formula.arg(), closure);
    } else if (formula.kind() == Formula::F) {
        build_enclosure(formula.lhs(), closure);
    } else if (formula.kind() == Formula::G) {
        build_enclosure(formula.lhs(), closure);
    } else if (formula.kind() == Formula::NOT) {
        build_enclosure(formula.lhs(), closure);
    } else {
        build_enclosure(formula.lhs(), closure);
        build_enclosure(formula.rhs(), closure);
    }
}

unordered_set<Formula> get_elementary_sets(unordered_set<Formula> &closure, bool &has_true) {
    unordered_set<Formula> sets;
    for (auto it = closure.begin(); it != closure.end(); it++) {
        if (it->kind() == Formula::ATOM && it->prop() == "1") {
            has_true = true;
        } else if (it->kind() == Formula::X || it->kind() == Formula::ATOM) {
            sets.insert(*it);
        }
    }
    return sets;
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

vector<unordered_set<Formula>>
recombinate_elementary_sets(unordered_set<Formula> elementary_sets, unordered_set<Formula> &closure,
                            bool has_true) {
    int vectorSize = pow(2, elementary_sets.size());
    vector<unordered_set<Formula>> S;
    for (int i = 0; i < vectorSize; i++) {
        unordered_set<Formula> true_set;
        auto k = elementary_sets.begin();
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
        while (k != elementary_sets.end()) {
            true_set.insert(!(*k));
            k++;
        }
        cout << "True set" << endl;
        cout << true_set << endl;
        S.push_back(get_s(true_set, closure));
        if (has_true) {
            S.at(i).insert(P("1"));
        }
    }
    return S;
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
        res.add_state("s" + to_string(i));
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

void simplify(vector<Formula> &stack) {
    Formula latestFormula = stack[stack.size() - 1];
    if (latestFormula.kind() == Formula::ATOM) {
        return;
    } else if (latestFormula.kind() == Formula::X) {
        if (latestFormula.lhs().kind() == Formula::OR) {
            stack.push_back(X(latestFormula.lhs().lhs()));
            simplify(stack);
            size_t tmp = stack.size() - 1;
            stack.push_back(X(latestFormula.lhs().rhs()));
            simplify(stack);
            stack.push_back(stack[tmp] || stack[stack.size() - 1]);
            stack.push_back(X(stack[stack.size() - 1]));
        } else if (latestFormula.lhs().kind() == Formula::NOT) {
            stack.push_back(latestFormula.lhs().lhs());
            simplify(stack);
            stack.push_back(X(stack[stack.size() - 1]));
            stack.push_back(!(stack[stack.size() - 1]));
        } else {
            stack.push_back(latestFormula.lhs());
            simplify(stack);
            stack.push_back(X(stack[stack.size() - 1]));
        }
    } else if (latestFormula.kind() == Formula::F) {
        stack.push_back(P("1"));
        simplify(stack);
        stack.push_back(latestFormula.lhs());
        simplify(stack);
        stack.push_back(U(stack[stack.size() - 2], stack[stack.size() - 1]));
    } else if (latestFormula.kind() == Formula::G) {
        stack.push_back(latestFormula.lhs());
        simplify(stack);
        stack.push_back(P("1"));
        stack.push_back(!(stack[stack.size() - 2]));
        stack.push_back(U(stack[stack.size() - 2], stack[stack.size() - 1]));
        stack.push_back(!(stack[stack.size() - 1]));
    } else if (latestFormula.kind() == Formula::NOT) {
        stack.push_back(latestFormula.lhs());
        simplify(stack);
        stack.push_back(!(stack[stack.size() - 1]));
    } else if (latestFormula.kind() == Formula::AND) {
        stack.push_back(latestFormula.lhs());
        simplify(stack);
        Formula tmp = stack[stack.size() - 1];
        stack.push_back(latestFormula.rhs());
        simplify(stack);
        stack.push_back(tmp && stack[stack.size() - 1]);
    } else if (latestFormula.kind() == Formula::OR) {
        stack.push_back(latestFormula.lhs());
        simplify(stack);
        Formula tmp = stack[stack.size() - 1];
        stack.push_back(latestFormula.rhs());
        simplify(stack);
        stack.push_back(tmp || stack[stack.size() - 1]);
    } else if (latestFormula.kind() == Formula::IMPL) {
        stack.push_back(latestFormula.lhs());
        simplify(stack);
        size_t tmp = stack.size() - 1;
        stack.push_back(latestFormula.rhs());
        simplify(stack);
        stack.push_back(!(stack[tmp]));
        stack.push_back(stack[stack.size() - 1] || stack[stack.size() - 2]);
    }
}

Automaton formula_to_automaton(Formula &formula) {
    cout << "Formula: " << endl;
    cout << formula << endl;
    vector<Formula> simple;
    simple.push_back(formula);
    simplify(simple);
    Formula simple_formula = simple[simple.size() - 1];
    cout << "Simplified formula: " << endl << simple_formula << endl;

    Automaton res;
    bool has_true = false;
    unordered_set<Formula> closure;

    build_enclosure(simple_formula, closure);
    cout << "Closure: " << endl;
    cout << closure << endl << endl;

    unordered_set<Formula> elementary_sets = get_elementary_sets(closure, has_true);
    cout << "Elementary sets: " << endl;
    cout << elementary_sets << endl << endl;

    vector<unordered_set<Formula>> S = recombinate_elementary_sets(elementary_sets, closure, has_true);
    cout << "Recombination: " << endl;
    cout << S << endl;

    unordered_set<Formula> untils = get_until(closure);
    cout << "Untils:" << endl << untils << endl;
    add_until(untils, S);
    cout << "With until:" << endl << S << endl;
    add_states(res, simple_formula, untils, S);
    add_transitions(res, S, untils, closure);

    cout << "Automaton: " << endl;
    cout << res << endl;

    return res;
}

int main() {
    const string p = "p";
    const string q = "q";
//    Formula formula = U(P("p") || P("q"), (P("p") && P("q")));
    Formula formula = G(P("p") >> X(P("q")));
//    Formula formula = X(!(P("q")));
//    cout << formula << endl;

    Automaton automaton = formula_to_automaton(formula);
    return 0;
}
