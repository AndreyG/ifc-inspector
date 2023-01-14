#include "commander.h"

#include <ifc/blob_reader.h>

#include <iostream>

ifc::File::BlobView schema_raw_data();

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "USAGE: " << argv[0] << " .ifc file\n";
        return EXIT_FAILURE;
    }

    const std::filesystem::path path_to_ifc = argv[1];
    if (!is_regular_file(path_to_ifc))
    {
        std::cerr << "expected: path to .ifc file\n";
        return EXIT_FAILURE;
    }

    const auto input_data = ifc::read_blob(path_to_ifc);
    ifc::File file(input_data->view());

    ifc::File schema(schema_raw_data());
    Commander commander(reflifc::Module(schema), file);
    for (;;)
    {
        std::string command;
        std::cout << "> ";
        std::getline(std::cin, command);

        if (command == "exit")
            break;

        if (command.empty())
            continue;

        commander(command);
    }
    return EXIT_SUCCESS;
}