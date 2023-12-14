#include "file_system.h"
#include "gtest/gtest.h"

class FileSystemTest : public ::testing::Test {
protected:
    void SetUp() override {}

    void TearDown() override {}
};

TEST_F(FileSystemTest, BasicFunctionality) {
    FileSystem fs;

    EXPECT_EQ("Contents of testDir:\ntestFile.txt\n", captureStdout([&]() {
        fs.mkdir("testDir");
        fs.cd("testDir");
        fs.touch("testFile.txt");
        fs.ls();
    }));

    EXPECT_EQ("Contents of /:\ntestDir/ ", captureStdout([&]() {
        fs.cd("..");
        fs.ls();
    }));

    EXPECT_EQ("Contents of testDir:\nDirectory is empty\n", captureStdout([&]() {
        fs.rm("testDir/testFile.txt");
        fs.ls();
    }));

    EXPECT_EQ("Text written to newFile.txt: Hello, World!\n", captureStdout([&]() {
        fs.touch("newFile.txt");
        fs.echo("Hello, World!", "newFile.txt");
        fs.cat("newFile.txt");
    }));

    EXPECT_EQ("Contents of /:\ntestDir/ newDir/ copiedFile.txt ", captureStdout([&]() {
        fs.mkdir("newDir");
        fs.mv("newFile.txt", "newDir");
        fs.cp("newDir/newFile.txt", "copiedFile.txt");
        fs.ls();
    }));
}

TEST_F(FileSystemTest, ErrorHandling) {
    FileSystem fs;

    EXPECT_EQ("Error: Invalid path\n", captureStdout([&]() {
        fs.cd("nonexistentDir");
    }));

    EXPECT_EQ("Error: Invalid file or not a regular file\n", captureStdout([&]() {
        fs.cat("nonexistentFile.txt");
    }));

    EXPECT_EQ("Error: Target not found\n", captureStdout([&]() {
        fs.rm("nonexistentFile.txt");
    }));
}

TEST_F(FileSystemTest, MoveAndCopyErrors) {
    FileSystem fs;

    EXPECT_EQ("Error: Invalid source or not a regular file\n", captureStdout([&]() {
        fs.mv("nonexistentFile.txt", "newDir");
    }));

    EXPECT_EQ("Error: Invalid destination or not a directory\n", captureStdout([&]() {
        fs.touch("fileToMove.txt");
        fs.mv("fileToMove.txt", "nonexistentDir");
    }));

    EXPECT_EQ("Error: Invalid source or not a regular file\n", captureStdout([&]() {
        fs.cp("nonexistentFile.txt", "newDir");
    }));

    EXPECT_EQ("Error: Invalid destination or not a directory\n", captureStdout([&]() {
        fs.touch("fileToCopy.txt");
        fs.cp("fileToCopy.txt", "nonexistentDir");
    }));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
