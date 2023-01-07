#include <iostream>
#include <fstream>
#include <string>

using namespace std::string_literals;

int main(int argc, char* argv[])
{
    if (argc != 4)
    {
        std::cerr << "USAGE: " << argv[0] << " {output-name} {output-dir} {resource-file}\n\n"
                  << "  Creates {output-dir}/{output-name}.cpp from the contents of {resource-file}\n";
        return EXIT_FAILURE;
    }

    const char* resource_file = argv[3];
    std::ifstream in(resource_file, std::ios::in | std::ios::binary);
    if (!in)
    {
        std::cerr << "unable to open " << resource_file << " for reading\n";
        return EXIT_FAILURE;
    }

    auto resource_name = argv[1];
    auto output = argv[2] + "/"s + resource_name + ".cpp";
    std::ofstream out(output, std::ios::out | std::ios::trunc);
    if (!out)
    {
        std::cerr << "unable to open " << output << " for writing\n";
        return EXIT_FAILURE;
    }

    out << "#include <cstddef>\n"
        << "#include <span>\n"
        << "\n"
        << "static unsigned char data[] = {\n";

    size_t n = 0;
    for (;;)
    {
        char c;
        in.get(c);

        if (!in.good())
        {
            if (in.eof())
                break;

            std::cerr << "error during reading from " << resource_file << "\n";
            return EXIT_FAILURE;
        }

        out << "0x" << std::hex << ((unsigned)(unsigned char)c >> 4)
                    << std::hex << ((unsigned)(unsigned char)c & 0x0f) << ", ";
        ++n;
        if ((n % 16) == 0)
            out << "\n";
    }

    out << "};\n\n"
        << "std::span<std::byte const> "
        << resource_name << "() { return as_bytes(std::span(data)); }\n";

    if (!out.good())
    {
        std::cerr << "error during writing to " << output << "\n";
        return EXIT_FAILURE;
    }
}
