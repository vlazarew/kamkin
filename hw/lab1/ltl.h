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

#pragma once

#include <iostream>

namespace model {
    namespace ltl {

        class Formula final {
        public:
            enum Kind {
                ATOM, // Atomic proposition: p
                NOT,  // Negation: ~A
                AND,  // Conjunction: (A1 & A2)
                OR,   // Disjunction: (A1 | A2)
                IMPL, // Implication: (A1 -> A2)
                X,    // NeXt time: X{A}
                G,    // Globally: G{A}
                F,    // In the Future: F{A}
                U,    // Until: (A1 U A2)
                R     // Release: (A1 R A2)
            };

            Formula operator !() const;
            Formula operator &&(const Formula &rhs) const;
            Formula operator ||(const Formula &rhs) const;
            Formula operator >>(const Formula &rhs) const;

            friend Formula P(const std::string &prop);
            friend Formula X(const Formula &arg);
            friend Formula G(const Formula &arg);
            friend Formula F(const Formula &arg);
            friend Formula U(const Formula &lhs, const Formula &rhs);
            friend Formula R(const Formula &lhs, const Formula &rhs);

            Kind kind() const { return _kind; }
            std::string prop() const { return _prop; }

            const Formula& arg() const { return *_lhs; }
            const Formula& lhs() const { return *_lhs; }
            const Formula& rhs() const { return *_rhs; }

        private:
            Formula(Kind kind, const std::string &prop, const Formula *lhs, const Formula *rhs):
                    _kind(kind), _prop(prop), _lhs(lhs), _rhs(rhs) {}

            Formula(const std::string &prop):
                    Formula(ATOM, prop, 0, 0) {}

            Formula(Kind kind, const Formula *arg):
                    Formula(kind, "", arg, 0) {}

            Formula(Kind kind, const Formula *lhs, const Formula *rhs):
                    Formula(kind, "", lhs, rhs) {}

            const Kind _kind;
            const std::string _prop;
            const Formula *_lhs;
            const Formula *_rhs;
        };

        inline Formula Formula::operator !() const {
            return Formula(NOT, this);
        }

        inline Formula Formula::operator &&(const Formula &rhs) const {
            return Formula(AND, this, &rhs);
        }

        inline Formula Formula::operator ||(const Formula &rhs) const {
            return Formula(OR, this, &rhs);
        }

        inline Formula Formula::operator >>(const Formula &rhs) const {
            return Formula(IMPL, this, &rhs);
        }

        inline Formula P(const std::string &prop) {
            return Formula(prop);
        }

        inline Formula X(const Formula &arg) {
            return Formula(Formula::X, &arg);
        }

        inline Formula G(const Formula &arg) {
            return Formula(Formula::G, &arg);
        }

        inline Formula F(const Formula &arg) {
            return Formula(Formula::F, &arg);
        }

        inline Formula U(const Formula &lhs, const Formula &rhs) {
            return Formula(Formula::U, &lhs, &rhs);
        }

        inline Formula R(const Formula &lhs, const Formula &rhs) {
            return Formula(Formula::R, &lhs, &rhs);
        }

        std::ostream& operator <<(std::ostream &out, const Formula &formula) {
            switch (formula.kind()) {
                case Formula::ATOM:
                    return out << formula.prop();
                case Formula::NOT:
                    return out << "!(" << formula.arg() << ")";
                case Formula::AND:
                    return out << "(" << formula.lhs() << ") && (" << formula.rhs() << ")";
                case Formula::OR:
                    return out << "(" << formula.lhs() << ") || (" << formula.rhs() << ")";
                case Formula::IMPL:
                    return out << "(" << formula.lhs() << ") -> (" << formula.rhs() << ")";
                case Formula::X:
                    return out << "X(" << formula.arg() << ")";
                case Formula::G:
                    return out << "G(" << formula.arg() << ")";
                case Formula::F:
                    return out << "F(" << formula.arg() << ")";
                case Formula::U:
                    return out << "(" << formula.lhs() << ") U (" << formula.rhs() << ")";
                case Formula::R:
                    return out << "(" << formula.lhs() << ") R (" << formula.rhs() << ")";
            }

            return out;
        }

    }} // namespace model::ltl

