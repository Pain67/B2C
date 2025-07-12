#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>

// minimum size in bytes a source file needs to have
// to enable Logging a progress in std::out
// ~20 mb
#define PROGRESS_MIN_SIZE 20000000

class B2C {
private:

protected:
    const char HexList[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F',};
    int ArgC = 0;
    char** ArgV = nullptr;

    std::string RootDir = "";
    std::vector<std::string> Args;

    std::string SourceFileName = "";
    std::string SourcePath = "";
    std::string TargetPath = "";
    std::string DataSetName = "";
    bool is_hpp = false;
    uint64_t SourceSize = 0;
    bool is_NoProgress = false;
    bool is_ForceProgress = false;
    uint64_t ProgressStep = 3000000;
    bool is_Compact = false;
    uint32_t RowSize = 8;
    bool is_IncludeGuard = false;
    std::string GuardStr = "";

    std::string OutFileContent = "";

    std::string CharToHex(uint8_t IN_Value);
    std::string StringToUpperCase(std::string& REF_String);

    void ParseArguments();
    bool EvaluateArgs();
    void PrintUsage();
    void PrintState();
    bool Generate();
public:
    B2C(int argc, char** argv);
};
