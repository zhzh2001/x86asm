tracelog.exe -start counters -f counters.etl -eflag CSWITCH+PROC_THREAD+LOADER -PMC BranchMispredictions,BranchInstructions:CSWITCH
sha256.exe
xperf -stop counters
xperf -merge counters.etl pmc_counters_merged.etl
xperf -i pmc_counters_merged.etl -o pmc_counters.txt