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

package org.nd4j.linalg.api.ops.impl.transforms.custom;

import lombok.NoArgsConstructor;
import org.nd4j.autodiff.samediff.SDVariable;
import org.nd4j.autodiff.samediff.SameDiff;
import org.nd4j.common.base.Preconditions;
import org.nd4j.imports.NoOpNameFoundException;
import org.nd4j.linalg.api.buffer.DataType;
import org.nd4j.linalg.api.ndarray.INDArray;
import org.nd4j.linalg.api.ops.DynamicCustomOp;

import java.util.Arrays;
import java.util.Collections;
import java.util.List;


@NoArgsConstructor
public class XwPlusBBp extends DynamicCustomOp {


    private boolean aTranpose,bTranspose;


    public XwPlusBBp(SameDiff sameDiff, SDVariable input, SDVariable weights, SDVariable bias,SDVariable dldX, boolean transposeA, boolean transposeB) {
        super(null, sameDiff, new SDVariable[] {input, weights, bias,dldX}, false);
        addIArgument(transposeA ? 1 : 0, transposeB ? 1 : 0);
        this.aTranpose = transposeA;
        this.bTranspose = transposeB;
    }

    public XwPlusBBp(INDArray input, INDArray weights, INDArray bias) {
        super(new INDArray[] {input, weights, bias}, null);
    }

    public XwPlusBBp(INDArray[] inputs, INDArray output){
        super(inputs, wrapOrNull(output));
    }



    @Override
    public String opName() {
        return "xw_plus_b_bp";
    }


    @Override
    public String tensorflowName() {
        throw new NoOpNameFoundException("No tensorflow name found for shape " + opName());
    }

    @Override
    public String onnxName() {
        throw new NoOpNameFoundException("No onnx name found for shape " + opName());
    }

    @Override
    public List<SDVariable> doDiff(List<SDVariable> gradient) {
        throw new UnsupportedOperationException("Unable to take gradient of a back prop op");
    }


    @Override
    public List<DataType> calculateOutputDataTypes(List<DataType> dataTypes) {
        DataType first = dataTypes.get(0);
        for( int i = 0; i < 4; i++) {
            Preconditions.checkState(dataTypes.get(i).isFPType(), "Input %s datatype must be a floating point type, got datypes %s", dataTypes);
            if(i > 0){
                Preconditions.checkState(first == dataTypes.get(i), "All datatypes must be same type, got input datatypes %s", dataTypes);
            }
        }

        return dataTypes.subList(0,3);
    }

}
