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

package org.datavec.local.transforms.transform.join;


import org.datavec.api.transform.ColumnType;
import org.datavec.api.transform.join.Join;
import org.datavec.api.transform.schema.Schema;
import org.datavec.api.writable.*;
import org.datavec.local.transforms.LocalTransformExecutor;
import org.junit.jupiter.api.Tag;
import org.junit.jupiter.api.Test;
import org.nd4j.common.tests.tags.TagNames;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

import static org.junit.jupiter.api.Assertions.assertEquals;
@Tag(TagNames.FILE_IO)
@Tag(TagNames.JAVA_ONLY)
public class TestJoin  {

    @Test
    public void testJoinOneToMany_ManyToOne() {

        Schema customerInfoSchema =
                        new Schema.Builder().addColumnLong("customerID").addColumnString("customerName").build();

        Schema purchasesSchema = new Schema.Builder().addColumnLong("purchaseID").addColumnLong("customerID")
                        .addColumnDouble("amount").build();

        List<List<Writable>> infoList = new ArrayList<>();
        infoList.add(Arrays.asList(new LongWritable(12345), new Text("Customer12345")));
        infoList.add(Arrays.asList(new LongWritable(98765), new Text("Customer98765")));
        infoList.add(Arrays.asList(new LongWritable(50000), new Text("Customer50000")));

        List<List<Writable>> purchaseList = new ArrayList<>();
        purchaseList.add(Arrays.asList(new LongWritable(1000000), new LongWritable(12345),
                        new DoubleWritable(10.00)));
        purchaseList.add(Arrays.asList(new LongWritable(1000001), new LongWritable(12345),
                        new DoubleWritable(20.00)));
        purchaseList.add(Arrays.asList(new LongWritable(1000002), new LongWritable(98765),
                        new DoubleWritable(30.00)));

        Join join = new Join.Builder(Join.JoinType.RightOuter).setJoinColumns("customerID")
                        .setSchemas(customerInfoSchema, purchasesSchema).build();

        List<List<Writable>> expected = new ArrayList<>();
        expected.add(Arrays.asList(new LongWritable(12345), new Text("Customer12345"),
                        new LongWritable(1000000), new DoubleWritable(10.00)));
        expected.add(Arrays.asList(new LongWritable(12345), new Text("Customer12345"),
                        new LongWritable(1000001), new DoubleWritable(20.00)));
        expected.add(Arrays.asList(new LongWritable(98765), new Text("Customer98765"),
                        new LongWritable(1000002), new DoubleWritable(30.00)));



        List<List<Writable>> info = (infoList);
        List<List<Writable>> purchases = (purchaseList);

        List<List<Writable>> joined = LocalTransformExecutor.executeJoin(join, info, purchases);
        List<List<Writable>> joinedList = new ArrayList<>(joined);
        //Sort by order ID (column 3, index 2)
        Collections.sort(joinedList, (o1, o2) -> Long.compare(o1.get(2).toLong(), o2.get(2).toLong()));
        assertEquals(expected, joinedList);

        assertEquals(3, joinedList.size());

        List<String> expectedColNames = Arrays.asList("customerID", "customerName", "purchaseID", "amount");
        assertEquals(expectedColNames, join.getOutputSchema().getColumnNames());

        List<ColumnType> expectedColTypes =
                        Arrays.asList(ColumnType.Long, ColumnType.String, ColumnType.Long, ColumnType.Double);
        assertEquals(expectedColTypes, join.getOutputSchema().getColumnTypes());


        //Test Many to one: same thing, but swap the order...
        Join join2 = new Join.Builder(Join.JoinType.LeftOuter).setJoinColumns("customerID")
                        .setSchemas(purchasesSchema, customerInfoSchema).build();

        List<List<Writable>> expectedManyToOne = new ArrayList<>();
        expectedManyToOne.add(Arrays.<Writable>asList(new LongWritable(1000000), new LongWritable(12345),
                        new DoubleWritable(10.00), new Text("Customer12345")));
        expectedManyToOne.add(Arrays.<Writable>asList(new LongWritable(1000001), new LongWritable(12345),
                        new DoubleWritable(20.00), new Text("Customer12345")));
        expectedManyToOne.add(Arrays.<Writable>asList(new LongWritable(1000002), new LongWritable(98765),
                        new DoubleWritable(30.00), new Text("Customer98765")));

        List<List<Writable>> joined2 = LocalTransformExecutor.executeJoin(join2, purchases, info);
        List<List<Writable>> joinedList2 = new ArrayList<>(joined2);
        //Sort by order ID (column 0)
        Collections.sort(joinedList2, (o1, o2) -> Long.compare(o1.get(0).toLong(), o2.get(0).toLong()));
        assertEquals(3, joinedList2.size());

        assertEquals(expectedManyToOne, joinedList2);

        List<String> expectedColNames2 = Arrays.asList("purchaseID", "customerID", "amount", "customerName");
        assertEquals(expectedColNames2, join2.getOutputSchema().getColumnNames());

        List<ColumnType> expectedColTypes2 =
                        Arrays.asList(ColumnType.Long, ColumnType.Long, ColumnType.Double, ColumnType.String);
        assertEquals(expectedColTypes2, join2.getOutputSchema().getColumnTypes());
    }


    @Test
    public void testJoinManyToMany() {
        Schema schema1 = new Schema.Builder().addColumnLong("id")
                        .addColumnCategorical("category", Arrays.asList("cat0", "cat1", "cat2")).build();

        Schema schema2 = new Schema.Builder().addColumnLong("otherId")
                        .addColumnCategorical("otherCategory", Arrays.asList("cat0", "cat1", "cat2")).build();

        List<List<Writable>> first = new ArrayList<>();
        first.add(Arrays.<Writable>asList(new LongWritable(0), new Text("cat0")));
        first.add(Arrays.<Writable>asList(new LongWritable(1), new Text("cat0")));
        first.add(Arrays.<Writable>asList(new LongWritable(2), new Text("cat1")));

        List<List<Writable>> second = new ArrayList<>();
        second.add(Arrays.<Writable>asList(new LongWritable(100), new Text("cat0")));
        second.add(Arrays.<Writable>asList(new LongWritable(101), new Text("cat0")));
        second.add(Arrays.<Writable>asList(new LongWritable(102), new Text("cat2")));



        List<List<Writable>> expOuterJoin = new ArrayList<>();
        expOuterJoin.add(Arrays.<Writable>asList(new LongWritable(0), new Text("cat0"), new LongWritable(100)));
        expOuterJoin.add(Arrays.<Writable>asList(new LongWritable(0), new Text("cat0"), new LongWritable(101)));
        expOuterJoin.add(Arrays.<Writable>asList(new LongWritable(1), new Text("cat0"), new LongWritable(100)));
        expOuterJoin.add(Arrays.<Writable>asList(new LongWritable(1), new Text("cat0"), new LongWritable(101)));
        expOuterJoin.add(Arrays.<Writable>asList(new LongWritable(2), new Text("cat1"), new NullWritable()));
        expOuterJoin.add(Arrays.<Writable>asList(new NullWritable(), new Text("cat2"), new LongWritable(102)));

        List<List<Writable>> expLeftJoin = new ArrayList<>();
        expLeftJoin.add(Arrays.<Writable>asList(new LongWritable(0), new Text("cat0"), new LongWritable(100)));
        expLeftJoin.add(Arrays.<Writable>asList(new LongWritable(0), new Text("cat0"), new LongWritable(101)));
        expLeftJoin.add(Arrays.<Writable>asList(new LongWritable(1), new Text("cat0"), new LongWritable(100)));
        expLeftJoin.add(Arrays.<Writable>asList(new LongWritable(1), new Text("cat0"), new LongWritable(101)));
        expLeftJoin.add(Arrays.<Writable>asList(new LongWritable(2), new Text("cat1"), new NullWritable()));


        List<List<Writable>> expRightJoin = new ArrayList<>();
        expRightJoin.add(Arrays.<Writable>asList(new LongWritable(0), new Text("cat0"), new LongWritable(100)));
        expRightJoin.add(Arrays.<Writable>asList(new LongWritable(0), new Text("cat0"), new LongWritable(101)));
        expRightJoin.add(Arrays.<Writable>asList(new LongWritable(1), new Text("cat0"), new LongWritable(100)));
        expRightJoin.add(Arrays.<Writable>asList(new LongWritable(1), new Text("cat0"), new LongWritable(101)));
        expRightJoin.add(Arrays.<Writable>asList(new NullWritable(), new Text("cat2"), new LongWritable(102)));

        List<List<Writable>> expInnerJoin = new ArrayList<>();
        expInnerJoin.add(Arrays.<Writable>asList(new LongWritable(0), new Text("cat0"), new LongWritable(100)));
        expInnerJoin.add(Arrays.<Writable>asList(new LongWritable(0), new Text("cat0"), new LongWritable(101)));
        expInnerJoin.add(Arrays.<Writable>asList(new LongWritable(1), new Text("cat0"), new LongWritable(100)));
        expInnerJoin.add(Arrays.<Writable>asList(new LongWritable(1), new Text("cat0"), new LongWritable(101)));

        List<List<Writable>> firstRDD = (first);
        List<List<Writable>> secondRDD = (second);

        int count = 0;
        for (Join.JoinType jt : Join.JoinType.values()) {
            Join join = new Join.Builder(jt).setJoinColumnsLeft("category").setJoinColumnsRight("otherCategory")
                            .setSchemas(schema1, schema2).build();
            List<List<Writable>> out =
                            new ArrayList<>(LocalTransformExecutor.executeJoin(join, firstRDD, secondRDD));

            //Sort output by column 0, then column 1, then column 2 for comparison to expected...
            Collections.sort(out, (o1, o2) -> {
                Writable w1 = o1.get(0);
                Writable w2 = o2.get(0);
                if (w1 instanceof NullWritable)
                    return 1;
                else if (w2 instanceof NullWritable)
                    return -1;
                int c = Long.compare(w1.toLong(), w2.toLong());
                if (c != 0)
                    return c;
                c = o1.get(1).toString().compareTo(o2.get(1).toString());
                if (c != 0)
                    return c;
                w1 = o1.get(2);
                w2 = o2.get(2);
                if (w1 instanceof NullWritable)
                    return 1;
                else if (w2 instanceof NullWritable)
                    return -1;
                return Long.compare(w1.toLong(), w2.toLong());
            });

            switch (jt) {
                case Inner:
                    assertEquals(expInnerJoin, out);
                    break;
                case LeftOuter:
                    assertEquals(expLeftJoin, out);
                    break;
                case RightOuter:
                    assertEquals(expRightJoin, out);
                    break;
                case FullOuter:
                    assertEquals(expOuterJoin, out);
                    break;
            }
            count++;
        }

        assertEquals(4, count);
    }

}
