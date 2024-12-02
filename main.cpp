#include <iostream>
#include "ArgumentsParser.h"
#include "GrammarReader.h"
#include <fstream>

int main(int argc, char** argv)
{
    try
    {
        Args args = ParseArgs(argc, argv);

        std::ifstream inputFile(args.inputFilename);
        if (!inputFile.is_open())
        {
            throw std::runtime_error("Failed to open input file");
        }

        MooreAutomata automata = GetAutomata(inputFile);

        automata.ExportToFile(args.outputFilename);

        std::cout << "Executed!\n";
    }
    catch (const std::exception& err)
    {
        std::cout << err.what() << std::endl;
    }

    return 0;
}
