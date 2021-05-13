#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include "Index.h"

namespace fs = std::filesystem;


class IndexTest : public ::testing::Test {

protected:

    fs::path temp_dir_ {"./temp"};
    fs::path temp_1_ {temp_dir_ / "file1.txt"};
    fs::path temp_2_ {temp_dir_ / "file2.txt"};

    Index index_;

    void writeToTemp(const std::string& content, const fs::path& path) {
        std::ofstream fout {path};
        fout << content;
        fout.close();
    }

    void SetUp() override {
        fs::create_directory(temp_dir_);
        writeToTemp("words test test", temp_1_);
        writeToTemp("test other here", temp_2_);

        index_.addFile(temp_1_);
        index_.addFile(temp_2_);
    }

    void TearDown() override {
        fs::remove_all(temp_dir_);
    }
};


TEST_F(IndexTest, FailToFindNonExistingWord) {
    auto results = index_.find("nonexisting");
    EXPECT_TRUE(results.empty());
}

TEST_F(IndexTest, FindSingleWord_FirstPosition){
    auto search_results = index_.find("test");
    
    ASSERT_FALSE(search_results.empty());
    EXPECT_EQ(search_results.begin()->start, (std::streamoff)5);
};

TEST_F(IndexTest, FindAllWords){
    auto search_results = index_.find("test");
    
    ASSERT_FALSE(search_results.empty());
    EXPECT_EQ(search_results.size(), 3);
};

TEST_F(IndexTest, FindTwoWordsInDocument){
    auto search_results = index_.find("test other");
    
    ASSERT_FALSE(search_results.empty());
    EXPECT_EQ(search_results.begin()->document_index, 1);
    EXPECT_EQ(search_results.size(), 2);
};

TEST_F(IndexTest, GetFilePaths) {
    auto results = index_.find("test");
    
    std::map<int, std::string> expected {
        {0, temp_1_.string()}, 
        {1, temp_2_.string()}
    };

    EXPECT_EQ(index_.getFilePaths(results), expected);
}

TEST_F(IndexTest, TestIndexSerialization) {
    index_.save(temp_dir_ / "index");

    auto other_index = Index::load(temp_dir_ / "index");

    ASSERT_TRUE(fs::exists(temp_dir_ / "index"));
    EXPECT_EQ(index_.getTokenPositions(), other_index.getTokenPositions());
}
