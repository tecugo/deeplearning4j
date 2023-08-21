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
// @author Yurii Shyrma (iuriish@yahoo.com)
//
#include <flatbuffers/util.h>
#include <helpers/ShapeUtils.h>

#include <algorithm>
#include <climits>
#include <numeric>
#include <set>

namespace sd {

//////////////////////////////////////////////////////////////////////////
// evaluate shape for array resulting from tensorDot operation, also evaluate shapes and dimensions permutations for
// transposition of two input arrays
std::vector<LongType> ShapeUtils::evalShapeForTensorDot(
    const sd::LongType* aShapeInfo, const sd::LongType* bShapeInfo, const std::vector<LongType> axesA,
    const std::vector<LongType> axesB, std::vector<sd::LongType>& permutAt, std::vector<sd::LongType>& permutBt,
    std::vector<sd::LongType>& shapeAt, std::vector<sd::LongType>& shapeBt) {
  sd::LongType axeAsize = static_cast<sd::LongType>(axesA.size());
  sd::LongType axeBsize =  static_cast<sd::LongType>(axesB.size());


  sd::LongType aRank = aShapeInfo[0];
  sd::LongType bRank = bShapeInfo[0];

  if (axeAsize != axeBsize) {
    std::string errorMessage;
    errorMessage += "ShapeUtils::evalShapeForTensorDot method: the numbers of a axes and b axes to make dot product along must have identical values !\n";
    errorMessage += "axesASize: ";
    errorMessage += std::to_string(axeAsize);
    errorMessage += ", axesBSize: ";
    errorMessage += std::to_string(axeBsize);
    errorMessage += "\n";
    THROW_EXCEPTION(errorMessage.c_str());
  }

  if (axeAsize > aRank || axeBsize > bRank) {
    std::string errorMessage;
    errorMessage += "ShapeUtils::evalShapeForTensorDot method: the length of vector of a or b axes is larger than array rank !\n";
    errorMessage += "axesASize: ";
    errorMessage += std::to_string(axeAsize);
    errorMessage += ", axesBSize: ";
    errorMessage += std::to_string(axeBsize);
    errorMessage += "\n";
    errorMessage += "aRank: ";
    errorMessage += std::to_string(aRank);
    errorMessage += ", bRank: ";
    errorMessage += std::to_string(bRank);
    errorMessage += "\n";
    THROW_EXCEPTION(errorMessage.c_str());
  }


  // check whether axesA and axesB contain only unique numbers
  std::set<sd::LongType> uniqueElems(axesA.begin(), axesA.end());
  if ((sd::LongType)uniqueElems.size() != axeAsize) {
    THROW_EXCEPTION("ShapeUtils::evalShapeForTensorDot method: the vector of a axes contains duplicates !");
  }
  uniqueElems.clear();
  uniqueElems = std::set<sd::LongType>(axesB.begin(), axesB.end());
  if ((sd::LongType)uniqueElems.size() != axeBsize) {
    std::string errorMessage;
    errorMessage += "ShapeUtils::evalShapeForTensorDot method: the vector of b axes contains duplicates !\n";
    errorMessage += "axesBsize: ";
    errorMessage += std::to_string(axesB.size());
    errorMessage += " uniqueElems: ";
    errorMessage += std::to_string(uniqueElems.size());
    THROW_EXCEPTION(errorMessage.c_str());
  }
  std::vector<sd::LongType> list_A, list_B;
  for (sd::LongType i = 0; i < aRank; i++)
    if (std::find(axesA.begin(), axesA.end(), i) == axesA.end()) list_A.emplace_back(i);
  for (sd::LongType i = 0; i < bRank; i++)
    if (std::find(axesB.begin(), axesB.end(), i) == axesB.end()) list_B.emplace_back(i);

  permutAt = list_A;
  permutAt.insert(permutAt.end(), axesA.begin(), axesA.end());
  permutBt = axesB;
  permutBt.insert(permutBt.end(), list_B.begin(), list_B.end());

  // if permute contains something like {0,1,2,..rank-1}, then there is no need to make permutation and we return empty
  // vector in this case
  sd::LongType i1, i2;
  for (i1 = 0; i1 < aRank; ++i1)
    if (permutAt[i1] != i1) break;
  if (i1 == aRank) permutAt = {};
  for (i2 = 0; i2 < bRank; ++i2)
    if (permutBt[i2] != i2) break;
  if (i2 == bRank) permutBt = {};

  sd::LongType n2 = 1;
  for (sd::LongType i = 0; i < axeAsize; i++) n2 *= aShapeInfo[axesA[i] + 1];
  shapeAt = {shape::length(aShapeInfo) / n2, n2};

  std::vector<sd::LongType> oldShapeA;
  oldShapeA.resize(list_A.size());
  for (sd::LongType i = 0; i < oldShapeA.size(); ++i) oldShapeA[i] = aShapeInfo[list_A[i] + 1];

  sd::LongType n3 = 1;
  for (sd::LongType i = 0; i < axeBsize; i++) n3 *= bShapeInfo[axesB[i] + 1];
  shapeBt = {n3, shape::length(bShapeInfo) / n3};

  std::vector<sd::LongType> oldShapeB;
  oldShapeB.resize(list_B.size());
  for (sd::LongType i = 0; i < oldShapeB.size(); i++) oldShapeB[i] = bShapeInfo[list_B[i] + 1];

  std::vector<sd::LongType> aPlusB(oldShapeA);
  aPlusB.insert(aPlusB.end(), oldShapeB.begin(), oldShapeB.end());

  return aPlusB;
}

//////////////////////////////////////////////////////////////////////////
std::vector<LongType> ShapeUtils::evalShapeForTensorDot(
    const NDArray* a, const NDArray* b, const std::vector<sd::LongType>& axesA, const std::vector<sd::LongType>& axesB,
    std::vector<sd::LongType>& permutAt, std::vector<sd::LongType>& permutBt, std::vector<sd::LongType>& shapeAt,
    std::vector<sd::LongType>& shapeBt) {
  return evalShapeForTensorDot(a->shapeInfo(), b->shapeInfo(), axesA, axesB, permutAt, permutBt, shapeAt, shapeBt);
}

//////////////////////////////////////////////////////////////////////////
// evaluate output shape for reduce operation when input shape is empty
const sd::LongType* ShapeUtils::evalReduceShapeInfoEmpty(const char order, std::vector<LongType>* dimsToExclude,
                                                         const sd::LongType* shapeInfo, const sd::DataType dataType,
                                                         const bool keepDims, sd::memory::Workspace* workspace) {
  if (dimsToExclude->size() == 0) {  // return copy of input shape
    sd::LongType* outShapeInfo = ShapeBuilders::copyShapeInfoAndType(shapeInfo, dataType, true, workspace);
    ShapeDescriptor *descriptor = new ShapeDescriptor(outShapeInfo, dataType);
    RELEASE(outShapeInfo, workspace);
    auto ret =  ConstantShapeHelper::getInstance().bufferForShapeInfo(descriptor)->primary();
    delete descriptor;
    return ret;
  }

  const sd::LongType rank = shape::rank(shapeInfo);
  sd::LongType* outShapeInfo = nullptr;

  if (dimsToExclude->size() == rank) {  // return scalar or shape filled with unities

    if (!keepDims)
      outShapeInfo = ShapeBuilders::createScalarShapeInfo(dataType, workspace);
    else
      outShapeInfo = ShapeBuilders::createShapeInfo(dataType, order, std::vector<sd::LongType>(rank, 1), workspace);
  } else {
    shape::checkDimensions(rank, dimsToExclude);

    std::vector<sd::LongType> outShape;

    if (keepDims) {
      outShape.assign(shapeInfo + 1, shapeInfo + 1 + rank);
      for (const auto dim : *dimsToExclude) outShape[dim] = 1;
    } else {
      for (sd::LongType i = 0, j = 0; i < rank; ++i) {
        if (j < dimsToExclude->size() && i == dimsToExclude->at(j))
          ++j;
        else
          outShape.emplace_back(shapeInfo[i + 1]);
      }
    }

    outShapeInfo = ShapeBuilders::createShapeInfo(dataType, order, outShape, workspace);
  }

  ShapeDescriptor *descriptor = new ShapeDescriptor(outShapeInfo, dataType);
  RELEASE(outShapeInfo, workspace);
  auto ret =  ConstantShapeHelper::getInstance().bufferForShapeInfo(descriptor)->primary();
  delete descriptor;
  return ret;
}

const sd::LongType* ShapeUtils::evalReduceShapeInfo(const char order, std::vector<LongType>* dimsToExclude,
                                                    const NDArray& arr, const bool keepDims,
                                                    const bool supportOldShapes, sd::memory::Workspace* workspace) {
  return evalReduceShapeInfo(order, dimsToExclude, arr, arr.dataType(), keepDims, supportOldShapes, workspace);
}

const sd::LongType* ShapeUtils::evalReduceShapeInfo(const char order, std::vector<LongType>* dimsToExclude,
                                                    const sd::LongType* shapeInfo, const bool keepDims,
                                                    const bool supportOldShapes, sd::memory::Workspace* workspace) {
  return evalReduceShapeInfo(order, dimsToExclude, shapeInfo, ArrayOptions::dataType(shapeInfo), keepDims,
                             supportOldShapes, workspace);
}

//////////////////////////////////////////////////////////////////////////
const sd::LongType* ShapeUtils::evalReduceShapeInfo(const char order, std::vector<LongType>* dimsToExclude,
                                                    const NDArray& arr, const sd::DataType dataType,
                                                    const bool keepDims, const bool supportOldShapes,
                                                    sd::memory::Workspace* workspace) {
  return evalReduceShapeInfo(order, dimsToExclude, arr.shapeInfo(), dataType, keepDims, supportOldShapes, workspace);
}

//////////////////////////////////////////////////////////////////////////
// evaluate shape resulting from reduce operation
const sd::LongType* ShapeUtils::evalReduceShapeInfo(const char order, std::vector<LongType>* dimsToExclude,
                                                    const sd::LongType* shapeInfo, const sd::DataType dataType,
                                                    const bool keepDims, const bool supportOldShapes,
                                                    sd::memory::Workspace* workspace) {
  if (ArrayOptions::arrayType(shapeInfo) == ArrayType::EMPTY)
    return ShapeUtils::evalReduceShapeInfoEmpty(order, dimsToExclude, shapeInfo, dataType, keepDims, workspace);

  sd::LongType* newShapeInfo = nullptr;

  sd::LongType rank = shape::rank(const_cast<sd::LongType*>(shapeInfo));

  if (dimsToExclude->size() == 0) {  // return scalar or array with len=1 in this case

    if (keepDims && rank > 1) {
      ALLOCATE(newShapeInfo, workspace, shape::shapeInfoLength(rank), sd::LongType);
      newShapeInfo[0] = rank;
      for (sd::LongType i = 0; i < rank; ++i) newShapeInfo[i + 1] = 1;
      ShapeUtils::updateStridesAndType(newShapeInfo, shapeInfo, order);
      ArrayOptions::setDataType(newShapeInfo, dataType);

      ShapeDescriptor *descriptor = new ShapeDescriptor(newShapeInfo, dataType);
      RELEASE(newShapeInfo, workspace);
      auto ret =  ConstantShapeHelper::getInstance().bufferForShapeInfo(descriptor)->primary();
      delete descriptor;
      return ret;
    } else if (supportOldShapes) {
      ALLOCATE(newShapeInfo, workspace, shape::shapeInfoLength(2), sd::LongType);
      shape::shapeOldScalar(dataType, newShapeInfo, 'c');
      ShapeDescriptor *descriptor = new ShapeDescriptor(newShapeInfo, dataType);
      RELEASE(newShapeInfo, workspace);
      auto ret =  ConstantShapeHelper::getInstance().bufferForShapeInfo(descriptor)->primary();
      delete descriptor;
      return ret;
    } else {
      newShapeInfo = ShapeBuilders::createScalarShapeInfo(dataType, workspace);
      ShapeDescriptor *descriptor = new ShapeDescriptor(newShapeInfo, dataType);
      auto ret =  ConstantShapeHelper::getInstance().bufferForShapeInfo(descriptor)->primary();
      delete descriptor;
      return ret;
    }
  }

  shape::checkDimensions(rank, dimsToExclude);

  sd::LongType dimSize = dimsToExclude->size();

  if (keepDims) {
    ALLOCATE(newShapeInfo, workspace, shape::shapeInfoLength(rank), sd::LongType);
    newShapeInfo[0] = rank;
    for (sd::LongType i = 0; i < rank; ++i)
      if (std::binary_search(dimsToExclude->begin(), dimsToExclude->end(),
                             i))  // dimsToExclude is already sorted after shape::checkDimensions() has been applied
        newShapeInfo[i + 1] = 1;
      else
        newShapeInfo[i + 1] = shapeInfo[i + 1];

    ShapeUtils::updateStridesAndType(newShapeInfo, shapeInfo, order);
    ShapeDescriptor *descriptor = new ShapeDescriptor(newShapeInfo, dataType);
    RELEASE(newShapeInfo, workspace);
    auto ret =  ConstantShapeHelper::getInstance().bufferForShapeInfo(descriptor)->primary();
    delete descriptor;
    return ret;
  }

  sd::LongType newRank = rank - dimSize;
  if (newRank == 0 ||
      (dimSize == 1 &&
       dimsToExclude->at(0) == INT_MAX)) {  // check whether given dimension is meant for the whole dimension

    if (supportOldShapes) {
      ALLOCATE(newShapeInfo, workspace, shape::shapeInfoLength(2), sd::LongType);
      shape::shapeOldScalar(ArrayOptions::dataType(shapeInfo), newShapeInfo, 'c');
      ShapeDescriptor *descriptor = new ShapeDescriptor(newShapeInfo, dataType);
      auto ret = ConstantShapeHelper::getInstance().bufferForShapeInfo(descriptor)->primary();
      RELEASE(newShapeInfo, workspace);
      delete descriptor;
      return ret;
    } else {
      newShapeInfo = ShapeBuilders::createScalarShapeInfo(ArrayOptions::dataType(shapeInfo), workspace);
      ShapeDescriptor *descriptor = new ShapeDescriptor(newShapeInfo, dataType);
      auto ret =  ConstantShapeHelper::getInstance().bufferForShapeInfo(descriptor)->primary();
      RELEASE(newShapeInfo, workspace);
      delete descriptor;
      return ret;
    }
  }

  ALLOCATE(newShapeInfo, workspace, shape::shapeInfoLength(newRank), sd::LongType);
  newShapeInfo[0] = newRank;  // set rank
  sd::LongType j = 1;
  for (sd::LongType i = 0; i < rank; ++i)
    if (!std::binary_search(dimsToExclude->begin(), dimsToExclude->end(),
                            i))  // dimsToExclude is already sorted after shape::checkDimensions() has been applied
      newShapeInfo[j++] = shapeInfo[i + 1];

  // ensure whether vector has proper shape for old shape type
  if (newRank == 1 && supportOldShapes) {
    sd::LongType oldValue = newShapeInfo[1];
    RELEASE(newShapeInfo, workspace);
    ALLOCATE(newShapeInfo, workspace, shape::shapeInfoLength(2), sd::LongType);  // set newRank = 2
    newShapeInfo[0] = 2;
    if (dimsToExclude->at(0) == 0) {
      newShapeInfo[1] = 1;
      newShapeInfo[2] = oldValue;
    } else {
      newShapeInfo[1] = oldValue;
      newShapeInfo[2] = 1;
    }
  }

  ShapeUtils::updateStridesAndType(newShapeInfo, shapeInfo, order);

  ShapeDescriptor *descriptor = new ShapeDescriptor(newShapeInfo, dataType);
  RELEASE(newShapeInfo, workspace);
  auto ret = ConstantShapeHelper::getInstance().bufferForShapeInfo(descriptor)->primary();
  delete descriptor;
  return ret;
}

//////////////////////////////////////////////////////////////////////////
// evaluate shape for array which is result of repeat operation applied to arr
std::vector<sd::LongType> ShapeUtils::evalRepeatShape(LongType axis, const std::vector<LongType>& repeats, const NDArray& arr) {
  if (axis < 0) axis += arr.rankOf();

  if (repeats.size() != 1 && repeats.size() != arr.sizeAt(axis))
    THROW_EXCEPTION(
        "ShapeUtils::evalRepeatShape: size of repeats vector must be 1 or equal to dimension at given axis !");

  std::vector<sd::LongType> outShape = arr.getShapeAsVector();

  if (repeats.size() == 1)
    outShape[axis] *= repeats[0];
  else
    outShape[axis] = std::accumulate(repeats.begin(), repeats.end(), 0);

  return outShape;
}

//////////////////////////////////////////////////////////////////////////
// evaluate shapeInfo of permuted array
const sd::LongType* ShapeUtils::evalPermShapeInfo(const LongType* dimensions, const LongType rank, const NDArray& arr,
                                                  sd::memory::Workspace* workspace, const bool setContigStrides) {

  if (rank != arr.rankOf())
    THROW_EXCEPTION("ShapeUtils::evalPermShapeInfo static method: wrong arguments: rank is not suitable!");


  auto shapeInfoLength = shape::shapeInfoLength(rank);

  // allocate memory for new array - shapeInfo
  sd::LongType* shapeInfoNew = nullptr;
  ALLOCATE(shapeInfoNew, workspace, shapeInfoLength, sd::LongType);

  // copy arr _shapeInfo into new array
  memcpy(shapeInfoNew, arr.shapeInfo(), shape::shapeInfoByteLength(rank));

  // perform buffer permutation
  shape::doPermuteShapeInfo(shapeInfoNew, dimensions, arr.lengthOf());

  if (setContigStrides) shape::updateStrides(shapeInfoNew, arr.ordering());

  ShapeDescriptor *descriptor = new ShapeDescriptor(shapeInfoNew);

  auto ret = ConstantShapeHelper::getInstance().bufferForShapeInfo(descriptor)->primary();
  RELEASE(shapeInfoNew, workspace);
  delete descriptor;
  return ret;
}

//////////////////////////////////////////////////////////////////////////
// evaluate shapeInfo of permuted array

//////////////////////////////////////////////////////////////////////////
// evaluate shapeInfo of transposed array
const sd::LongType* ShapeUtils::evalTransposeShapeInfo(const NDArray& arr, sd::memory::Workspace* workspace,
                                                       const bool setContigStrides) {
  sd::LongType rank = arr.rankOf();

  //note we do this because of stack allocation crashes
  //if the stack is used a vector's data can cause crashes when it goes out of scope
  sd::LongType  *dims = new sd::LongType[rank];
  for (sd::LongType i = 0; i < rank; i++) {
    dims[i] = rank - 1 - i;
    sd_printf("evalTransposeShapeInfo: dims[%i] = %i\n", i, dims[i]);
  }

  auto ret = evalPermShapeInfo(dims, rank, arr, workspace, setContigStrides);
  delete[] dims;
  return ret;
}

//////////////////////////////////////////////////////////////////////////
bool ShapeUtils::copyVectorPart(std::vector<sd::LongType>& target, std::vector<sd::LongType>& source, LongType rank,
                                LongType offset) {
  if (source.size() < offset + rank) return false;

  for (sd::LongType e = offset; e < offset + rank; e++) target.push_back(source[e]);

  return true;
}

//////////////////////////////////////////////////////////////////////////
// return new (shorter) sorted dimensions array without dimensions that are present in input vector
std::vector<sd::LongType>* ShapeUtils::evalDimsToExclude(const LongType rank, const LongType dimsLen, const sd::LongType* dimensions) {
  std::vector<sd::LongType> *newDimensions = new std::vector<sd::LongType>();
  if (dimsLen == 0) {  // if input vector is empty then return whole shape range
    newDimensions->resize(rank);
    std::iota(newDimensions->begin(), newDimensions->end(), 0);  // fill with 0, 1, ... rank-1
  } else {
    bool isAbsent;
    for (sd::LongType  i = 0; i < rank; i++) {
      isAbsent = true;
      for (sd::LongType  j = 0; j < dimsLen; j++) {
        sd::LongType dim = dimensions[j] >= 0 ? dimensions[j] : dimensions[j] + rank;
        if (i == dim) {
          isAbsent = false;
          break;
        }
      }
      if (isAbsent) newDimensions->emplace_back(i);
    }
  }


  return newDimensions;
}

//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// check whether 2 arrays have mutually broadcastable shapes
// shape comparison starts from the end
bool ShapeUtils::areShapesBroadcastable(const NDArray& arr1, const NDArray& arr2) {
  return areShapesBroadcastable(arr1.shapeInfo(), arr2.shapeInfo());
}

bool ShapeUtils::areShapesBroadcastable(const sd::LongType* shapeInfo1, const sd::LongType* shapeInfo2) {
  sd::LongType minRank = shape::rank(shapeInfo1) < shape::rank(shapeInfo2) ? shape::rank(shapeInfo1) : shape::rank(shapeInfo2);

  for (sd::LongType i = -1; i >= -minRank; --i)
    if (shape::sizeAt(shapeInfo1, i) != shape::sizeAt(shapeInfo2, i) && shape::sizeAt(shapeInfo1, i) != 1 &&
        shape::sizeAt(shapeInfo2, i) != 1)
      return false;

  return true;
}

bool ShapeUtils::areShapesBroadcastable(const std::vector<sd::LongType>& shape1,
                                        const std::vector<sd::LongType>& shape2) {
  const auto rank1 = shape1.size();
  const auto rank2 = shape2.size();
  const sd::LongType minRank = rank1 < rank2 ? rank1 : rank2;

  for (sd::LongType i = 1; i <= minRank; ++i)
    if (shape1[rank1 - i] != shape2[rank2 - i] && shape1[rank1 - i] != 1 && shape2[rank2 - i] != 1) return false;

  return true;
}

//////////////////////////////////////////////////////////////////////////
// check the possibility of broadcast operation, if true then return shapeInfo of resulting array
// if evalMinMax == false the array with larger rank has to be passed as first argument
bool ShapeUtils::evalBroadcastShapeInfo(const NDArray& max, const NDArray& min, const bool evalMinMax,
                                        const sd::LongType*& resultShapeInfo, sd::memory::Workspace* workspace) {
  return evalBroadcastShapeInfo(max.shapeInfo(), min.shapeInfo(), evalMinMax, resultShapeInfo, workspace);
}

bool ShapeUtils::evalBroadcastShapeInfo(const sd::LongType* max, const sd::LongType* min, const bool evalMinMax,
                                        const sd::LongType*& resultShapeInfo, sd::memory::Workspace* workspace) {
  // check whether broadcast operation is possible for input arrays
  if (!areShapesBroadcastable(max, min)) return false;

  auto maxShapeInfo = max;
  auto minShapeInfo = min;
  if (evalMinMax && (shape::rank(max) < shape::rank(min))) {
    maxShapeInfo = min;
    minShapeInfo = max;
  }

  const auto maxRank = shape::rank(maxShapeInfo);
  const auto minRank = shape::rank(minShapeInfo);

  // evaluate shapeInfo for resulting array
  if (resultShapeInfo != nullptr)
    THROW_EXCEPTION(
        "std::runtime_error(ShapeUtils::evalBroadcastShapeInfo method: the input pointer on shapeInfo must be empty "
        "(=nullptr) !");

  sd::LongType* tmpShapeInfo = nullptr;
  ALLOCATE(tmpShapeInfo, workspace, shape::shapeInfoLength(maxRank), sd::LongType);

  // FIXME: get rid of memcpy here
  memcpy(tmpShapeInfo, maxShapeInfo, shape::shapeInfoByteLength(maxRank));
  for (sd::LongType i = 0; i < minRank; ++i)
    if ((maxShapeInfo[maxRank - i] != 0 && maxShapeInfo[maxRank - i] < minShapeInfo[minRank - i]) ||
        minShapeInfo[minRank - i] == 0)
      tmpShapeInfo[maxRank - i] = minShapeInfo[minRank - i];

  ShapeUtils::updateStridesAndType(tmpShapeInfo, DataTypeUtils::pickPairwiseResultType(maxShapeInfo, minShapeInfo),
                                   shape::order(maxShapeInfo));

  if (shape::isEmpty(max) || shape::isEmpty(min)) {
    ArrayOptions::setPropertyBit(tmpShapeInfo, ARRAY_EMPTY);
    memset(shape::stride(tmpShapeInfo), 0, shape::rank(tmpShapeInfo) * sizeof(sd::LongType));
  }

  ShapeDescriptor *descriptor = new ShapeDescriptor(tmpShapeInfo);
  RELEASE(tmpShapeInfo, workspace);
  resultShapeInfo = ConstantShapeHelper::getInstance().bufferForShapeInfo(descriptor)->primary();
  delete descriptor;
  return true;
}

//////////////////////////////////////////////////////////////////////////
// check the possibility of broadcast operation for set of arrays, if true then return resulting broadcasted shapeInfo
bool ShapeUtils::evalCommonBroadcastShapeInfo(const std::vector<const NDArray*>& arrays, sd::LongType*& resultShapeInfo,
                                              memory::Workspace* workspace) {
  if (resultShapeInfo != nullptr)
    THROW_EXCEPTION(
        "ShapeUtils::evalCommonBroadcastShapeInfo method: the input pointer on shapeInfo must be empty (=nullptr) !");

  sd::LongType size = arrays.size();
  sd::LongType maxRank = arrays[size - 1]->rankOf();

  for (sd::LongType i = 0; i < size - 1; ++i) {
    if (arrays[i]->rankOf() > maxRank) maxRank = arrays[i]->rankOf();
    for (sd::LongType j = i + 1; j < size; ++j)
      if (!areShapesBroadcastable(*arrays[i], *arrays[j])) return false;
  }

  sd::LongType* tmpShapeInfo = nullptr;
  ALLOCATE(tmpShapeInfo, workspace, shape::shapeInfoLength(maxRank), sd::LongType);
  memset(tmpShapeInfo, 0, shape::shapeInfoByteLength(maxRank));
  tmpShapeInfo[0] = maxRank;

  for (const auto& item : arrays) {
    for (sd::LongType i = -1; i >= -item->rankOf(); --i)
      if (tmpShapeInfo[i + 1 + maxRank] < item->sizeAt(i)) tmpShapeInfo[i + 1 + maxRank] = item->sizeAt(i);
  }

  shape::updateStrides(tmpShapeInfo, arrays[0]->ordering());
  ArrayOptions::setDataType(tmpShapeInfo, arrays[0]->dataType());

  ShapeDescriptor *descriptor = new ShapeDescriptor(tmpShapeInfo);
  RELEASE(tmpShapeInfo, workspace);
  auto bufferForSHape = ConstantShapeHelper::getInstance().bufferForShapeInfo(descriptor);
  resultShapeInfo = const_cast<sd::LongType*>(bufferForSHape->primary());
  delete descriptor;
  return true;
}

//////////////////////////////////////////////////////////////////////////
// return sorted vector of dimensions common (same) for two arrays, dimensions values corresponds to array with bigger
// rank for example if arr1{2,7}, arr2{2,5,4,7} then vector = {0,3}
std::vector<sd::LongType> ShapeUtils::getDimsWithSameShape(const NDArray& arr1, const NDArray& arr2) {
  const NDArray *min, *max;

  if (arr1.rankOf() >= arr2.rankOf()) {
    max = &arr1;
    min = &arr2;
  } else {
    max = &arr2;
    min = &arr1;
  }

  const sd::LongType rankDiff = max->rankOf() - min->rankOf();

  std::vector<sd::LongType> dims;

  for (sd::LongType i = 0; i < min->rankOf(); ++i)
    if (min->sizeAt(i) == max->sizeAt(rankDiff + i)) dims.emplace_back(rankDiff + i);

  return dims;
}

//////////////////////////////////////////////////////////////////////////
// evaluate shapeInfo for resulting array from tile operation
const sd::LongType* ShapeUtils::evalTileShapeInfo(const NDArray& arr, const std::vector<sd::LongType>& reps,
                                                  sd::memory::Workspace* workspace) {
  // check whether reps contains at least one zero (then throw exception) or whether all elements in reps are unities
  // (then simply reshape or do nothing)
  sd::LongType repsSize = reps.size();
  sd::LongType product = 1;
  for (const auto& item : reps) product *= item;
  if (product == 0) THROW_EXCEPTION("NDArray::tile method: one of the elements in reps array is zero !");

  sd::LongType rankOld = arr.rankOf();
  sd::LongType diff = rankOld - repsSize;

  // evaluate new shapeInfo
  sd::LongType* newShapeInfo = nullptr;
  if (diff < 0) {
    ALLOCATE(newShapeInfo, workspace, shape::shapeInfoLength(repsSize), sd::LongType);
    newShapeInfo[0] = repsSize;  // set new rank
    for (sd::LongType i = 1; i <= -diff; ++i)
      newShapeInfo[i] = 1;  // set unities to be new dimensions at left-hand side of newShapeInfo shape place
    memcpy(newShapeInfo + 1 - diff, arr.shapeInfo() + 1,
           rankOld * sizeof(sd::LongType));  // copy old dimensions to the right-hand side of newShapeInfo shape place
    for (sd::LongType i = 1; i <= repsSize; ++i)
      newShapeInfo[i] *= reps[i - 1];  // set new shape by multiplying old dimensions by corresponding numbers from reps
  } else {
    ALLOCATE(newShapeInfo, workspace, shape::shapeInfoLength(rankOld), sd::LongType);
    memcpy(newShapeInfo, arr.shapeInfo(),
           shape::shapeInfoByteLength(rankOld));  // copy all elements of _shapeInfo to newShapeInfo
    for (sd::LongType i = 1; i <= repsSize; ++i)
      newShapeInfo[rankOld + 1 - i] *=
          reps[repsSize - i];  // set new shape by multiplying old dimensions by corresponding numbers from reps
  }
  shape::updateStrides(newShapeInfo, arr.ordering());
  ArrayOptions::setDataType(newShapeInfo, arr.dataType());

  ShapeDescriptor *descriptor = new ShapeDescriptor(newShapeInfo);
  RELEASE(newShapeInfo, workspace);
  auto ret =  ConstantShapeHelper::getInstance().bufferForShapeInfo(descriptor)->primary();
  delete descriptor;
  return ret;
}

std::vector<sd::LongType> ShapeUtils::pullShapeFromShapeInfo(const sd::LongType* shapeInfo) {
  std::vector<sd::LongType> shape(shape::rank(shapeInfo));
  sd::LongType shapeSize = shape.size();

  for (sd::LongType e = 0; e < shapeSize; e++) shape[e] = shape::shapeOf(shapeInfo)[e];

  return shape;
}

std::string ShapeUtils::shapeAsString(const NDArray* array) {
  if(array->rankOf() == 0 && !array->isEmpty())
    return "[0]";

  std::string result;

  result.append("[");
  for (sd::LongType e = 0; e < array->rankOf(); e++) {
    result += flatbuffers::NumToString(array->sizeAt(e));
    if (e < array->rankOf() - 1) result.append(", ");
  }
  result.append("]");

  return result;
}

std::string ShapeUtils::strideAsString(const NDArray* array) {
  std::string result;

  auto shapeBuffer = array->shapeInfo();  // sd::LongType*
  sd::LongType rank = (sd::LongType)*shapeBuffer;
  result.append("[");
  for (sd::LongType e = 0; e < rank; e++) {
    if (e > 0) result.append(",");
    sd::LongType stride = *(shapeBuffer + rank + 1 + e);
    result += flatbuffers::NumToString(stride);
  }
  result.append("]");

  return result;
}

std::string ShapeUtils::shapeAsString(const std::vector<sd::LongType>& shape) {
  std::string result;

  result.append("[");
  for (sd::LongType e = 0; e < shape.size(); e++) {
    result += flatbuffers::NumToString(shape.at(e));
    if (e < shape.size() - 1) result.append(", ");
  }
  result.append("]");

  return result;
}

std::string ShapeUtils::shapeAsString(const sd::LongType* shapeInfo) {
  if (shapeInfo == nullptr) THROW_EXCEPTION("ShapeUtils::shapeAsString method: input shapeInfo must not be nullptr !");

  if(shapeInfo[0] < 0 || shapeInfo[0] > SD_MAX_RANK) {
    THROW_EXCEPTION("Shape info appears to be corrupt. Shape info[0] is less than 0 or greater than 32. Might have been deallocated.");
  }

  std::string result;

  result.append("[");
  for (sd::LongType e = 0; e < shapeInfo[0]; e++) {
    result += flatbuffers::NumToString(shapeInfo[e + 1]);
    if (e < shapeInfo[0] - 1) result.append(", ");
  }
  result.append("]");

  return result;
}

std::string ShapeUtils::shapeInfoAsString(const sd::LongType* shapeInfo) {
  if (!shapeInfo) THROW_EXCEPTION("ShapeUtils::shapeAsString method: input shapeInfo must not be nullptr !");

  std::string result;

  sd::LongType len = shape::shapeInfoLength(shapeInfo[0]);

  result.append("[");
  for (sd::LongType e = 0; e < len; e++) {
    result += flatbuffers::NumToString(shapeInfo[e]);
    if (e < len - 1) result.append(", ");
  }
  result.append("]");

  return result;
}

std::string ShapeUtils::shapeAsString(const LongType rank, const sd::LongType* shapeInfo) {
  if (!shapeInfo) THROW_EXCEPTION("ShapeUtils::shapeAsString method: input shapeInfo must not be nullptr !");

  std::string result;

  result.append("[");
  for (sd::LongType e = 0; e < rank; e++) {
    result += flatbuffers::NumToString(shapeInfo[e]);
    if (e < rank - 1) result.append(", ");
  }
  result.append("]");

  return result;
}

//////////////////////////////////////////////////////////////////////////
std::vector<sd::LongType> ShapeUtils::shapeAsVector(const sd::LongType* shapeInfo) {
  if (!shapeInfo) THROW_EXCEPTION("ShapeUtils::shapeAsVector method: input shapeInfo must not be nullptr !");

  std::vector<sd::LongType> vector(shapeInfo[0]);

  for (sd::LongType e = 0; e < shapeInfo[0]; e++) vector[e] = shapeInfo[e + 1];

  return vector;
}

//////////////////////////////////////////////////////////////////////////
// evaluate shapeInfo for diagonal array which is made using input arr elements as diagonal
const sd::LongType* ShapeUtils::evalDiagShapeInfo(const sd::LongType* shapeInfoConst,
                                                  sd::memory::Workspace* workspace) {
  auto shapeInfo = const_cast<sd::LongType*>(shapeInfoConst);

  const auto rank = shape::rank(shapeInfo);

  sd::LongType* outputShapeInfo = nullptr;

  if (shape::isVector(shapeInfo) || shape::isScalar(shapeInfo)) {
    ALLOCATE(outputShapeInfo, workspace, shape::shapeInfoLength(2), sd::LongType);
    outputShapeInfo[0] = 2;
    outputShapeInfo[1] = outputShapeInfo[2] = shape::length(shapeInfo);
  } else {
    ALLOCATE(outputShapeInfo, workspace, shape::shapeInfoLength(2 * rank), sd::LongType);
    outputShapeInfo[0] = 2 * rank;
    for (sd::LongType i = 1; i <= rank; ++i) outputShapeInfo[i] = outputShapeInfo[i + rank] = shapeInfo[i];
  }

  ShapeUtils::updateStridesAndType(outputShapeInfo, shapeInfo, shape::order(shapeInfo));
  auto nonConstShape = const_cast<sd::LongType *>(outputShapeInfo);
  auto result = ConstantShapeHelper::getInstance().bufferForShapeInfo(nonConstShape);
  RELEASE(outputShapeInfo, workspace);
  return result->primary();
}

std::vector<sd::LongType> ShapeUtils::evalBroadcastBackwardAxis(const sd::LongType* operand,
                                                                const sd::LongType* result) {
  // rRank >= oRank always  !!
  const auto oRank = shape::rank(operand);
  const auto rRank = shape::rank(result);
  const auto diff = rRank - oRank;
  std::vector<sd::LongType> axis;

  for (sd::LongType i = 0; i < rRank; ++i)
    if (i < diff || shape::sizeAt(operand, i - diff) != shape::sizeAt(result, i)) axis.push_back(i);

  return axis;
}

////////////////////////////////////////////////////////////////////////////////
const sd::LongType* ShapeUtils::matrixProductShape(const sd::LongType* theFirstShape,
                                                   const sd::LongType* theSecondShape, bool shouldTranspondFirst,
                                                   bool shouldTranspondSecond, sd::DataType dtype,
                                                   sd::memory::Workspace* workspace) {
  auto inA = theFirstShape;
  auto inB = theSecondShape;
  sd::LongType* shape;
  ALLOCATE(shape, workspace, shape::shapeInfoLength(2), sd::LongType);

  sd::LongType* tmpA = ShapeBuilders::copyShapeInfo(inA, true, workspace);
  sd::LongType* tmpB = ShapeBuilders::copyShapeInfo(inB, true, workspace);

  if (shouldTranspondFirst) shape::transposeInplace(tmpA);

  if (shouldTranspondSecond) shape::transposeInplace(tmpB);

  if (shape::rank(tmpA) == 1 && shape::isMatrix(tmpB)) {
    // special case here
    shape[0] = 1;
    shape[1] = tmpB[2];
    sd::LongType* newShape = ShapeBuilders::createShapeInfo(dtype, 'f', 2, shape, workspace);

    RELEASE(shape, workspace);
    RELEASE(tmpA, workspace);
    RELEASE(tmpB, workspace);

    return newShape;
  } else if (shape::isScalar(tmpA) && shape::isScalar(tmpB)) {
    // just scalar vs scalar
    shape[0] = 1;
    shape[1] = 1;
  } else if (shape::isMatrix(tmpA) && shape::isVector(tmpB)) {
    // gemv case
    if (shape::rank(tmpB) == 2) {
      shape[0] = tmpA[1];
      shape[1] = tmpB[2];
    } else {
      // we have new 1D shape here
      auto newShape = ShapeBuilders::createVectorShapeInfo(dtype, tmpA[1], workspace);

      RELEASE(shape, workspace);
      RELEASE(tmpA, workspace);
      RELEASE(tmpB, workspace);

      return newShape;
    }
  } else if ((shape::isMatrix(tmpA) && shape::isMatrix(tmpB)) || (shape::isVector(tmpA) && shape::isMatrix(tmpB)) ||
             (shape::isColumnVector(tmpA) && shape::isVector(tmpB))) {
    // gemm case
    shape[0] = tmpA[1];
    shape[1] = tmpB[2];
  } else if ((shape::isVector(tmpA) && shape::isScalar(tmpB)) || (shape::isScalar(tmpA) && shape::isVector(tmpB))) {
    // element-wise
    shape[0] = 1;
    shape[1] = (sd::LongType)sd::math::sd_max<sd::LongType>(shape::length(tmpA), shape::length(tmpB));
  } else if (shape::isRowVector(tmpA) && shape::isRowVector(tmpB)) {
    // dot case
    shape[0] = 1;
    shape[1] = 1;
  } else if (shape::isRowVector(tmpA) && shape::isColumnVector(tmpB)) {
    // dot case
    shape[0] = 1;
    shape[1] = 1;
  }

  auto newShape = ConstantShapeHelper::getInstance().createShapeInfo(dtype, 'f', 2, shape);

  RELEASE(shape, workspace);

  RELEASE(tmpA, workspace);
  RELEASE(tmpB, workspace);
  return newShape;
}

////////////////////////////////////////////////////////////////////////////////
std::vector<sd::LongType> ShapeUtils::evalPermutFromTo(const std::vector<sd::LongType>& shapeFrom,
                                                       const std::vector<sd::LongType>& shapeTo) {
  auto rank = shapeFrom.size();
  if (rank != shapeTo.size())
    THROW_EXCEPTION(
        "ShapeUtils::evalPermutFromTo static method: the input shapes are not suitable for mutual permutation !");

  if (std::equal(begin(shapeFrom), end(shapeFrom),
                 begin(shapeTo)))  // if shapes are identical (permutation is unnecessary) then return empty vector
    return std::vector<sd::LongType>();

  std::vector<sd::LongType> permutation(rank, -2);       // vector to be returned
  std::vector<sd::LongType> shapeTo2(shapeTo);  // make copy of const vector since we will change the content of shapeTo

  for (sd::LongType i = 0; i < rank; ++i)
    for (sd::LongType j = 0; j < rank; ++j)
      if (shapeFrom[i] == shapeTo2[j]) {
        permutation[j] = i;
        shapeTo2[j] = -2;  // mark coincidence as -2 in order to not account index of shapeTo twice
        break;
      }

  if (std::find(begin(permutation), end(permutation), -2) !=
      end(permutation))  // if -2 is still present in vector then permutation is impossible
    THROW_EXCEPTION(
        "ShapeUtils::evalPermutFromTo static method: the input shapes are not suitable for mutual permutation !");

  return permutation;
}

////////////////////////////////////////////////////////////////////////////////
std::vector<sd::LongType> ShapeUtils::composeShapeUsingDimsAndIdx(const std::vector<LongType>& dimsAndIdx) {
  auto size = dimsAndIdx.size();
  if (size % 2 != 0)
    THROW_EXCEPTION(
        "ShapeUtils::composeShapeUsingDimsAndIdx static method: the size of input vector must be even !");

  size /= 2;

  std::vector<sd::LongType> shape(size);
  sd::LongType index;

  for (sd::LongType i = 0; i < size; ++i) {
    index = dimsAndIdx[i + size];
    if (index > size - 1)
      THROW_EXCEPTION("ShapeUtils::composeShapeUsingDimsAndIdx static method: input index is too large !");
    shape[index] = dimsAndIdx[i];
  }

  return shape;
}

////////////////////////////////////////////////////////////////////////////////
std::vector<sd::LongType> ShapeUtils::evalShapeForMatmul(const sd::LongType* xShapeInfo, const sd::LongType* yShapeInfo,
                                                         const bool transX, const bool transY) {
  const auto xRank = xShapeInfo[0];
  const auto yRank = yShapeInfo[0];

  const sd::LongType x0Dim = transX ? xShapeInfo[xRank] : xShapeInfo[xRank - 1];
  const sd::LongType y0Dim = transY ? yShapeInfo[yRank] : yShapeInfo[yRank - 1];
  const sd::LongType x1Dim = transX ? xShapeInfo[xRank - 1] : xShapeInfo[xRank];
  const sd::LongType y1Dim = transY ? yShapeInfo[yRank - 1] : yShapeInfo[yRank];

  if (xRank == 1 && yRank == 1) {  // dot case, output is scalar
    if (xShapeInfo[1] != yShapeInfo[1]) {
      sd_printf(
          "ShapeUtils::evalShapeForMatmul method: since input arrays are vectors they must have the same length, but "
          "got x length = %i, y length = %i !",
          xShapeInfo[1], yShapeInfo[1]);
      THROW_EXCEPTION("");
    }
    return std::vector<sd::LongType>({});
  }

  if (xRank == 1 && yRank == 2) {  // vector x matrix, i.e. [4] x [4,5] = [5], output is vector
    if (xShapeInfo[1] != y0Dim) {
      sd_printf(
          "ShapeUtils::evalShapeForMatmul method: input arrays have inconsistent shapes for vector-matrix product: x "
          "%s, y %s !",
          ShapeUtils::shapeAsString(xShapeInfo).c_str(), ShapeUtils::shapeAsString(yShapeInfo).c_str());
      THROW_EXCEPTION("");
    }
    return std::vector<sd::LongType>({y1Dim});
  }

  if (xRank == 2 && yRank == 1) {  // matrix x vector , i.e. [4,5] x [5] = [4], output is vector
    if (x1Dim != yShapeInfo[1]) {
      sd_printf(
          "ShapeUtils::evalShapeForMatmul method: input arrays have inconsistent shapes for vector-matrix product: x "
          "%s, y %s !",
          ShapeUtils::shapeAsString(xShapeInfo).c_str(), ShapeUtils::shapeAsString(yShapeInfo).c_str());
      THROW_EXCEPTION("");
    }
    return std::vector<sd::LongType>({x0Dim});
  }

  // rest cases - usual 2Dx2D or batched mmul
  if (xRank != yRank) {
    sd_printf(
        "ShapeUtils::evalShapeForMatmul static method: the ranks of arrays must be the same, but got xRank = %i and "
        "yRank = %i ! \n",
        xRank, yRank);
    THROW_EXCEPTION("");
  }

  if (x1Dim != y0Dim) {
    sd_printf("ShapeUtils::evalShapeForMatmul static method: input shapes are inconsistent: xDim %i != yDim %i \n",
              x1Dim, y0Dim);
    THROW_EXCEPTION("");
  }

  for (sd::LongType i = 0; i < xRank - 2; ++i)
    if (xShapeInfo[i + 1] != yShapeInfo[i + 1]) {
      sd_printf(
          "ShapeUtils::evalShapeForMatmul static method: input shapes are inconsistent: xShape = %s, yShape = %s ! \n",
          ShapeUtils::shapeAsString(xShapeInfo).c_str(), ShapeUtils::shapeAsString(yShapeInfo).c_str());
      THROW_EXCEPTION("");
    }

  std::vector<sd::LongType> cShape(xRank);

  // copy batch part of shape (if present)
  for (sd::LongType i = 0; i < xRank - 2; ++i) cShape[i] = xShapeInfo[i + 1];
  // copy rest part of shape (two dims: multiplication part)
  cShape[xRank - 2] = x0Dim;
  cShape[xRank - 1] = y1Dim;

  return cShape;
}

////////////////////////////////////////////////////////////////////////////////
sd::LongType ShapeUtils::getNumOfSubArrs(const sd::LongType* shapeInfo, const std::vector<LongType>& dimsToExclude) {
  sd::LongType numOfSubArrs = 1;

  if (dimsToExclude.size() == shape::rank(shapeInfo) ||
      dimsToExclude.size() == 0)  // means there is only one sub-array and it coincides with whole array
    return numOfSubArrs;

  for (const auto& dim : dimsToExclude) numOfSubArrs *= shapeInfo[dim + 1];

  return numOfSubArrs;
}

////////////////////////////////////////////////////////////////////////////////
std::vector<sd::LongType> ShapeUtils::evalDimsWithoutUnities(const sd::LongType* shapeInfo) {
  std::vector<sd::LongType> result;
  for (sd::LongType i = 1; i <= shapeInfo[0]; ++i)
    if (shapeInfo[i] != 1) result.push_back(shapeInfo[i]);

  return result;
}

////////////////////////////////////////////////////////////////////////////////
void ShapeUtils::updateStridesAndType(sd::LongType* dest, const sd::LongType* source, const char order) {
  shape::updateStrides(dest, order);
  dest[2 * dest[0] + 1] = 0;  // zero extra
  ArrayOptions::copyDataType(dest, source);
}

////////////////////////////////////////////////////////////////////////////////
void ShapeUtils::updateStridesAndType(sd::LongType* dest, const DataType dtype, const char order) {
  shape::updateStrides(dest, order);
  ArrayOptions::setDataType(dest, dtype);
}

////////////////////////////////////////////////////////////////////////////////
std::vector<sd::LongType> ShapeUtils::tadAxesForSimpleBroadcast(const NDArray& max, const NDArray& min) {
  const sd::LongType maxRank = max.rankOf();
  const sd::LongType minRank = min.rankOf();
  const sd::LongType diff = maxRank - minRank;

  sd::LongType numOfMinTads(1), numOfMaxTads(1);
  std::vector<sd::LongType> maxTadDims;

  for (sd::LongType i = 0; i < minRank; ++i) {
    if (min.sizeAt(i) == max.sizeAt(diff + i))
      maxTadDims.push_back(diff + i);
    else {
      numOfMinTads *= min.sizeAt(i);
      numOfMaxTads *= max.sizeAt(i);
    }
  }

  if (min.lengthOf() > max.lengthOf()) {  // in this case tad is max array
    for (sd::LongType i = 0; i < diff; ++i) numOfMaxTads *= max.sizeAt(i);

    return numOfMaxTads == 1 ? maxTadDims : std::vector<sd::LongType>();
  }

  return numOfMinTads == 1 ? maxTadDims : std::vector<sd::LongType>();
}

void ShapeUtils::copyCertainStridesFromShapeInfo(const sd::LongType* inShapeInfo, const LongType nRank,
                                                 const LongType dimsSize,
                                                 const sd::LongType* dims, sd::LongType* outStrides) {
  sd::LongType yRank = shape::rank(inShapeInfo);
  auto yOrigStride = shape::stride(inShapeInfo);

  if (yRank == nRank) {
    for (sd::LongType i = 0; i < yRank; ++i) {
      // x[2,3,4] * y[2,1,4] = z[2,3,4]
      outStrides[i] = (1 == shape::sizeAt(inShapeInfo, i)) ? 0 : yOrigStride[i];
    }
  } else {
    auto dimEx = sd::ShapeUtils::evalDimsToExclude(nRank, dimsSize, dims);

    for (sd::LongType i = 0, it = 0; i < nRank; ++i) {
      auto nCount = std::count(dimEx->cbegin(), dimEx->cend(), i);
      outStrides[i] = (0 == nCount) ? yOrigStride[it++] : 0;
      if (it == yRank) break;
    }
  }
}

bool ShapeUtils::areShapesEqual(const sd::LongType* shapeInfo, const std::vector<sd::LongType>& shapeOnly) {
  if (shape::rank(shapeInfo) != shapeOnly.size()) return false;

  for (sd::LongType i = 0; i < shape::rank(shapeInfo); ++i)
    if (shape::shapeOf(shapeInfo)[i] != shapeOnly[i]) return false;

  return true;
}

////////////////////////////////////////////////////////////////////////////////
std::vector<sd::LongType>* ShapeUtils::evalDimsForReduceOp(const LongType rank,
                                                           const std::vector<LongType>* dimsToExclude) {
  std::vector<sd::LongType>* dims = ShapeUtils::evalDimsToExclude(rank, dimsToExclude->size(),dimsToExclude->data());
  std::vector<sd::LongType>* output = new std::vector<sd::LongType>(*dims);

  sd::LongType dimsExcludeLen = static_cast<sd::LongType>(dimsToExclude->size());
  for (sd::LongType j = 0; j < dimsExcludeLen; j++) {
    sd::LongType currElement = dimsToExclude->at(j);
    bool contains = false;
    for(int i = 0; i < output->size(); i++) {
      if(output->at(i) == currElement) {
        contains = true;
        break;
      }
      else {
        contains = false;
      }
    }

    bool elementLess = currElement < rank;
    if(!contains && elementLess) {
      output->push_back(dimsToExclude->at(j));
    }
  }

  delete dims;
  return output;
}

////////////////////////////////////////////////////////////////////////////////


}  // namespace sd
