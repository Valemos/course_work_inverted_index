#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include "index/Index.h"
#include "index/IndexBuilder.h"

namespace fs = std::filesystem;


class IndexTempFilesTest : public ::testing::Test {

protected:

    fs::path temp_dir_ {".\\test_temp"};
    std::vector<fs::path> temp_files_;

    Index index_;

    void writeToTemp(const std::string& content, const fs::path& path) {
        std::ofstream fout {path};
        fout << content;
        fout.close();
    }

    void SetUp() override {
        fs::create_directory(temp_dir_);

        int file_index = 0;
        temp_files_.resize(5, temp_dir_);
        for (auto& path : temp_files_) {
            path = path / ("file" + std::to_string(file_index++) + ".txt");
        }

        writeToTemp("words test test", temp_files_[0]);
        writeToTemp("test other here", temp_files_[1]);
        writeToTemp("", temp_files_[2]);
        writeToTemp("unique contents", temp_files_[3]);
        writeToTemp("1234567890 24564564\n433443", temp_files_[4]);

        for (auto& file : temp_files_) {
            index_.addFile(file);
        }
    }

    void TearDown() override {
        fs::remove_all(temp_dir_);
    }
};


TEST_F(IndexTempFilesTest, FailToFindNonExistingWord) {
    auto results = index_.find("nonexisting");
    EXPECT_TRUE(results.empty());
}

TEST_F(IndexTempFilesTest, FindSingleWord_FirstPosition){
    auto search_results = index_.find("test");
    
    ASSERT_FALSE(search_results.empty());
    EXPECT_EQ(search_results.begin()->position.start, (std::streamoff)5);
};

TEST_F(IndexTempFilesTest, FindAllWords){
    auto search_results = index_.find("test");
    
    ASSERT_FALSE(search_results.empty());
    EXPECT_EQ(search_results.size(), 3);
};

TEST_F(IndexTempFilesTest, FindTwoWordsInDocument){
    auto search_results = index_.find("test other");
    
    ASSERT_FALSE(search_results.empty());
    EXPECT_EQ(search_results.size(), 2);
    EXPECT_EQ(search_results[0].position, TokenPosition(1, 0));
    EXPECT_EQ(search_results[1].position, TokenPosition(1, 4));
};

TEST_F(IndexTempFilesTest, TestIndexSerialization) {
    index_.save(temp_dir_ / "index");

    auto other_index = Index::load(temp_dir_ / "index");

    ASSERT_TRUE(fs::exists(temp_dir_ / "index"));
    EXPECT_TRUE(index_ == other_index);
};

TEST_F(IndexTempFilesTest, IndexBuilder_MultithreadedEqualToSingleThreaded) {
    IndexBuilder builder(4);
    builder.indexDirectory(temp_dir_);

    Index single_threaded;
    for (auto& path : temp_files_) {
        single_threaded.addFile(path);
    }

    EXPECT_TRUE(builder.getIndex() == single_threaded);
};

TEST(IndexBuildTest, FailToCreateFromNonExistingDirectory) {
    IndexBuilder builder(4);
    try {
        builder.indexDirectory("./nonexisting");
        FAIL() << "not failed on not existing directory";
    } catch (std::exception& err) {
        EXPECT_TRUE(builder.getIndex() == Index()) << err.what();
    }
}

TEST(IndexBuildTest, FailToAddNotExistingFile) {
    auto index = Index();
    try {
        index.addFile("nonexisting.txt");
        FAIL() << "not failed on not existing file";
    } catch (std::exception& err) {
        EXPECT_TRUE(index == Index()) << err.what();
    }
};

