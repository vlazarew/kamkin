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

#include "bdd.h"

namespace model {
    namespace bdd {

        const Node Bdd::zero(-1, nullptr, nullptr);
        const Node Bdd::one(-1, nullptr, nullptr);

        Node Bdd::create(const Formula &formula) {
            if (formula.kind() == Formula::FALSE) {
                return Bdd::zero;
            }
            if (formula.kind() == Formula::TRUE) {
                return Bdd::one;
            }
            if (formula.kind() == Formula::VAR) {
                Node newNode = Node();
                newNode.var = formula.var();
                return newNode;
            }

//}         // FIXME:
            return Bdd::zero;
        }

    }
} // namespace model::bdd
