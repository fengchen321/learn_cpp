#include "gtest_prompt.h"
#include "storage.h"
#include "symbol_table.h"

#include <cmath>

TEST(SymbolTableTest, AddReturnsExistingIdForDuplicateName) {
    SymbolTable table;

    const unsigned int first = table.add("value");
    const unsigned int second = table.add("value");

    EXPECT_EQ(first, second);
    EXPECT_EQ(table.find("value"), first);
    EXPECT_EQ(table.getSymbolName(first), "value");
}

TEST(SymbolTableTest, FindReturnsInvalidIdForMissingName) {
    SymbolTable table;
    EXPECT_EQ(table.find("missing"), SymbolTable::kInvalidSymbolId);
}

TEST(StorageTest, SeedsBuiltinConstants) {
    SymbolTable table;
    Storage storage(table);

    const unsigned int piId = table.find("pi");
    const unsigned int eId = table.find("e");

    ASSERT_NE(piId, SymbolTable::kInvalidSymbolId);
    ASSERT_NE(eId, SymbolTable::kInvalidSymbolId);
    EXPECT_TRUE(storage.isInit(piId));
    EXPECT_TRUE(storage.isInit(eId));
    EXPECT_NEAR(storage.getValue(piId), 2.0 * std::acos(0.0), 1e-12);
    EXPECT_NEAR(storage.getValue(eId), std::exp(1.0), 1e-12);
}

TEST(StorageTest, SetValueInitializesSymbolSlot) {
    SymbolTable table;
    Storage storage(table);

    const unsigned int valueId = table.add("value");
    storage.setValue(valueId, 42.5);

    EXPECT_TRUE(storage.isInit(valueId));
    EXPECT_DOUBLE_EQ(storage.getValue(valueId), 42.5);
}

TEST(StorageTest, UpdatingLowerIdPreservesHigherIdValue) {
    SymbolTable table;
    Storage storage(table);

    const unsigned int xId = table.add("x");
    const unsigned int yId = table.add("y");

    storage.setValue(yId, 9.0);
    storage.setValue(xId, 5.0);

    EXPECT_TRUE(storage.isInit(xId));
    EXPECT_TRUE(storage.isInit(yId));
    EXPECT_DOUBLE_EQ(storage.getValue(xId), 5.0);
    EXPECT_DOUBLE_EQ(storage.getValue(yId), 9.0);
}

TEST(StorageTest, GetValueThrowsForUninitializedSymbol) {
    SymbolTable table;
    Storage storage(table);

    const unsigned int valueId = table.add("value");
    EXPECT_FALSE(storage.isInit(valueId));
    EXPECT_THROW(storage.getValue(valueId), std::runtime_error);
}

TEST(StorageTest, SetValueRejectsInvalidSymbolId) {
    SymbolTable table;
    Storage storage(table);

    EXPECT_THROW(storage.setValue(SymbolTable::kInvalidSymbolId, 1.0), std::out_of_range);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
