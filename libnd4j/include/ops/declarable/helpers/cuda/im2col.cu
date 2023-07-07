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
// Created by raver119 on 30.11.17.
//
#include <helpers/PointersManager.h>
#include <ops/declarable/helpers/im2col.h>

namespace sd {
namespace ops {
namespace helpers {

//////////////////////////////////////////////////////////////////////////
// input [bS, iC, iH, iW] is convoluted to output [bS, iC, kH, kW, oH, oW]
template <typename T>
SD_KERNEL static void im2colCuda(const void *image, void *columns, const sd::LongType *imShapeInfo,
                                 const sd::LongType *colShapeInfo, const LongType sH, const LongType sW, const LongType pH,
                                 const LongType pW, const LongType dH, const LongType dW, const double zeroPadValD) {
  T zeroPadVal = static_cast<T>(zeroPadValD);  // Value to use when value is padding. Usually 0 but not always
  const auto im = reinterpret_cast<const T *>(image);
  auto col = reinterpret_cast<T *>(columns);

  __shared__ sd::LongType colLen, iH, iW;
  __shared__ sd::LongType imRank, colRank, *sharedMem;

  if (threadIdx.x == 0) {
    extern __shared__ unsigned char shmem[];
    sharedMem = reinterpret_cast<sd::LongType *>(shmem);

    colRank = 6;
    imRank = 4;

    colLen = shape::length(colShapeInfo);

    iH = imShapeInfo[3];
    iW = imShapeInfo[4];
  }
  __syncthreads();

  const auto colInd = threadIdx.x + blockIdx.x * blockDim.x;

  if (colInd >= colLen) return;

  auto coords = sharedMem + threadIdx.x * colRank;

  shape::index2coords(colInd, colShapeInfo, coords);

  const auto colOffset = shape::getOffset(colShapeInfo, coords);

  coords[2] = (-pH + coords[2] * dH) + coords[4] * sH;  // imH
  coords[3] = (-pW + coords[3] * dW) + coords[5] * sW;  // imW


  if (static_cast<sd::LongType>(coords[2]) >= static_cast<sd::LongType>(iH) ||
      static_cast<sd::LongType>(coords[3]) >= static_cast<sd::LongType>(iW) ||
      coords[2] < 0 || coords[3] < 0)
    col[colOffset] = zeroPadVal;
  else
    col[colOffset] = im[shape::getOffset(imShapeInfo, coords)];
}

//////////////////////////////////////////////////////////////////////////
template <typename T>
static void im2colCudaLauncher(const int blocksPerGrid, const int threadsPerBlock, sd::LaunchContext &context,
                               const void *image, void *columns, const sd::LongType *imShapeInfo,
                               const sd::LongType *colShapeInfo, LongType sH, LongType sW, LongType pH, LongType pW, LongType dH, LongType dW,
                               double zeroPadVal) {
  im2colCuda<T>
  <<<blocksPerGrid, threadsPerBlock, threadsPerBlock * sizeof(sd::LongType) * 6 /* rank of columns = 6 */,
  *context.getCudaStream()>>>(image, columns, imShapeInfo, colShapeInfo, sH, sW, pH, pW, dH, dW, zeroPadVal);
}

//////////////////////////////////////////////////////////////////////////
void im2col(sd::LaunchContext &context, const NDArray &image, NDArray &columns, const LongType kH, const LongType kW,
            const LongType sH, const LongType sW, const LongType pH, const LongType pW, const LongType dH, const LongType dW,
            const NDArray &arrZeroPadVal) {
  PointersManager manager(&context, "im2col");

  const int threadsPerBlock = 512;
  const int blocksPerGrid = (columns.lengthOf() + threadsPerBlock - 1) / threadsPerBlock;

  NDArray::prepareSpecialUse({&columns}, {&image});
  BUILD_SINGLE_SELECTOR(
      columns.dataType(), im2colCudaLauncher,
      (blocksPerGrid, threadsPerBlock, context, image.specialBuffer(), columns.specialBuffer(),
          image.specialShapeInfo(), columns.specialShapeInfo(), sH, sW, pH, pW, dH, dW, arrZeroPadVal.e<double>(0)),
      SD_FLOAT_TYPES);
  NDArray::registerSpecialUse({&columns}, {&image});

  manager.synchronize();
}

}  // namespace helpers
}  // namespace ops
}  // namespace sd
