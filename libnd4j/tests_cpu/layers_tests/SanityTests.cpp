/* ******************************************************************************
 *
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License, Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0.
 *
 *  See the NOTICE file distributed with this work for additional
 *  information regarding copyright ownership.
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 * SPDX-License-Identifier: Apache-2.0
 ******************************************************************************/

//
// Created by raver119 on 13/11/17.
//
#include <graph/Graph.h>
#include <graph/Node.h>
#include <ops/declarable/CustomOperations.h>

#include "testlayers.h"

using namespace sd;
using namespace sd::graph;

class SanityTests : public NDArrayTests {
 public:
};

TEST_F(SanityTests, VariableSpace_1) {
  VariableSpace variableSpace;
  variableSpace.putVariable(1, new Variable());
  variableSpace.putVariable(1, 1, new Variable());

  std::pair<int, int> pair(1, 2);
  variableSpace.putVariable(pair, new Variable());
}

TEST_F(SanityTests, VariableSpace_2) {
  VariableSpace variableSpace;
  variableSpace.putVariable(1, new Variable(NDArrayFactory::create_<float>('c', {3, 3})));
  variableSpace.putVariable(1, 1, new Variable(NDArrayFactory::create_<float>('c', {3, 3})));

  std::pair<int, int> pair(1, 2);
  variableSpace.putVariable(pair, new Variable(NDArrayFactory::create_<float>('c', {3, 3})));
}

TEST_F(SanityTests, Graph_1) {
  Graph graph;

  graph.getVariableSpace()->putVariable(1, new Variable(NDArrayFactory::create_<float>('c', {3, 3})));
  graph.getVariableSpace()->putVariable(1, 1, new Variable(NDArrayFactory::create_<float>('c', {3, 3})));

  std::pair<int, int> pair(1, 2);
  graph.getVariableSpace()->putVariable(pair, new Variable(NDArrayFactory::create_<float>('c', {3, 3})));
}
