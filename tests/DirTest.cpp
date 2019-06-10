
#include "UTCommon.h"

#include "io/Dir.h"

#include <boost/filesystem.hpp>

struct DirTest : public testing::Test {
    DirTest() {
        m_tmp_test_dir = create_temp_test_directory();
    }

protected:
    boost::filesystem::path m_tmp_test_dir;
};

TEST_F(DirTest, default_constructor) {
    io::EventLoop loop;

    auto dir = new io::Dir(loop);

    ASSERT_EQ(0, loop.run());
    dir->schedule_removal();
}

// TODO: open dir test

TEST_F(DirTest, list_elements) {
    boost::filesystem::create_directories(m_tmp_test_dir/ "dir_1");
    boost::filesystem::create_directories(m_tmp_test_dir/ "dir_2");
    {
        std::ofstream ofile((m_tmp_test_dir/ "file_1").string());
        ASSERT_FALSE(ofile.fail());
    }
    boost::filesystem::create_directories(m_tmp_test_dir/ "dir_3");
    {
        std::ofstream ofile((m_tmp_test_dir/ "file_2").string());
        ASSERT_FALSE(ofile.fail());
    }

    io::EventLoop loop;

    bool dir_1_listed = false;
    bool dir_2_listed = false;
    bool dir_3_listed = false;
    bool file_1_listed = false;
    bool file_2_listed = false;

    auto dir = new io::Dir(loop);
    dir->open(m_tmp_test_dir.string(), [&](io::Dir& dir) {
        dir.read([&](io::Dir& dir, const char* name, io::DirectoryEntryType entry_type) {
            if (std::string(name) == "dir_1") {
                EXPECT_FALSE(dir_1_listed);
                EXPECT_EQ(io::DirectoryEntryType::DIR, entry_type);
                dir_1_listed = true;
            } else if (std::string(name) == "dir_2") {
                EXPECT_FALSE(dir_2_listed);
                EXPECT_EQ(io::DirectoryEntryType::DIR, entry_type);
                dir_2_listed = true;
            } else if (std::string(name) == "dir_3") {
                EXPECT_FALSE(dir_3_listed);
                EXPECT_EQ(io::DirectoryEntryType::DIR, entry_type);
                dir_3_listed = true;
            } else if (std::string(name) == "file_1") {
                EXPECT_FALSE(file_1_listed);
                EXPECT_EQ(io::DirectoryEntryType::FILE, entry_type);
                file_1_listed = true;
            } else if (std::string(name) == "file_2") {
                EXPECT_FALSE(file_2_listed);
                EXPECT_EQ(io::DirectoryEntryType::FILE, entry_type);
                file_2_listed = true;
            }
        });
    });

    ASSERT_EQ(0, loop.run());
    dir->schedule_removal();

    EXPECT_TRUE(dir_1_listed);
    EXPECT_TRUE(dir_2_listed);
    EXPECT_TRUE(dir_3_listed);
    EXPECT_TRUE(file_1_listed);
    EXPECT_TRUE(file_2_listed);
}

// dir open not exist
// dir iterate not existing