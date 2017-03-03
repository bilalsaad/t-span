# t-span
Mini project for low dist embeddings


1. How to use the project
The project is to be used in the following manner.

1.1 First compile the project according to the previous section.

1.2 Structure of input
  The project accepts input in the following form:
  A json file with the following grammar:
   INPUT_FILE  :=  '{'
                      "type" : ALGORITHM_TYPE
                      "experiments":
                         '['
                             EXPERIMENT+ 
                         ']' 
                    '}'
    ALGORITHM_TYPE := "2k_spanner" | "3_spanner" | "2k_spanner2"
    EXPERIMENT := 
  

2. run the project from the <build> directory as follows:
  2.1 $../bin/t-spanner --experiments_config_file=$PATH_TO_INPUT_FILE$

