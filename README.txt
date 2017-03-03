# t-span
Mini project for low dist embeddings
0. 
Installation instructions:
Requirements:
 *LINUX OS
 * g++-6
 * CMake 3.6 or higher
 * if you want to visualize graphs then python and python-matplotlib 

0.1 Compiling project:
  I assume you're in the project directory.
  ~/t-span$ mkdir build
  ~/t-span$ cd build
  ~/t-span/build$ cmake ..
  ~/t-span/build$ make
* executable is in ~/t-span/bin/t-spanner
1. How to use the project
The project is to be used in the following manner.

1.1 First compile the project according to the previous section.

1.2 Structure of input
  The project accepts input in the following form:
  A json file with the following grammar:
------------------------------START INPUT FILE GRAMMER -------------------------
   INPUT_FILE  :=  '{'
                      "type" : ALGORITHM_TYPE
                      "experiments":
                         '['
                             EXPERIMENT+ 
                         ']' 
                    '}'
    ALGORITHM_TYPE := "2k_spanner" | "3_spanner" | "2k_spanner2"
    EXPERIMENT := EDGE_EXPERIMENT | MAX_STRETCH_EXPERIMENT | DENSITY_EXPERIMENT
    EDGE_EXPERIMENT := '{'
                          "type" : "EdgeCount" ,
                          EDGE_STRETCH_BODY
                        '}'

    MAX_STRETCH_BODY := '{'
                          "type" : "MaxStretch" ,
                           EDGE_STRETCH_BODY
                        '}'
    DENSITY_EXPERIMENT := '{' 
                             "type" : "Density", 
                             "k" : NUMBER,
                             "size" : NUMBER,
                             "densities" : '[' REAL_NUMBER+ ']',
                             "
                          '}'
    EDGE_STRETCH_BODY :=  
                          COMMON_EXPERIMENT_FIELDS,
                          "sizes" : '[' NUMBER+ ']' ,
                          "density" : REAL_NUMBER
    COMMON_EXPERIMENT_FIELDS := "k" : NUMBER,
                               "num_runs_per_size": NUMBER,
                               DISTRIBUTION
    DISTRIBUTION := EMPTY | EXPONENTIAL | GAMMA | WEIBULL
    EXPONENTIAL := 
              "edge_weight_distribution" : "exponential",
              "mean" : REAL_NUMBER
    GAMMA :=
          "edge_weight_distribution" : "gamma"

    WEIBULL :=
          "edge_weight_distribution" : "weibull"              
---------------------------END_INPUT_FILE_GRAMMAR-------------------------------

1.2 Output files:
Such an input file will result into result json files, given an input file
$file$, for each experiment in "experiments", a json result file will be created
under the path out/<MONTH><DAY>/Report_<index of experiment>_random_suffix.
The grammar of a result json file is as follows:

---------------------------START_OUTPUT_FILE_GRAMMAR----------------------------
OUTPUT_FILE := '['
                    RESULT_OUTPUT+
                ']'
RESULT_OUTPUT :=
                '{'
                    TYPE_OF_EXPERIMENT,
                    COMMON_EXPERIMENT_FIELDS
                '}'
COMMON_EXPERIMENT_FIELDS :=
                      "size" : NUMBER,
                      "k" : NUMBER,
                      "density" : REAL_NUMBER,
                      "num_runs" : NUMBER
SPECIFIC_EXPERIMENT_FIELDS := EDGE_COUNT | MAX_STRETCH
EDGE_COUNT := "average_spanner_size" : REAL_NUMBER
MAX_STRETCH := "max_stretch" : REAL_NUMBER
---------------------------END_OUTPUT_FILE_GRAMMER------------------------------


2. run the project from the <build> directory as follows:
  $ report_suffix=../bin/t-spanner --experiments_config_file=$PATH_TO_INPUT_FILE
  $ ls build/out/<MONTH><DAY>/*${report_suffix}* should hold the the results.
  now to visualize the data, you may run the following command:
  $ cd ..
  $ python create_statistics $report_suffix ("edge_report" | "stretch_report" \
      | "density_report" #depending on which results are in the report) \
      "build/out/<MONTH><DAY"


