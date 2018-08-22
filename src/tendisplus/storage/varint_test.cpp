#include <vector>
#include <algorithm>
#include "tendisplus/storage/varint.h"
#include "gtest/gtest.h"

namespace tendisplus {

void testVarint(uint64_t val, std::vector<uint8_t> bytes) {
    EXPECT_EQ(bytes, varintEncode(val));

    auto expt = varintDecodeFwd(bytes.data(), bytes.size());
    EXPECT_TRUE(expt.ok());
    EXPECT_EQ(expt.value().first, val);
    EXPECT_EQ(expt.value().second, bytes.size());

    std::reverse(bytes.begin(), bytes.end());
    expt = varintDecodeRvs(bytes.data()+bytes.size()-1, bytes.size());
    EXPECT_TRUE(expt.ok());
    EXPECT_EQ(expt.value().first, val);
    EXPECT_EQ(expt.value().second, bytes.size());

    // then, test trailing bytes
    std::reverse(bytes.begin(), bytes.end());
    // varint64 has a maxsize of 10
    uint8_t buf[10];
    memcpy(buf, bytes.data(), 10);
    uint8_t fills[] = {0, 0x7f, 0x80, 0xff};
    for (auto& v : fills) {
        memset(buf+bytes.size(), v, 10 - bytes.size());
        std::vector<uint8_t> tmp(buf, buf+10);
        expt = varintDecodeFwd(tmp.data(), tmp.size());
        EXPECT_TRUE(expt.ok());
        EXPECT_EQ(expt.value().first, val);
        EXPECT_EQ(expt.value().second, bytes.size());

        std::reverse(tmp.begin(), tmp.end());
        expt = varintDecodeRvs(tmp.data()+tmp.size()-1, tmp.size());
        EXPECT_TRUE(expt.ok());
        EXPECT_EQ(expt.value().first, val);
        EXPECT_EQ(expt.value().second, bytes.size());
    }
}

TEST(Varint, Common) {
    // NOTE(deyukong): the testdata are stolen from
    // facebook folly's varint testcases.
    testVarint(0, {0});
    testVarint(1, {1});
    testVarint(127, {127});
    testVarint(128, {0x80, 0x01});
    testVarint(300, {0xac, 0x02});
    testVarint(16383, {0xff, 0x7f});
    testVarint(16384, {0x80, 0x80, 0x01});

    testVarint(static_cast<uint32_t>(-1),
        {0xff, 0xff, 0xff, 0xff, 0x0f});
    testVarint(static_cast<uint64_t>(-1),
        {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x01});
}

}  // namespace tendisplus