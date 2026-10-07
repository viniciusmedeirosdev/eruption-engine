#include "core/ConfigManager.hpp"
#include <iostream>
#include <stdexcept>
#include <unistd.h>

int main() {
    namespace fs = std::filesystem;
    const auto dir = fs::temp_directory_path() / ("eruption-config-test-" + std::to_string(getpid()));
    fs::create_directories(dir);
    const auto file = dir / "config.json";
    auto require = [](bool ok, const char* message) {
        if (!ok) throw std::runtime_error(message);
    };
    try {
        eruption::ConfigManager config;
        require(!config.load((dir / "missing.json").string()), "missing file accepted");
        require(config.root().value("enabled", true), "missing config must allow defaults");
        auto write = [&](const char* text) { std::ofstream out(file); out << text; };
        write(R"({"enabled":false,"radius":12})");
        require(config.load(file.string()), "valid config rejected");
        require(!config.root().value("enabled", true), "valid setting lost");
        for (const auto* invalid : {"null", "[]", "42", "{broken"}) {
            write(invalid);
            require(!config.load(file.string()), "invalid config accepted");
            require(config.root().value("radius", 0) == 12, "failed reload lost valid config");
        }
        fs::remove_all(dir);
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        fs::remove_all(dir);
        return 1;
    }
}
