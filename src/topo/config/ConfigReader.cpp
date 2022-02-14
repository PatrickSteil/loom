// Copyright 2016, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Authors: Patrick Brosi <brosi@informatik.uni-freiburg.de>

#include <float.h>
#include <boost/program_options.hpp>
#include <exception>
#include <iostream>
#include <string>
#include "topo/config/ConfigReader.h"
#include "util/log/Log.h"

using topo::config::ConfigReader;
namespace opts = boost::program_options;

using std::string;
using std::exception;
using std::vector;

// _____________________________________________________________________________
ConfigReader::ConfigReader() {}

// _____________________________________________________________________________
void ConfigReader::read(TopoConfig* cfg, int argc, char** argv) const {
  string VERSION_STR = " - unversioned - ";

  opts::options_description generic("General");
  generic.add_options()("version", "output version")(
      "help,?", "show this message")("verbose,v", "verbosity level");

  opts::options_description config("Output");
  config.add_options()
    ("max-aggr-distance,d",
      opts::value<double>(&(cfg->maxAggrDistance))->default_value(40),
      "maximum aggregation distance between shared segments")
    ("write-stats",
      opts::bool_switch(&(cfg->outputStats))
      ->default_value(false),
      "write stats to output file")
    ("no-infer-restrs",
      opts::bool_switch(&(cfg->noInferRestrs))
      ->default_value(false),
      "don't infer turn restrictions")
    ("max-length-dev",
      opts::value<double>(&(cfg->maxLengthDev))->default_value(500),
      "maximal distance deviation for turn restriction infer");

  opts::options_description cmdlineOptions;
  cmdlineOptions.add(config).add(generic);

  opts::options_description visibleDesc("Allowed options");
  visibleDesc.add(generic).add(config);
  opts::variables_map vm;

  try {
    opts::store(opts::command_line_parser(argc, argv)
                    .options(cmdlineOptions)
                    .run(),
                vm);
    opts::notify(vm);
  } catch (exception e) {
    LOG(ERROR) << e.what() << std::endl;
    std::cout << visibleDesc << "\n";
    exit(1);
  }

  if (vm.count("help")) {
    std::cout << argv[0] << " [options] <input-feed>\n"
              << VERSION_STR << "\n\n"
              << visibleDesc << "\n";
    exit(0);
  }

  if (vm.count("version")) {
    std::cout << "\n" << VERSION_STR << "\n";
    exit(0);
  }
}
