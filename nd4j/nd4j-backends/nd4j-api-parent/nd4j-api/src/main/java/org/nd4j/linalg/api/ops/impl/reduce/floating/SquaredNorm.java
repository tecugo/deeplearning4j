/*
 *  ******************************************************************************
 *  *
 *  *
 *  * This program and the accompanying materials are made available under the
 *  * terms of the Apache License, Version 2.0 which is available at
 *  * https://www.apache.org/licenses/LICENSE-2.0.
 *  *
 *  *  See the NOTICE file distributed with this work for additional
 *  *  information regarding copyright ownership.
 *  * Unless required by applicable law or agreed to in writing, software
 *  * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 *  * License for the specific language governing permissions and limitations
 *  * under the License.
 *  *
 *  * SPDX-License-Identifier: Apache-2.0
 *  *****************************************************************************
 */

package org.nd4j.linalg.api.ops.impl.reduce.floating;

import org.nd4j.autodiff.samediff.SDVariable;
import org.nd4j.autodiff.samediff.SameDiff;
import org.nd4j.imports.NoOpNameFoundException;
import org.nd4j.linalg.api.ndarray.INDArray;
import org.nd4j.linalg.api.ops.BaseReduceFloatOp;
import org.nd4j.linalg.api.ops.impl.reduce.bp.SquaredNormBp;

import java.util.List;


public class SquaredNorm extends BaseReduceFloatOp {
    public SquaredNorm(SameDiff sameDiff, SDVariable input, boolean keepDims, long... dimensions) {
        super(sameDiff, input, keepDims, dimensions);
    }

    public SquaredNorm(SameDiff sameDiff, SDVariable i_v, SDVariable i_v2, long[] dimensions) {
        super(sameDiff, i_v, i_v2, dimensions);
    }

    public SquaredNorm(SameDiff sameDiff, SDVariable input, long[] dimensions, boolean keepDims) {
        super(sameDiff, input, dimensions, keepDims);
    }

    public SquaredNorm(SameDiff sameDiff, SDVariable input, long... dimensions) {
        super(sameDiff, input, dimensions);
    }

    public SquaredNorm(SameDiff sameDiff, SDVariable i_v, boolean keepDims, SDVariable dimensions) {
        super(sameDiff, i_v, keepDims, dimensions);
    }

    public SquaredNorm(SameDiff sameDiff, SDVariable i_v, SDVariable i_v2, SDVariable dimensions) {
        super(sameDiff, i_v, i_v2, dimensions);
    }

    public SquaredNorm(SameDiff sameDiff, SDVariable input, SDVariable dimensions, boolean keepDims) {
        super(sameDiff, input, dimensions, keepDims);
    }

    public SquaredNorm(SameDiff sameDiff, SDVariable input, SDVariable dimensions) {
        super(sameDiff, input, dimensions);
    }

    public SquaredNorm(INDArray input, INDArray output, boolean keepDims, long... dimensions){
        super(input, output, keepDims, dimensions);
    }

    public SquaredNorm(INDArray x, INDArray y, INDArray z, long... dimensions) {
        super(x, y, z, dimensions);
    }

    public SquaredNorm(INDArray x, INDArray z, long... dimensions) {
        super(x, z, dimensions);
    }

    public SquaredNorm(INDArray input, boolean keepDims, long... dimensions){
        this(input, null, keepDims, dimensions);
    }

    public SquaredNorm(){}

    public SquaredNorm(INDArray x, long... dimensions){
        super(x,  dimensions);
    }

    public SquaredNorm(INDArray x, INDArray y, INDArray z, boolean keepDims, long... dimensions) {
        super(x, y, z, keepDims, dimensions);
    }

    public SquaredNorm(INDArray in, long[] dimensions, boolean keepDims) {
        super(in,keepDims,dimensions);
    }

    @Override
    public int opNum() {
        return 7;
    }

    @Override
    public String opName() {
        return "reduce_sqnorm";
    }

    @Override
    public String onnxName() {
        throw new NoOpNameFoundException("No Onnx op found for" + getClass().getName());
    }

    @Override
    public String tensorflowName() {
        throw new NoOpNameFoundException("No Tensorflow op found for" + getClass().getName());
    }

    @Override
    public List<SDVariable> doDiff(List<SDVariable> grad){
        return new SquaredNormBp(sameDiff, arg(), grad.get(0), keepDims, dimensions).outputs();
    }
}
