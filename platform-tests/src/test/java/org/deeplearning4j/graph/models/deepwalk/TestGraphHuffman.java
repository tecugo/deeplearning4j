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

package org.deeplearning4j.graph.models.deepwalk;

import org.deeplearning4j.BaseDL4JTest;
import org.junit.jupiter.api.Tag;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.Timeout;
import org.nd4j.common.tests.tags.NativeTag;
import org.nd4j.common.tests.tags.TagNames;

import java.util.Arrays;
import java.util.HashSet;
import java.util.Set;

import static org.junit.jupiter.api.Assertions.*;

@NativeTag
@Tag(TagNames.FILE_IO)
public class TestGraphHuffman extends BaseDL4JTest {

    @Test()
    @Timeout(10000)
    public void testGraphHuffman() {
        //Simple test case from Weiss - Data Structires and Algorithm Analysis in Java 3ed pg436
        //Huffman code is non-unique, but length of code for each node is same for all Huffman codes

        GraphHuffman gh = new GraphHuffman(7);

        int[] vertexDegrees = {10, 15, 12, 3, 4, 13, 1};

        gh.buildTree(vertexDegrees);

        for (int i = 0; i < 7; i++) {
            String s = i + "\t" + gh.getCodeLength(i) + "\t" + gh.getCodeString(i) + "\t\t" + gh.getCode(i)
                    + "\t\t" + Arrays.toString(gh.getPathInnerNodes(i));
//            System.out.println(s);
        }

        int[] expectedLengths = {3, 2, 2, 5, 4, 2, 5};
        for (int i = 0; i < vertexDegrees.length; i++) {
            assertEquals(expectedLengths[i], gh.getCodeLength(i));
        }

        //Check that codes are actually unique:
        Set<String> codeSet = new HashSet<>();
        for (int i = 0; i < 7; i++) {
            String code = gh.getCodeString(i);
            assertFalse(codeSet.contains(code));
            codeSet.add(code);
        }

        //Furthermore, Huffman code is a prefix code: i.e., no code word is a prefix of any other code word
        //Check all pairs of codes to ensure this holds
        for (int i = 0; i < 7; i++) {
            String code = gh.getCodeString(i);
            for (int j = i + 1; j < 7; j++) {
                String codeOther = gh.getCodeString(j);

                if (code.length() == codeOther.length()) {
                    assertNotEquals(code, codeOther);
                } else if (code.length() < codeOther.length()) {
                    assertNotEquals(code, codeOther.substring(0, code.length()));
                } else {
                    assertNotEquals(codeOther, code.substring(0, codeOther.length()));
                }
            }
        }
    }
}
