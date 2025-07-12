#include "b2c.hpp"

#include <chrono>
#include <cctype>

std::string B2C::CharToHex(uint8_t IN_Value) {
    char High = IN_Value >> 4;
    char Low = IN_Value & 0b00001111;

    std::string Result = "0x";
    Result += HexList[High];
    Result += HexList[Low];

    return Result;
}

void B2C::ParseArguments() {
    // Safe to assume that 1st argument must always be provided
    std::filesystem::path P(ArgV[0]);
    RootDir = P.remove_filename().string();

    // See if any more arguments
    if (ArgC > 1) {
        for (int X = 0; X < ArgC; X++) {
            std::string Temp(ArgV[X]);
            Args.push_back(Temp);
        }
    }
}
std::string B2C::StringToUpperCase(std::string& REF_String) {
    std::string Result;
    int Num = REF_String.length();
    if (Num > 0) {
        for (int X = 0; X < Num; X++) {
            Result += std::toupper(REF_String[X]);
        }
    }
    return Result;
}

bool B2C::EvaluateArgs() {
    unsigned int Num = Args.size();

    if (Num < 2) {
        std::cout << "Missing required argument <source_file_path>" << std::endl;
        PrintUsage();
        return false;
    }

    // 1st argument is different
    if (Args[1][0] == '-') {
        std::cout << "Invalid argument. First argument expected to be the source file path but found [" << Args[1] << "]" << std::endl;
        return false;
    }
    SourcePath = Args[1];
    std::filesystem::path P(SourcePath);
    SourceSize = std::filesystem::file_size(P);
    SourceFileName = P.stem().string();

    if (Num > 2) {
        for (unsigned int X = 2; X < Num; X++) {

            if (Args[X] == "-o") {
                if ((X+1) < Num) {
                    TargetPath = Args[X + 1];
                    X++;
                    continue;
                }
                else {
                    std::cout << "Missing required argument; '-o' expects a file path to be specified" << std::endl;
                    return false;
                }
            }

            if (Args[X] == "-t") {
                if ((X+1) < Num) {
                    std::string& REF = Args[X + 1];
                    if (REF == "h") { is_hpp = false; }
                    else if (REF == "hpp") { is_hpp = true; }
                    else {
                        std::cout << "Invalid Argument; '-t' expected 'h' or 'hpp' but found [" << REF << "]" << std::endl;
                        return false;
                    }
                    X++;
                    continue;
                }
                else {
                    std::cout << "Missing required argument; '-t' expects the data type to be specified {h, hpp}" << std::endl;
                    return false;
                }
            }

            if (Args[X] == "-n") {
                if ((X+1) < Num) {
                    DataSetName = Args[X + 1];
                    X++;
                    continue;
                }
                else {
                    std::cout << "Missing required argument; '-n' expects a data set name to be specified" << std::endl;
                    return false;
                }
            }
            if (Args[X] == "-ps") {
                if ((X+1) < Num) {
                    ProgressStep = std::stoi(Args[X + 1]);
                    X++;
                    continue;
                }
                else {
                    std::cout << "Missing required argument; '-ps' expects a number to be specified" << std::endl;
                    return false;
                }
            }
            if (Args[X] == "-r") {
                if ((X+1) < Num) {
                    RowSize = std::stoi(Args[X + 1]);
                    X++;
                    continue;
                }
                else {
                    std::cout << "Missing required argument; '-r' expects a number to be specified" << std::endl;
                    return false;
                }
            }
            if (Args[X] == "-p") {
                is_ForceProgress = true;
                continue;
            }
            if (Args[X] == "-c") {
                is_Compact = true;
                continue;
            }
            if (Args[X] == "-nop") {
                is_NoProgress = true;
                continue;
            }
            if (Args[X] == "-g") {
                is_IncludeGuard = true;
                continue;
            }

            // if none of them picked up the argument,
            // it is unexpected
            std::cout << "Unrecognised argument [" << Args[X] << "]." << std::endl;
            return false;
        }
    }

    // if targetFileName not provided
    if (TargetPath.length() == 0) {
        std::filesystem::path P(SourcePath);
        TargetPath = RootDir + P.stem().string();
    }


    // if data Set name not provided
    if (DataSetName.length() == 0) {
        DataSetName = SourceFileName;
    }

    if (is_hpp) { TargetPath += ".hpp"; }
    else { TargetPath += ".h"; }


    GuardStr = StringToUpperCase(DataSetName);
    GuardStr += "_ARRAY_INCLUDE";

    return true;
}

void B2C::PrintUsage() {
    std::cout << "Usage:" << std::endl;
    std::cout << "b2c <source_file_path> [options]:" << std::endl;
    std::cout << "  <source_file_path> (Required): Full path to the file to be coverted" << std::endl;
    std::cout << "  [options] (Optional): Any of the following options / flags:" << std::endl;
    std::cout << "    -o <target_file_name>: File name for the resulting file. If not specified, it defauls to \"out.extension\" where extension corresponds to the choosen output type (c or cpp)" << std::endl;
    std::cout << "    -t {h,hpp}: Specifies the out file format. Default: \"c\"" << std::endl;
    std::cout << "    -n <name>: Specifies the name of the data array generated from source file. If not specified, defaults to the source file name" << std::endl;
    std::cout << "    -nop: No progress printed, even if source file size above the limit" << std::endl;
    std::cout << "    -p: Progress will be printed even if source file size below the limit" << std::endl;
    std::cout << "    -ps <N>: Set the progress printing steps. Print progress status after every N bytes" << std::endl;
    std::cout << "    -c : Output file will be compact (no spaces or new lines)" << std::endl;
    std::cout << "    -r <N> : Sets the number of elements in a row (for the data array)" << std::endl;
    std::cout << "    -g : Add Include guard ('#pragme once' in hpp or 'ifndef' in h)" << std::endl;
}
void B2C::PrintState() {
    std::cout << std::endl << "Source File: " << SourcePath << std::endl;
    std::cout << "Target File: " << TargetPath << std::endl;
    std::cout << "Data Set Name: " << DataSetName << std::endl;
    if (is_hpp) { std::cout << "Extension: .HPP" << std::endl; }
    else { std::cout << "Extension: .H" << std::endl; }

    float EstSize = (SourceSize * 6) + 700;
    std::string Unit = "byte";
    if (EstSize > 1000) { EstSize /= 1000; Unit = "KB"; }
    if (EstSize > 1000) { EstSize /= 1000; Unit = "MB"; }
    if (EstSize > 1000) { EstSize /= 1000; Unit = "GB"; }


    std::cout << "Estimated target file size: " << EstSize << " " << Unit << std::endl;
    if (is_Compact) { std::cout << "Compact Mode: TRUE" << std::endl; }
    else { std::cout << "Compact Mode: FALSE" << std::endl; }
    std::cout << "Elements Per Row: " << RowSize << std::endl << std::endl;
}

bool B2C::Generate() {
    // Open files
    std::ifstream InFile(SourcePath);
    if (!InFile.is_open()) {
        std::cout << "Unable to open source file from [" << SourcePath << "]" << std::endl;
        return false;
    }

    std::ofstream OutFile(TargetPath);
    if (!OutFile.is_open()) {
        std::cout << "Unable create target file at [" << TargetPath << "]" << std::endl;
        InFile.close();
        return false;
    }

    // Get Source File Size
    InFile.seekg(0, std::ios::end);
    uint64_t FileSize = InFile.tellg();
    InFile.seekg(0, std::ios::beg);

    // Print Info
    std::cout << "Wrinting [" << FileSize << "] bytes into array [" << DataSetName << "]...\n" << std::endl;

    // Add Header and stuff
    if (is_IncludeGuard) {

        if (is_hpp) {
            // This case new line will be inserted even in compact mode - otherwise it will no work
            OutFile << "#pragma once\n";
        }
        else {
            // This case new line will be inserted even in compact mode - otherwise it will no work
            OutFile << "#ifndef " << GuardStr << "\n";
            OutFile << "#define " << GuardStr << "\n";
        }
    }

    OutFile << "const char " << DataSetName << "[" << FileSize << "] = {";
    if (!is_Compact) { OutFile << std::endl << "\t";}

    // process data - If data size bigger then PROGRESS_MIN_SIZE
    // Show a progress bar in std::out
    bool isProgress = FileSize >= PROGRESS_MIN_SIZE;
    float Progress = 0.0F;
    auto StartTime = std::chrono::high_resolution_clock::now();

    // Chec for progress midifying flags
    if (is_NoProgress) { isProgress = false; }
    if (is_ForceProgress) { isProgress = true; }

    uint8_t ENum = 0;
    for (uint64_t X = 0; X < FileSize; X++) {
        if (ENum > 0) {
            OutFile << ",";
            if (!is_Compact) { OutFile << " "; }
        }

        char C = InFile.get();
        std::string Hex = CharToHex(C);

        if (Hex.length() != 4) {
            std::cout << "Unexpected HEx value. Expected 2 [0xNN] but for [" << Hex << "] " << std::endl;
            InFile.close();
            OutFile.close();
            return false;
        }

        OutFile << Hex;
        if (ENum >= RowSize) {
            ENum = 0;

            OutFile << ",";
            if (!is_Compact) { OutFile << std::endl << "\t"; }
        }
        else { ENum++; }

        if (isProgress) {
            if ((X % ProgressStep) == 0) {
                // Time it took to process the last ProgressStep number of bytes
                auto EndTime = std::chrono::high_resolution_clock::now();
                double Time = std::chrono::duration_cast<std::chrono::microseconds>(EndTime - StartTime).count();

                // microsec to process 1 byte
                double Temp = Time / ProgressStep;
                // Byte to be processed in 1 sec
                double Speed = 1.0F / (Temp * 0.000001F);
                // Convert to MB
                Speed /= 1000000;

                Progress = (float)X / (float)FileSize;
                std::cout << "Processing " << std::setprecision(3) <<  (Progress * 100) << "% (" << Speed <<  " MB/s)"  << std::endl;

                // Restart Timer
                StartTime = std::chrono::high_resolution_clock::now();
            }
        }
    }

    // Add Closing bracket and Array Size
    if (is_Compact) {
        OutFile << "};const size_t " << DataSetName << "_length = " << FileSize << ";";
    }
    else {
        OutFile << "\n};\n\nconst size_t " << DataSetName << "_length = " << FileSize << ";";
    }

    if (is_IncludeGuard && !is_hpp) { OutFile << "\n#endif // " << GuardStr; }

    // Done :D
    InFile.close();
    OutFile.close();

    // Print Info
    std::cout << "Completed." << std::endl << std::endl;

    return true;
}

B2C::B2C(int argc, char** argv) {
    ArgC = argc;
    ArgV = argv;
    ParseArguments();
    if (!EvaluateArgs()) { throw -1; }

    if (is_ForceProgress && is_NoProgress) {
        std::cout << "Conflicting arguments. '-p' and '-nop' cannot be present at the same time" << std::endl;
        return;
    }

    PrintState();
    Generate();
}




int main(int argc, char** argv) {

    try { B2C Test(argc, argv); }
    catch(...) { return -1; }

    return 0;
}
