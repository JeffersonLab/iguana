#include <iguana/services/YAMLReader.h>
#include <iostream>

int main(int argc, char **argv) {

    // instantiate a `YAMLReader` parser, and set its log level to "debug"
    iguana::YAMLReader conf;
    conf.SetLogLevel("debug");

    // parse arguments
    std::string inFileName;
    if(argc>1) { // read this particular YAML file
      inFileName = std::string(argv[1]);
    }
    else { // read the example YAML file
      // add a config file directory that is relative to this executable;
      // since this example is installed with the rest of `iguana`, it's in the
      // installation prefix under `etc/`:
      //
      //    iguana_installation_prefix
      //    ├── bin
      //    │   └── argv[0]
      //    └── etc
      //        └── iguana
      //            └── examples
      //                └── ex2.yaml
      //
      auto executableDir = iguana::ConfigFileReader::DirName(argv[0]);
      conf.AddDirectory(executableDir + "/../etc/iguana/examples");
      // set the example filename (to be found in the above path)
      inFileName = "ex2.yaml";
    }

    // add the config file and parse it
    conf.AddFile(inFileName);
    conf.LoadFiles();

    //Below we access cut values defined for different and different pids.
    //There's a series of key we need to use
    //First the cutkey relates to the array containing all runperiods.
    std::string cutKey="cuts";

    //second the runkey relates to each run period.
    std::string runKey="runs";

    //the pidkey relates to the pid dependent cuts.
    //Note we could have sector dependent cuts too (but not both atm).
    std::string pidKey="pid";
    std::string secKey="sector";

    //finally the values key relates to the actual cut values.
    std::string valKey="vals";

    //If a pid dependence is not specified for a given run period the reader
    // will still get the cut values for that run period.
    //We could pass any pid value at thsat point.

    int runnb = 4768;
    int pid=0;
    std::vector<double> cutvalues = conf.findKeyAtRunAndPIDVector<double>(cutKey,runKey,pidKey,valKey,runnb, pid, {-20.0, 20.0});

    std::cout<<"\nFor run "<<runnb<<" no restriction on pid "<<std::endl;
    std::cout<<"Cut value low "<<cutvalues[0]<<" high "<<cutvalues[1]<<std::endl;

    runnb = 5423;
    cutvalues = conf.findKeyAtRunAndPIDVector<double>(cutKey,runKey,pidKey,valKey,runnb, pid, {-20.0, 20.0});

    std::cout<<"\nFor run "<<runnb<<" no restriction on pid "<<std::endl;
    std::cout<<"Cut value low "<<cutvalues[0]<<" high "<<cutvalues[1]<<std::endl;

    runnb = 6143;
    pid=11;
    cutvalues = conf.findKeyAtRunAndPIDVector<double>(cutKey,runKey,pidKey,valKey,runnb, pid, {-20.0, 20.0});

    std::cout<<"\nFor run "<<runnb<<" and pid "<<pid<<std::endl;
    std::cout<<"Cut value low "<<cutvalues[0]<<" high "<<cutvalues[1]<<std::endl;

    pid=211;
    cutvalues = conf.findKeyAtRunAndPIDVector<double>(cutKey,runKey,pidKey,valKey,runnb, pid, {-20.0, 20.0});

    std::cout<<"\nFor run "<<runnb<<" and pid "<<pid<<std::endl;
    std::cout<<"Cut value low "<<cutvalues[0]<<" high "<<cutvalues[1]<<std::endl;

    //now switching to sector key instead of pid
    int sector=5;
    cutvalues = conf.findKeyAtRunAndPIDVector<double>(cutKey,runKey,secKey,valKey, runnb, sector, {-20.0, 20.0});

    std::cout<<"\nFor run "<<runnb<<" and sector "<<sector<<std::endl;
    std::cout<<"Cut value low "<<cutvalues[0]<<" high "<<cutvalues[1]<<std::endl;

    //now getting individual values
    int l=0;
    int h=1;
    double low = conf.findKeyAtRunAndPID<double>(cutKey,runKey,"single",valKey, runnb, l, -20);
    double high = conf.findKeyAtRunAndPID<double>(cutKey,runKey,"single",valKey, runnb, h, 20);

    std::cout<<"\nFor run "<<runnb<<std::endl;
    std::cout<<"Cut value low "<<low<<" high "<<high<<std::endl;

    //Accessing a run that doesn't have a corresponding range returns default
    //Same if we did PID or sector that doesn't exist
    runnb = 4;
    cutvalues = conf.findKeyAtRunAndPIDVector<double>(cutKey,runKey,pidKey,valKey,runnb, pid, {-20.0, 20.0});

    std::cout<<"\nFor run "<<runnb<<" and pid "<<pid<<std::endl;
    std::cout<<"Cut value low "<<cutvalues[0]<<" high "<<cutvalues[1]<<std::endl;

    //Now we just get some single values
    int mi = conf.readValue("myInt",0);
    double md = conf.readValue("myDouble",0.0);
    std::string ms= conf.readValue<std::string>("myString","");//force template to use std::string not char const*

    std::cout<<"\nSingle values"<<std::endl;
    std::cout<<"myInt "<<mi<<" myDouble "<<md<<" myString "<<ms<<std::endl;

    //Now we just get some individual arrays
    std::vector<int> miv = conf.readArray<int>("myIntVector",{});
    std::vector<double> mdv = conf.readArray<double>("myDoubleVector",{});
    std::vector<std::string> msv= conf.readArray<std::string>("myStringVector",{});

    std::cout<<"\nIndividual Arrays"<<std::endl;
    std::cout<<"myIntVector: ";
    for(auto& val:miv){std::cout<<val<<" ";}
    std::cout<<std::endl;
    std::cout<<"myDoubleVector: ";
    for(auto& val:mdv){std::cout<<val<<" ";}
    std::cout<<std::endl;
    std::cout<<"myStringVector: ";
    for(auto& val:msv){std::cout<<val<<" ";}
    std::cout<<std::endl;

}
