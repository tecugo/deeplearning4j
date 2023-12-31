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
//  @author raver119@gmail.com
//  @author Yurii Shyrma

#include <system/op_boilerplate.h>
#if NOT_EXCLUDED(OP_conv1d)

#include <ops/declarable/CustomOperations.h>
#include <ops/declarable/DeclarableOp.h>
#include <ops/declarable/helpers/convolutions.h>

namespace sd {
namespace ops {

CUSTOM_OP_IMPL(conv1d, 2, 1, false, 0, 5) {
  auto input = INPUT_VARIABLE(0);                               // [bS, iW, iC] (NWC) or [bS, iC, iW] (NCW)
  auto weights = INPUT_VARIABLE(1);                             // [kW, iC, oC], [oC, iC, kW], [oC, kW, iC]
  auto bias = block.width() > 2 ? INPUT_VARIABLE(2) : nullptr;  // [oC]

  auto output = OUTPUT_NULLIFIED(0);  // [bS, oW, oC] (NWC) or [bS, oC, oW] (NCW)

  int kW = INT_ARG(0) > 0 ? INT_ARG(0) : static_cast<int>(weights->sizeAt(0));  // filter(kernel) width
  int sW = INT_ARG(1);                                                          // strides width
  int pW = INT_ARG(2);                                                          // paddings width
  int dW = INT_ARG(3);                                                          // dilations width
  int paddingMode = INT_ARG(4);                                                 // 0-VALID, 1-SAME, 2-CAUSAL
  int isNCW = block.getIArguments()->size() > 5 ? !INT_ARG(5) : 1;              // INT_ARG(4): 0-NCW,  1-NWC
  int wFormat =
      block.getIArguments()->size() > 6 ? INT_ARG(6) : 0;  // 0 - [kW, iC, oC], 1 - [oC, iC, kW], 2 - [oC, kW, iC]

  const int rank = 3;
  REQUIRE_TRUE(input->rankOf() == rank, 0,
               "CUSTOM CONV1D OP: rank of input array must be equal to %i, but got %i instead !", rank,
               input->rankOf());
  REQUIRE_TRUE(weights->rankOf() == rank, 0,
               "CUSTOM CONV1D OP: rank of weights array must be equal to %i, but got %i instead !", rank,
               weights->rankOf());

  int indIOioC, indIiW, indWoC(0 == wFormat ? 2 : 0);
  if (!isNCW) {
    indIOioC = 2;
    indIiW = 1;
  } else {
    indIOioC = 1;
    indIiW = 2;
  }

  int bS = input->sizeAt(0);         // batch size
  int iW = input->sizeAt(indIiW);    // input width
  int iC = input->sizeAt(indIOioC);  // input channels
  int oC = weights->sizeAt(indWoC);  // output channels
  std::vector<sd::LongType> expectedWeightsShape =
      0 == wFormat ? std::vector<sd::LongType>({kW, iC, oC})
                   : (1 == wFormat ? std::vector<sd::LongType>({oC, iC, kW}) : std::vector<sd::LongType>({oC, kW, iC}));

  REQUIRE_TRUE(weights->isSameShape(expectedWeightsShape), 0,
               "CUSTOM CONV1D OP: wrong shape of weights array, expected is %s, but got %s instead !",
               ShapeUtils::shapeAsString(expectedWeightsShape).c_str(), ShapeUtils::shapeAsString(weights).c_str());
  if (bias)
    REQUIRE_TRUE(
        bias->rankOf() <= 2 && oC == bias->lengthOf(), 0,
        "CUSTOM CONV1D OP: wrong shape of array with biases, expected rank, length: <=2, %i, but got %i, %i instead !",
        oC, bias->rankOf(), bias->lengthOf());

  std::vector<sd::LongType> reshapeForInput, reshapeForOutput;
  if (!isNCW) {
    reshapeForInput = {input->sizeAt(0), 1, input->sizeAt(1), input->sizeAt(2)};      // [bS, iW, iC] -> [bS, 1, iW, iC]
    reshapeForOutput = {output->sizeAt(0), 1, output->sizeAt(1), output->sizeAt(2)};  // [bS, oW, oC] -> [bS, 1, oW, oC]
  } else {
    reshapeForInput = {input->sizeAt(0), input->sizeAt(1), 1, input->sizeAt(2)};      // [bS, iC, iW] -> [bS, iC, 1, iW]
    reshapeForOutput = {output->sizeAt(0), output->sizeAt(1), 1, output->sizeAt(2)};  // [bS, oC, oW] -> [bS, oC, 1, oW]
  }

  auto inputReshaped = input->reshape(input->ordering(), reshapeForInput);
  auto outputReshaped = output->reshape(output->ordering(), reshapeForOutput, false);
  auto weightsReshaped = weights->reshape(
      weights->ordering(),
      {1, weights->sizeAt(0), weights->sizeAt(1), weights->sizeAt(2)});  // [kW, iC, oC] -> [1, kW, iC, oC]

  sd::ops::conv2d conv2d;
  const sd::Status status = conv2d.execute({&inputReshaped, &weightsReshaped, bias}, {&outputReshaped}, {},
                                           {1, kW, 1, sW, 0, pW, 1, dW, paddingMode, !isNCW, wFormat}, {});
  if (status != sd::Status::OK) return status;


  return sd::Status::OK;
}

DECLARE_SHAPE_FN(conv1d) {
  auto inputShapeInfo = inputShape->at(0);
  auto weightsShapeInfo = inputShape->at(1);
  sd::LongType const* biasShapeInfo = block.width() > 2 ? inputShape->at(2) : nullptr;

  LongType kW = INT_ARG(0) > 0 ? INT_ARG(0) : static_cast<LongType>(shape::sizeAt(weightsShapeInfo, static_cast<sd::LongType>(0)));  // filter(kernel) width
  LongType sW = INT_ARG(1);                                                                          // strides width
  LongType pW = INT_ARG(2);                                                                          // paddings width
  LongType dW = INT_ARG(3);                                                                          // dilations width
  int paddingMode = INT_ARG(4);                                                                 // 0-VALID, 1-SAME
  int isNCW = block.getIArguments()->size() > 5 ? !INT_ARG(5) : 1;  // INT_ARG(4): 1-NWC, 0-NCW
  int wFormat =
      block.getIArguments()->size() > 6 ? INT_ARG(6) : 0;  // 0 - [kW, iC, oC], 1 - [oC, iC, kW], 2 - [oC, kW, iC]

  int indIOioC, indIiW, indWoC(0 == wFormat ? 2 : 0);
  if (!isNCW) {
    indIOioC = 2;
    indIiW = 1;
  } else {
    indIOioC = 1;
    indIiW = 2;
  }

  const int rank = 3;
  REQUIRE_TRUE(inputShapeInfo[0] == rank, 0,
               "CUSTOM CONV1D OP: rank of input array must be equal to %i, but got %i instead !", rank, inputShapeInfo);
  REQUIRE_TRUE(weightsShapeInfo[0] == rank, 0,
               "CUSTOM CONV1D OP: rank of weights array must be equal to %i, but got %i instead !", rank,
               weightsShapeInfo);

  LongType bS = inputShapeInfo[1];             // batch size
  LongType iW = inputShapeInfo[indIiW + 1];    // input width
  LongType iC = inputShapeInfo[indIOioC + 1];  // input channels
  LongType oC = weightsShapeInfo[indWoC + 1];  // output channels

  std::vector<sd::LongType> expectedWeightsShape =
      0 == wFormat ? std::vector<sd::LongType>({kW, iC, oC})
                   : (1 == wFormat ? std::vector<sd::LongType>({oC, iC, kW}) : std::vector<sd::LongType>({oC, kW, iC}));

  if (biasShapeInfo)
    REQUIRE_TRUE(
        biasShapeInfo[0] <= 2 && oC == shape::length(biasShapeInfo), 0,
        "CUSTOM CONV1D OP: wrong shape of array with biases, expected rank, length: <=2, %i, but got %i, %i instead !",
        oC, biasShapeInfo[0], shape::length(biasShapeInfo));

  LongType oH, oW;  // output height, width
  ConvolutionUtils::calcOutSizePool2D(oH, oW, 1, kW, 1, sW, 0, pW, 1, dW, 1, iW, paddingMode);

  sd::LongType* outputShapeInfo = nullptr;
  ALLOCATE(outputShapeInfo, block.getWorkspace(), shape::shapeInfoLength(rank), sd::LongType);

  outputShapeInfo[0] = 3;
  outputShapeInfo[1] = bS;

  if (isNCW) {
    outputShapeInfo[2] = oC;
    outputShapeInfo[3] = oW;
  } else {
    outputShapeInfo[2] = oW;
    outputShapeInfo[3] = oC;
  }

  ShapeUtils::updateStridesAndType(outputShapeInfo, weightsShapeInfo, shape::order(weightsShapeInfo));
  return SHAPELIST(CONSTANT(outputShapeInfo));
}

DECLARE_TYPES(conv1d) {
  getOpDescriptor()
      ->setAllowedInputTypes(0, {ALL_FLOATS, ALL_INTS, DataType::QINT8, DataType::QINT16})
      ->setAllowedInputTypes(1, {ALL_FLOATS})
      ->setAllowedInputTypes(2, {ALL_FLOATS})
      ->setAllowedOutputTypes(0, {ALL_FLOATS});
}

//////////////////////////////////////////////////////////////////////////
CUSTOM_OP_IMPL(conv1d_bp, 3, 2, false, 0, 5) {
  auto input = INPUT_VARIABLE(0);                               // [bS, iW, iC] (NWC) or [bS, iC, iW] (NCW)
  auto weights = INPUT_VARIABLE(1);                             // [kW, iC, oC], [oC, iC, kW], [oC, kW, iC]
  auto bias = block.width() > 3 ? INPUT_VARIABLE(2) : nullptr;  // [oC]
  auto gradO = block.width() > 3 ? INPUT_VARIABLE(3)
                                 : INPUT_VARIABLE(2);  // [bS, oW, oC] (NWC) or [bS, oC, oW] (NCW), epsilon_next

  auto gradI = OUTPUT_NULLIFIED(0);                                // [bS, iW, iC] (NWC) or [bS, iC, iW] (NCW), epsilon
  auto gradW = OUTPUT_NULLIFIED(1);                                // [kW, iC, oC], [oC, iC, kW], [oC, kW, iC]
  auto gradB = block.width() > 3 ? OUTPUT_NULLIFIED(2) : nullptr;  // [oC]

  LongType kW = INT_ARG(0) > 0 ? INT_ARG(0) : static_cast<LongType>(weights->sizeAt(0));  // filter(kernel) width
  LongType sW = INT_ARG(1);                                                          // strides width
  LongType pW = INT_ARG(2);                                                          // paddings width
  LongType dW = INT_ARG(3);                                                          // dilations width
  int paddingMode = INT_ARG(4);                                                 // 0-VALID, 1-SAME, 2-CAUSAL
  int isNCW = block.getIArguments()->size() > 5 ? !INT_ARG(5) : 1;              // INT_ARG(4): 1-NWC, 0-NCW
  int wFormat =
      block.getIArguments()->size() > 6 ? INT_ARG(6) : 0;  // 0 - [kW, iC, oC], 1 - [oC, iC, kW], 2 - [oC, kW, iC]

  const int rank = 3;
  REQUIRE_TRUE(input->rankOf() == rank, 0,
               "CUSTOM CONV1D_BP OP: rank of input array must be equal to %i, but got %i instead !", rank,
               input->rankOf());
  REQUIRE_TRUE(weights->rankOf() == rank, 0,
               "CUSTOM CONV1D_BP OP: rank of weights array must be equal to %i, but got %i instead !", rank,
               weights->rankOf());
  REQUIRE_TRUE(
      gradO->rankOf() == rank, 0,
      "CUSTOM CONV1D_BP OP: rank of output gradients (next epsilon) array must be equal to %i, but got %i instead !",
      rank, gradO->rankOf());

  int indIOioC, indIiW, indWoC(0 == wFormat ? 2 : 0);
  if (!isNCW) {
    indIOioC = 2;
    indIiW = 1;
  } else {
    indIOioC = 1;
    indIiW = 2;
  }

  const LongType bS = input->sizeAt(0);         // batch size
  const LongType iW = input->sizeAt(indIiW);    // input width
  const LongType iC = input->sizeAt(indIOioC);  // input channels
  const LongType oC = weights->sizeAt(indWoC);  // output channels

  LongType trueoH, trueoW;  // true output height, width
  ConvolutionUtils::calcOutSizePool2D(trueoH, trueoW, 1, kW, 1, sW, 0, pW, 1, dW, 1, iW, paddingMode);

  std::vector<sd::LongType> expectedGradOShape =
      ShapeUtils::composeShapeUsingDimsAndIdx({bS, oC, trueoW, 0, indIOioC, indIiW});
  std::vector<sd::LongType> expectedWeightsShape =
      0 == wFormat ? std::vector<sd::LongType>({kW, iC, oC})
                   : (1 == wFormat ? std::vector<sd::LongType>({oC, iC, kW}) : std::vector<sd::LongType>({oC, kW, iC}));
  REQUIRE_TRUE(
      gradO->isSameShape(expectedGradOShape), 0,
      "CUSTOM CONV1D_BP OP: wrong shape of output gradients (next epsilon) array, expected is %s, but got %s instead !",
      ShapeUtils::shapeAsString(expectedGradOShape).c_str(), ShapeUtils::shapeAsString(gradO).c_str());
  REQUIRE_TRUE(weights->isSameShape(expectedWeightsShape), 0,
               "CUSTOM CONV1D_BP OP: wrong shape of weights array, expected is %s, but got %s instead !",
               ShapeUtils::shapeAsString(expectedWeightsShape).c_str(), ShapeUtils::shapeAsString(weights).c_str());
  if (bias)
    REQUIRE_TRUE(bias->rankOf() <= 2 && oC == bias->lengthOf(), 0,
                 "CUSTOM CONV1D_BP OP: wrong shape of array with biases, expected rank, length: <=2, %i, but got %i, "
                 "%i instead !",
                 oC, bias->rankOf(), bias->lengthOf());

  std::vector<sd::LongType> reshapeForInput, reshapeForGradO;
  if (!isNCW) {
    reshapeForInput = {input->sizeAt(0), 1, input->sizeAt(1), input->sizeAt(2)};  // [bS, iW, iC] -> [bS, 1, iW, iC]
    reshapeForGradO = {gradO->sizeAt(0), 1, gradO->sizeAt(1), gradO->sizeAt(2)};  // [bS, oW, oC] -> [bS, 1, oW, oC]
  } else {
    reshapeForInput = {input->sizeAt(0), input->sizeAt(1), 1, input->sizeAt(2)};  // [bS, iC, iW] -> [bS, iC, 1, iW]
    reshapeForGradO = {gradO->sizeAt(0), gradO->sizeAt(1), 1, gradO->sizeAt(2)};  // [bS, oC, oW] -> [bS, oC, 1, oW]
  }

  auto inputReshaped = input->reshape(input->ordering(), reshapeForInput);
  auto gradIReshaped = gradI->reshape(gradI->ordering(), reshapeForInput, false);
  auto gradOReshaped = gradO->reshape(gradO->ordering(), reshapeForGradO);
  auto weightsReshaped = weights->reshape(
      weights->ordering(),
      {1, weights->sizeAt(0), weights->sizeAt(1), weights->sizeAt(2)});  // [kW, iC, oC] -> [1, kW, iC, oC]
  auto gradWReshaped =
      gradW->reshape(gradW->ordering(), {1, weights->sizeAt(0), weights->sizeAt(1), weights->sizeAt(2)},
                     false);  // [kW, iC, oC] -> [1, kW, iC, oC]

  sd::ops::conv2d_bp conv2dBP;
  auto status = conv2dBP.execute({&inputReshaped, &weightsReshaped, bias, &gradOReshaped},
                                 {&gradIReshaped, &gradWReshaped, gradB}, {},
                                 {1, kW, 1, sW, 0, pW, 1, dW, paddingMode, !isNCW, wFormat}, {});
  if (status != sd::Status::OK) return status;

  // ConvolutionUtils::conv2dBP(block, &inputReshaped, &weightsReshaped, bias, &gradOReshaped, &gradIReshaped,
  // &gradWReshaped, gradB, 1,kW,  1,sW,  0,pW,  1,dW,  paddingMode, isNCW, wFormat);

  return sd::Status::OK;
}

DECLARE_SHAPE_FN(conv1d_bp) {
  auto inputShapeInfo = inputShape->at(0);    // [bS, iW, iC] (NWC) or [bS, iC, iW] (NCW)
  auto weightsShapeInfo = inputShape->at(1);  // [kW, iC, oC], [oC, iC, kW], [oC, kW, iC]
  sd::LongType const* biasShapeInfo = block.width() > 3 ? inputShape->at(2) : nullptr;  // [oC]
  sd::LongType const* gradOShapeInfo =
      block.width() > 3 ? inputShape->at(3)
                        : inputShape->at(2);  // [bS, oW, oC] (NWC) or [bS, oC, oW] (NCW), epsilon_next

  const int rank = 3;
  REQUIRE_TRUE(inputShapeInfo[0] == rank, 0,
               "CUSTOM CONV1D_BP OP: rank of input array must be equal to %i, but got %i instead !", rank,
               inputShapeInfo[0]);
  REQUIRE_TRUE(weightsShapeInfo[0] == rank, 0,
               "CUSTOM CONV1D_BP OP: rank of weights array must be equal to %i, but got %i instead !", rank,
               weightsShapeInfo[0]);
  REQUIRE_TRUE(
      gradOShapeInfo[0] == rank, 0,
      "CUSTOM CONV1D_BP OP: rank of output gradients (next epsilon) array must be equal to %i, but got %i instead !",
      rank, gradOShapeInfo[0]);

  LongType kW = INT_ARG(0) > 0 ? INT_ARG(0) : static_cast<sd::LongType>(shape::sizeAt(weightsShapeInfo, static_cast<sd::LongType>(0)));  // filter(kernel) width
  LongType sW = INT_ARG(1);                                                                          // strides width
  LongType pW = INT_ARG(2);                                                                          // paddings width
  LongType dW = INT_ARG(3);                                                                          // dilations width
  int paddingMode = INT_ARG(4);                                                                 // 0-VALID, 1-SAME
  int isNCW = block.getIArguments()->size() > 5 ? !INT_ARG(5) : 1;  // INT_ARG(4): 1-NWC, 0-NCW
  int wFormat =
      block.getIArguments()->size() > 6 ? INT_ARG(6) : 0;  // 0 - [kW, iC, oC], 1 - [oC, iC, kW], 2 - [oC, kW, iC]

  int indIOioC, indIiW, indWoC(0 == wFormat ? 2 : 0);
  if (!isNCW) {
    indIOioC = 2;
    indIiW = 1;
  } else {
    indIOioC = 1;
    indIiW = 2;
  }

  const LongType bS = inputShapeInfo[1];             // batch size
  const LongType iW = inputShapeInfo[indIiW + 1];    // input width
  const LongType iC = inputShapeInfo[indIOioC + 1];  // input channels
  const LongType oC = weightsShapeInfo[indWoC + 1];  // output channels

  LongType trueoH, trueoW;  // true output height, width
  ConvolutionUtils::calcOutSizePool2D(trueoH, trueoW, 1, kW, 1, sW, 0, pW, 1, dW, 1, iW, paddingMode);

  std::vector<sd::LongType> expectedGradOShape =
      ShapeUtils::composeShapeUsingDimsAndIdx({bS, oC, trueoW, 0, indIOioC, indIiW});
  std::vector<sd::LongType> expectedWeightsShape =
      0 == wFormat ? std::vector<sd::LongType>({kW, iC, oC})
                   : (1 == wFormat ? std::vector<sd::LongType>({oC, iC, kW}) : std::vector<sd::LongType>({oC, kW, iC}));
  REQUIRE_TRUE(
      ShapeUtils::areShapesEqual(gradOShapeInfo, expectedGradOShape), 0,
      "CUSTOM CONV1D_BP OP: wrong shape of output gradients (next epsilon) array, expected is %s, but got %s instead !",
      ShapeUtils::shapeAsString(expectedGradOShape).c_str(), ShapeUtils::shapeAsString(gradOShapeInfo).c_str());
  REQUIRE_TRUE(ShapeUtils::areShapesEqual(weightsShapeInfo, expectedWeightsShape), 0,
               "CUSTOM CONV1D_BP OP: wrong shape of weights array, expected is %s, but got %s instead !",
               ShapeUtils::shapeAsString(expectedWeightsShape).c_str(),
               ShapeUtils::shapeAsString(weightsShapeInfo).c_str());
  if (biasShapeInfo)
    REQUIRE_TRUE(biasShapeInfo[0] <= 2 && oC == shape::length(biasShapeInfo), 0,
                 "CUSTOM CONV1D_BP OP: wrong shape of array with biases, expected rank, length: <=2, %i, but got %i, "
                 "%i instead !",
                 oC, biasShapeInfo[0], shape::length(biasShapeInfo));

  auto gradIshapeInfo =
      ShapeBuilders::copyShapeInfoAndType(inputShapeInfo, gradOShapeInfo, false, block.getWorkspace());
  auto gradWshapeInfo =
      ShapeBuilders::copyShapeInfoAndType(weightsShapeInfo, gradOShapeInfo, false, block.getWorkspace());

  if (biasShapeInfo) {
    auto gradBshapeInfo =
        ShapeBuilders::copyShapeInfoAndType(biasShapeInfo, gradOShapeInfo, false, block.getWorkspace());
    return SHAPELIST(CONSTANT(gradIshapeInfo), CONSTANT(gradWshapeInfo), CONSTANT(gradBshapeInfo));
  }

  return SHAPELIST(CONSTANT(gradIshapeInfo), CONSTANT(gradWshapeInfo));
}

DECLARE_TYPES(conv1d_bp) {
  getOpDescriptor()
      ->setAllowedInputTypes(0, {ALL_FLOATS, ALL_INTS, DataType::QINT8, DataType::QINT16})
      ->setAllowedInputTypes(1, {ALL_FLOATS})
      ->setAllowedInputTypes(2, {ALL_FLOATS})
      ->setAllowedInputTypes(3, {ALL_FLOATS})
      ->setAllowedOutputTypes(0, {ALL_FLOATS})
      ->setAllowedOutputTypes(1, {ALL_FLOATS});
}

}  // namespace ops
}  // namespace sd

#endif
