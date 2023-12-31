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

package org.nd4j.linalg.api.ops;

import org.nd4j.linalg.api.ndarray.INDArray;

public interface ScalarOp extends Op {

    /**The normal scalar
     *@return the scalar
     */
    INDArray scalar();

    /**
     * This method allows to set scalar
     * @param scalar
     */
    void setScalar(Number scalar);

    void setScalar(INDArray scalar);

    /**
     * This method returns target dimensions for this op
     * @return
     */
    INDArray dimensions();

    long[] getDimension();

    void setDimension(long... dimension);

    boolean validateDataTypes(boolean experimentalMode);

    Type getOpType();
}
