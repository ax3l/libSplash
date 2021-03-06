/**
 * Copyright 2013-2015 Felix Schmitt, Axel Huebl
 *
 * This file is part of libSplash.
 *
 * libSplash is free software: you can redistribute it and/or modify
 * it under the terms of of either the GNU General Public License or
 * the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libSplash is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License and the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * and the GNU Lesser General Public License along with libSplash.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include "Parallel_AttributesTest.h"

#include <mpi.h>
#include <time.h>
#include <iostream>
#include <stdlib.h>
#include <cstring>
#include <cppunit/TestAssert.h>

CPPUNIT_TEST_SUITE_REGISTRATION(Parallel_AttributesTest);

using namespace splash;

#define BUF_SIZE 32
#define TEST_FILE "h5/attributesParallel"
#define TEST_FILE2 "h5/attributes_arrayParallel"

#define MPI_SIZE_X 2
#define MPI_SIZE_Y 2

Parallel_AttributesTest::Parallel_AttributesTest() :
  ctString4(4)
{
    srand(time(NULL));

    int initialized;
    MPI_Initialized(&initialized);
    if( !initialized )
        MPI_Init(NULL, NULL);

    int mpiSize;
    MPI_Comm_rank(MPI_COMM_WORLD, &mpiRank);
    MPI_Comm_size(MPI_COMM_WORLD, &mpiSize);

    CPPUNIT_ASSERT(mpiSize == (MPI_SIZE_X * MPI_SIZE_Y));
}

Parallel_AttributesTest::~Parallel_AttributesTest()
{
    int finalized;
    MPI_Finalized(&finalized);
    if (!finalized)
        MPI_Finalize();
}

void Parallel_AttributesTest::testDataAttributes()
{
    DataCollector::FileCreationAttr attr;
    DataCollector::initFileCreationAttr(attr);

    IParallelDataCollector *dataCollector = new ParallelDataCollector(
            MPI_COMM_WORLD, MPI_INFO_NULL, Dimensions(MPI_SIZE_X, MPI_SIZE_Y, 1), 1);
    dataCollector->open(TEST_FILE, attr);

    int *dummy_data = new int[BUF_SIZE * 2];
    int sum = 0;
    for (int i = 0; i < BUF_SIZE; i++)
    {
        int val_x = rand() % 10;
        int val_y = rand() % 10;

        sum += val_x * val_y;

        dummy_data[2 * i] = val_x;
        dummy_data[2 * i + 1] = val_y;
    }

    dataCollector->write(0, ctInt2, 1, Selection(Dimensions(BUF_SIZE, 1, 1)),
            "attr/attr2/attr3/data", dummy_data);

    dataCollector->writeAttribute(0, ctInt, "attr/attr2/attr3/data", "sum", &sum);
    int neg_sum = -sum;
    dataCollector->writeAttribute(0, ctInt, "attr/attr2/attr3/data", "neg_sum", &neg_sum);

    dataCollector->writeAttribute(0, ctInt, "attr/attr2/attr3", "sum_at_group", &sum);
    char c = 'Y';
    dataCollector->writeAttribute(0, ctChar, "attr/attr2/attr3", "my_char", &c);

    /* variable length string, '\0' terminated */
    const char *string_attr = {"My first c-string."};
    dataCollector->writeAttribute(0, ctString, NULL, "my_string", &string_attr);
    /* fixed length string, '\0' terminated */
    const char string_attr4[5] = {"ABCD"};
    dataCollector->writeAttribute(0, ctString4, NULL, "my_string4", &string_attr4);

    delete[] dummy_data;
    dummy_data = NULL;

    dataCollector->close();

    dummy_data = new int[BUF_SIZE * 2];
    int old_sum = sum;
    sum = 0;
    for (int i = 0; i < BUF_SIZE * 2; i++)
        dummy_data[i] = 0;

    attr.fileAccType = DataCollector::FAT_READ;

    dataCollector->open(TEST_FILE, attr);

    sum = 0;
    neg_sum = 0;
    c = 'A';
    dataCollector->readAttribute(0, "attr/attr2/attr3/data", "sum", &sum);
    dataCollector->readAttribute(0, "attr/attr2/attr3/data", "neg_sum", &neg_sum);

    CPPUNIT_ASSERT_THROW(dataCollector->readAttribute(0, "data", "sum", &sum), DCException);

    CPPUNIT_ASSERT(sum == old_sum);
    CPPUNIT_ASSERT(neg_sum == -old_sum);

    dataCollector->readAttribute(0, "attr/attr2/attr3", "sum_at_group", &sum);
    dataCollector->readAttribute(0, "attr/attr2/attr3", "my_char", &c);

    CPPUNIT_ASSERT(sum == old_sum);
    CPPUNIT_ASSERT(c == 'Y');

    char* string_read;
    dataCollector->readAttribute(0, NULL, "my_string", &string_read);
    char string_read4[5];
    dataCollector->readAttribute(0, NULL, "my_string4", &string_read4);

    CPPUNIT_ASSERT(strcmp(string_read, string_attr) == 0);
    CPPUNIT_ASSERT(strcmp(string_read4, string_attr4) == 0);

    dataCollector->finalize();
    dataCollector->close();
    delete dataCollector;
}

void Parallel_AttributesTest::testArrayTypes()
{
    DataCollector::FileCreationAttr attr;
    DataCollector::initFileCreationAttr(attr);

    const int array_data_write[3] = {17, 12, -99};
    Dimensions dim_write(104, 0, 2);

    IParallelDataCollector *dataCollector = new ParallelDataCollector(
            MPI_COMM_WORLD, MPI_INFO_NULL, Dimensions(MPI_SIZE_X, MPI_SIZE_Y, 1), 1);

    dataCollector->open(TEST_FILE2, attr);
    dataCollector->writeGlobalAttribute(10, ctInt3Array, "testpositionArray", array_data_write);
    dataCollector->writeGlobalAttribute(10, ctInt, "testposition", 1u, Dimensions(3, 1, 1), array_data_write);
    dataCollector->writeGlobalAttribute(20, ctDimArray, "testdim", dim_write.getPointer());
    dataCollector->close();

    int array_data_read[3] = {0, 0, 0};
    int data_read[3] = {0, 0, 0};
    Dimensions dim_read;

    attr.fileAccType = DataCollector::FAT_READ;
    dataCollector->open(TEST_FILE2, attr);
    dataCollector->readGlobalAttribute(10, "testpositionArray", array_data_read);
    dataCollector->readGlobalAttribute(10, "testposition", data_read);
    dataCollector->readGlobalAttribute(20, "testdim", dim_read.getPointer());
    dataCollector->close();

    for (int i = 0; i < 3; i++)
    {
        CPPUNIT_ASSERT(array_data_read[i] == array_data_write[i]);
        CPPUNIT_ASSERT(data_read[i] == array_data_write[i]);
        CPPUNIT_ASSERT(dim_write[i] == dim_read[i]);
    }

    delete dataCollector;
}
