#include "Sigil2Config.hpp"

auto Sigil2Config::registerBackend(ToolName name, Backend be) -> Sigil2Config&
{
    std::transform(name.begin(), name.end(), name.begin(), ::tolower);
    beFactory.add(name, be.generator);
    beFactory.add(name, be.parser);
    beFactory.add(name, be.finish);
    return *this;
}


auto Sigil2Config::registerFrontend(ToolName name, Frontend fe) -> Sigil2Config&
{
    std::transform(name.begin(), name.end(), name.begin(), ::tolower);
    feFactory.add(name, fe.start);
    feFactory.add(name, fe.acq);
    feFactory.add(name, fe.rel);
    feFactory.add(name, fe.ready);
    return *this;
}

auto Sigil2Config::parseCommandLine(int argc, char* argv[]) -> Sigil2Config&
{
    Sigil2Parser parser(argc, argv);

    /* number of threads for Sigil2 (N/A for valgrind frontend) */
    _threads = parser.threads();

    /* backend */
    std::string beName;
    std::vector<std::string> beArgs;
    std::tie(beName, beArgs) = parser.backend();
    _backend = beFactory.create(beName, beArgs);

    /* executable */
    auto execArgs = parser.executable();

    /* frontend */
    std::string feName;
    std::vector<std::string> feArgs;
    std::tie(feName, feArgs) = parser.frontend();
    _frontend = feFactory.create(feName, {execArgs, feArgs, _threads});

    return *this;
}
