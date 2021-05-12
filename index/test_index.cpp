#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <iostream>
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
        fs::remove_all("./temp");
    }
};

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


