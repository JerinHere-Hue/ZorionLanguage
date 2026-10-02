#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <vector>

#include "generation.h"

int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cerr << "Incorrect usage. Correct usage is..." << std::endl;
        std::cerr << "hydro <input.hy> [-o <output_path>]" << std::endl;
        return EXIT_FAILURE;
    }

    std::string input_path;
    std::string output_path = "out"; // default if -o not given

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-o") {
            if (i + 1 >= argc) {
                std::cerr << "Expected a path after -o" << std::endl;
                return EXIT_FAILURE;
            }
            output_path = argv[++i];
        }
        else if (input_path.empty()) {
            input_path = arg;
        }
        else {
            std::cerr << "Unexpected argument: " << arg << std::endl;
            return EXIT_FAILURE;
        }
    }

    if (input_path.empty()) {
        std::cerr << "No input file given." << std::endl;
        std::cerr << "hydro <input.hy> [-o <output_path>]" << std::endl;
        return EXIT_FAILURE;
    }

    std::string contents;
    {
        std::stringstream contents_stream;
        std::fstream input(input_path, std::ios::in);
        if (!input.is_open()) {
            std::cerr << "Could not open input file: " << input_path << std::endl;
            return EXIT_FAILURE;
        }
        contents_stream << input.rdbuf();
        contents = contents_stream.str();
    }

    Tokenizer tokenizer(std::move(contents));
    std::vector<Token> tokens = tokenizer.tokenize();

    Parser parser(std::move(tokens));
    std::optional<NodeProg> prog = parser.parse_prog();

    if (!prog.has_value()) {
        std::cerr << "Invalid program" << std::endl;
        return EXIT_FAILURE;
    }

    // Build the asm/object files next to the requested output path,
    // using its name as a base, so intermediate files don't collide
    // across multiple compiles.
    const std::string asm_path = output_path + ".asm";
    const std::string obj_path = output_path + ".o";

    {
        Generator generator(prog.value());
        std::fstream file(asm_path, std::ios::out);
        file << generator.gen_prog();
    }

    const std::string nasm_cmd = "nasm -felf64 \"" + asm_path + "\"";
    const std::string ld_cmd = "ld -o \"" + output_path + "\" \"" + obj_path + "\"";

    if (system(nasm_cmd.c_str()) != 0) {
        std::cerr << "nasm failed" << std::endl;
        return EXIT_FAILURE;
    }
    if (system(ld_cmd.c_str()) != 0) {
        std::cerr << "ld failed" << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Compiled to: " << output_path << std::endl;

    return EXIT_SUCCESS;
}