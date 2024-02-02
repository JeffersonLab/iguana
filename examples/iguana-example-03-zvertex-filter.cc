#include <iguana/algorithms/AlgorithmSequence.h>
#include <hipo4/reader.h>
#include <sstream>

// show a bank along with a header
void prettyPrint(std::string header, hipo::bank &bank)
{
    fmt::print("{:=^70}\n", " " + header + " ");
    bank.show();
}

int main(int argc, char **argv)
{

    // parse arguments
    int argi = 1;
    const char *inFileName = argc > argi ? argv[argi++] : "data.hipo";
    const int numEvents = argc > argi ? std::stoi(argv[argi++]) : 1;

    // read input file
    hipo::reader reader(inFileName);

    // set banks
    hipo::banklist banks = reader.getBanks({"REC::Particle"});
    enum banks_enum
    {
        b_particle
    }; // TODO: users shouldn't have to do this

    // iguana algorithm sequence
    iguana::AlgorithmSequence seq;

    seq.Add("clas12::ZVertexFilter"); // filter by Event Builder PID

    // set log levels
    seq.SetOption("clas12::ZVertexFilter", "log", "debug");
    seq.GetConfigFileManager()->SetLogLevel("debug");

    // set algorithm options - if we don't set these, the default values from config files are used.
    // seq.SetOption<std::vector<double>>("clas12::ZVertexFilter", "low&high", {-13.0,12.0});

    // start the algorithms
    seq.Start(banks);

    // run the algorithm sequence on each event
    int iEvent = 0;
    while (reader.next(banks) && (numEvents == 0 || iEvent++ < numEvents))
    {
        prettyPrint("BEFORE", banks.at(b_particle));
        seq.Run(banks);
        prettyPrint("AFTER", banks.at(b_particle));
    }

    // stop algorithms
    seq.Stop();
}
