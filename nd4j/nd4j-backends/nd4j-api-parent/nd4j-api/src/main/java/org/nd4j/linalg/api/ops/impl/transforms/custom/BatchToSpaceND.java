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


import lombok.val;
import org.nd4j.autodiff.samediff.SDVariable;
import org.nd4j.autodiff.samediff.SameDiff;
import org.nd4j.linalg.api.buffer.DataType;
import org.nd4j.linalg.api.ndarray.INDArray;
import org.nd4j.linalg.api.ops.DynamicCustomOp;
import org.nd4j.shade.guava.primitives.Ints;

import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Map;

public class BatchToSpaceND extends DynamicCustomOp {

    private int[] blocks;
    private int[][] crops;

    public BatchToSpaceND() {
    }

    public BatchToSpaceND(SameDiff sameDiff, SDVariable[] args, int[] blocks, int[][] crops, boolean inPlace) {
        super(null, sameDiff, args, inPlace);

        this.blocks = blocks;
        this.crops = crops;

        for (val b : blocks)
            addIArgument(b);

        for (int e = 0; e < crops.length; e++)
            addIArgument(crops[e][0], crops[e][1]);
    }

    @Override
    public String opName() {
        return "batch_to_space_nd";
    }

    @Override
    public String onnxName() {
        return "batch_to_space_nd";
    }

    @Override
    public String tensorflowName() {
        return "BatchToSpaceND";
    }


    @Override
    public void configureFromArguments() {
        SDVariable[] args = args();
        if(args != null && args.length > 1) {
            INDArray blocks = args[1].getArr();
            if(blocks != null) {
                this.blocks = blocks.toIntVector();
            }
            if(args.length > 2) {
                INDArray crops = args[2].getArr();
                if(crops != null)
                   this.crops = crops.toIntMatrix();
            }

        }
    }

    @Override
    public void setPropertiesForFunction(Map<String, Object> properties) {
        super.setPropertiesForFunction(properties);
    }

    @Override
    public List<SDVariable> doDiff(List<SDVariable> i_v) {
        // Inverse of batch to space is space to batch with same blocks and padding as crops
        SDVariable gradient = sameDiff.setupFunction(i_v.get(0));
        return Arrays.asList(sameDiff.cnn().spaceToBatch(gradient, blocks, crops[0], crops[1]));
    }

    @Override
    public List<DataType> calculateOutputDataTypes(List<DataType> dataTypes){
        return Collections.singletonList(dataTypes.get(0));
    }
}
