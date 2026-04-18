#include <cmath>
#include <cstdio>
#include <string>
#include "gtest_prompt.h"
#include "serial.h"

namespace {

std::string getTempFilePath() {
    return "/tmp/test_serial_" + std::to_string(std::rand()) + ".bin";
}

} // namespace

// Test int serialization
TEST(SerializerTest, IntRoundTrip) {
    std::string filepath = getTempFilePath();
    const int original = -12345;

    {
        Serializer ser(filepath);
        ser.put(original);
    }

    int restored = 0;
    {
        DeSerializer deser(filepath);
        deser.get(restored);
    }

    EXPECT_EQ(restored, original);
    std::remove(filepath.c_str());
}

// Test unsigned int serialization
TEST(SerializerTest, UnsignedIntRoundTrip) {
    std::string filepath = getTempFilePath();
    const unsigned int original = 4294967295U;

    {
        Serializer ser(filepath);
        ser.put(original);
    }

    unsigned int restored = 0;
    {
        DeSerializer deser(filepath);
        deser.get(restored);
    }

    EXPECT_EQ(restored, original);
    std::remove(filepath.c_str());
}

// Test long serialization
TEST(SerializerTest, LongRoundTrip) {
    std::string filepath = getTempFilePath();
    const long original = -9876543210L;

    {
        Serializer ser(filepath);
        ser.put(original);
    }

    long restored = 0;
    {
        DeSerializer deser(filepath);
        deser.get(restored);
    }

    EXPECT_EQ(restored, original);
    std::remove(filepath.c_str());
}

// Test unsigned long serialization
TEST(SerializerTest, UnsignedLongRoundTrip) {
    std::string filepath = getTempFilePath();
    const unsigned long original = 18446744073709551615UL;

    {
        Serializer ser(filepath);
        ser.put(original);
    }

    unsigned long restored = 0;
    {
        DeSerializer deser(filepath);
        deser.get(restored);
    }

    EXPECT_EQ(restored, original);
    std::remove(filepath.c_str());
}

// Test double serialization
TEST(SerializerTest, DoubleRoundTrip) {
    std::string filepath = getTempFilePath();
    const double original = 3.14159265358979;

    {
        Serializer ser(filepath);
        ser.put(original);
    }

    double restored = 0.0;
    {
        DeSerializer deser(filepath);
        deser.get(restored);
    }

    EXPECT_NEAR(restored, original, 1e-15);
    std::remove(filepath.c_str());
}

// Test bool serialization - true
TEST(SerializerTest, BoolTrueRoundTrip) {
    std::string filepath = getTempFilePath();
    const bool original = true;

    {
        Serializer ser(filepath);
        ser.putBool(original);
    }

    bool restored = false;
    {
        DeSerializer deser(filepath);
        deser.getBool(restored);
    }

    EXPECT_EQ(restored, original);
    std::remove(filepath.c_str());
}

// Test bool serialization - false
TEST(SerializerTest, BoolFalseRoundTrip) {
    std::string filepath = getTempFilePath();
    const bool original = false;

    {
        Serializer ser(filepath);
        ser.putBool(original);
    }

    bool restored = true;  // Initialize to opposite value
    {
        DeSerializer deser(filepath);
        deser.getBool(restored);
    }

    EXPECT_EQ(restored, original);
    std::remove(filepath.c_str());
}

// Test std::string serialization
TEST(SerializerTest, StringRoundTrip) {
    std::string filepath = getTempFilePath();
    const std::string original = "Hello, World!";

    {
        Serializer ser(filepath);
        ser.putString(original);
    }

    std::string restored;
    {
        DeSerializer deser(filepath);
        deser.getString(restored);
    }

    EXPECT_EQ(restored, original);
    std::remove(filepath.c_str());
}

// Test empty string
TEST(SerializerTest, EmptyStringRoundTrip) {
    std::string filepath = getTempFilePath();
    const std::string original = "";

    {
        Serializer ser(filepath);
        ser.putString(original);
    }

    std::string restored = "non-empty";
    {
        DeSerializer deser(filepath);
        deser.getString(restored);
    }

    EXPECT_EQ(restored, original);
    std::remove(filepath.c_str());
}

// Test long string
TEST(SerializerTest, LongStringRoundTrip) {
    std::string filepath = getTempFilePath();
    const std::string original(10000, 'x');

    {
        Serializer ser(filepath);
        ser.putString(original);
    }

    std::string restored;
    {
        DeSerializer deser(filepath);
        deser.getString(restored);
    }

    EXPECT_EQ(restored, original);
    std::remove(filepath.c_str());
}

// Test string with special characters
TEST(SerializerTest, StringWithSpecialCharsRoundTrip) {
    std::string filepath = getTempFilePath();
    const std::string original = "Line1\nLine2\tTab\x00Embedded";

    {
        Serializer ser(filepath);
        ser.putString(original);
    }

    std::string restored;
    {
        DeSerializer deser(filepath);
        deser.getString(restored);
    }

    EXPECT_EQ(restored, original);
    std::remove(filepath.c_str());
}

// Test multiple values
TEST(SerializerTest, MultipleValuesRoundTrip) {
    std::string filepath = getTempFilePath();
    const int intVal = 42;
    const double doubleVal = 2.71828;
    const std::string strVal = "test";
    const bool boolVal = true;

    {
        Serializer ser(filepath);
        ser << intVal << doubleVal << strVal << boolVal;
    }

    int restoredInt = 0;
    double restoredDouble = 0.0;
    std::string restoredStr;
    bool restoredBool = false;

    {
        DeSerializer deser(filepath);
        deser >> restoredInt >> restoredDouble >> restoredStr >> restoredBool;
    }

    EXPECT_EQ(restoredInt, intVal);
    EXPECT_NEAR(restoredDouble, doubleVal, 1e-10);
    EXPECT_EQ(restoredStr, strVal);
    EXPECT_EQ(restoredBool, boolVal);
    std::remove(filepath.c_str());
}

// Test operator<< and >> for all required types
TEST(SerializerTest, OperatorInt) {
    std::string filepath = getTempFilePath();
    const int original = -999;

    { Serializer ser(filepath); ser << original; }

    int restored;
    { DeSerializer deser(filepath); deser >> restored; }

    EXPECT_EQ(restored, original);
    std::remove(filepath.c_str());
}

TEST(SerializerTest, OperatorUnsignedInt) {
    std::string filepath = getTempFilePath();
    const unsigned int original = 12345U;

    { Serializer ser(filepath); ser << original; }

    unsigned int restored;
    { DeSerializer deser(filepath); deser >> restored; }

    EXPECT_EQ(restored, original);
    std::remove(filepath.c_str());
}

TEST(SerializerTest, OperatorLong) {
    std::string filepath = getTempFilePath();
    const long original = 1234567890L;

    { Serializer ser(filepath); ser << original; }

    long restored;
    { DeSerializer deser(filepath); deser >> restored; }

    EXPECT_EQ(restored, original);
    std::remove(filepath.c_str());
}

TEST(SerializerTest, OperatorUnsignedLong) {
    std::string filepath = getTempFilePath();
    const unsigned long original = 9876543210UL;

    { Serializer ser(filepath); ser << original; }

    unsigned long restored;
    { DeSerializer deser(filepath); deser >> restored; }

    EXPECT_EQ(restored, original);
    std::remove(filepath.c_str());
}

TEST(SerializerTest, OperatorDouble) {
    std::string filepath = getTempFilePath();
    const double original = 1.23456789;

    { Serializer ser(filepath); ser << original; }

    double restored;
    { DeSerializer deser(filepath); deser >> restored; }

    EXPECT_NEAR(restored, original, 1e-15);
    std::remove(filepath.c_str());
}

TEST(SerializerTest, OperatorString) {
    std::string filepath = getTempFilePath();
    const std::string original = "operator test";

    { Serializer ser(filepath); ser << original; }

    std::string restored;
    { DeSerializer deser(filepath); deser >> restored; }

    EXPECT_EQ(restored, original);
    std::remove(filepath.c_str());
}

TEST(SerializerTest, OperatorBool) {
    std::string filepath = getTempFilePath();
    const bool original = true;

    { Serializer ser(filepath); ser << original; }

    bool restored;
    { DeSerializer deser(filepath); deser >> restored; }

    EXPECT_EQ(restored, original);
    std::remove(filepath.c_str());
}

// Test error handling - file not found for reading
TEST(SerializerTest, DeserializeNonexistentFileThrows) {
    EXPECT_THROW({
        DeSerializer deser("/nonexistent/path/file.bin");
    }, RuntimeError);
}

// Test error handling - write to invalid path
TEST(SerializerTest, SerializeInvalidPathThrows) {
    EXPECT_THROW({
        Serializer ser("/nonexistent/path/file.bin");
    }, RuntimeError);
}

int main(int argc, char** argv) {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
