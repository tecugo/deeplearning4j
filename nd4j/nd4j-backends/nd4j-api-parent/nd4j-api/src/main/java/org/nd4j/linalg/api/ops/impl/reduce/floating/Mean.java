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
import org.nd4j.linalg.api.ndarray.INDArray;
import org.nd4j.linalg.api.ops.BaseReduceFloatOp;
import org.nd4j.linalg.api.ops.impl.reduce.bp.MeanBp;

import java.util.List;

public class Mean extends BaseReduceFloatOp {
    public Mean(SameDiff sameDiff, SDVariable i_v, boolean keepDims, long[] dimensions) {
        super(sameDiff, i_v, keepDims, dimensions);
    }

    public Mean(SameDiff sameDiff, SDVariable i_v, SDVariable i_v2, long[] dimensions) {
        super(sameDiff, i_v, i_v2, dimensions);
    }

    public Mean(SameDiff sameDiff, SDVariable input, long[] dimensions, boolean keepDims) {
        super(sameDiff, input, dimensions, keepDims);
    }

    public Mean(SameDiff sameDiff, SDVariable input, long... dimensions) {
        super(sameDiff, input, dimensions);
    }

    public Mean(SameDiff sameDiff, SDVariable i_v, boolean keepDims, SDVariable dimensions) {
        super(sameDiff, i_v, keepDims, dimensions);
    }

    public Mean(SameDiff sameDiff, SDVariable i_v, SDVariable i_v2, SDVariable dimensions) {
        super(sameDiff, i_v, i_v2, dimensions);
    }

    public Mean(SameDiff sameDiff, SDVariable input, SDVariable dimensions, boolean keepDims) {
        super(sameDiff, input, dimensions, keepDims);
    }

    public Mean(SameDiff sameDiff, SDVariable input, SDVariable dimensions) {
        super(sameDiff, input, dimensions);
    }

    public Mean() {
    }

    public Mean(INDArray x, INDArray z, long... dimensions) {
        super(x, null, z, dimensions);
    }

    public Mean(INDArray x, long... dimensions) {
        super(x, dimensions);
    }

    public Mean(INDArray x, boolean keepDims, long... dimensions) {
        super(x, keepDims, dimensions);
    }

    public Mean(INDArray x, INDArray z, boolean keepDims, long... dimensions) {
        super(x, z, keepDims, dimensions);
    }

    public Mean(INDArray x, INDArray y, INDArray z, long... dimensions) {
        super(x, y, z, dimensions);
    }

    public Mean(INDArray x, INDArray y, INDArray z, boolean keepDims, long... dimensions) {
        super(x, y, z, keepDims, dimensions);
    }

    public Mean(INDArray in, long[] dimensions, boolean keepDims) {
        super(in,keepDims,dimensions);
    }


    @Override
    public int opNum() {
        return 0;
    }

    @Override
    public String opName() {
        return "reduce_mean";
    }

    @Override
    public List<SDVariable> doDiff(List<SDVariable> i_v1) {
        //If out = mean(in), then dL/dIn = 1/N * dL/dOut  (broadcast to appropriate shape)
        //Note that N differs for "along dimension" vs. "whole array" reduce cases
        return new MeanBp(sameDiff, arg(), i_v1.get(0), keepDims, dimensions).outputs();
    }

    @Override
    public String onnxName() {
        return "ReduceMean";
    }

    @Override
    public String tensorflowName() {
        return "Mean";
    }
}
