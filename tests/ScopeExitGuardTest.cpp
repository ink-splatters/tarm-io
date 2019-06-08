#include "UTCommon.h"

#include "io/ScopeExitGuard.h"

struct ScopeExitGuardTest : public testing::Test {

};

TEST_F(ScopeExitGuardTest, callback_is_called) {
    bool callback_called_1 = false;
    bool callback_called_2 = false;

    io::ScopeExitGuard guard_1([&]() {
        ASSERT_TRUE(callback_called_2);
        callback_called_1 = true;
    });

    io::ScopeExitGuard guard_2([&]() {
        ASSERT_FALSE(callback_called_1);
        callback_called_2 = true;
    });

    ASSERT_FALSE(callback_called_1);
    ASSERT_FALSE(callback_called_1);
}
