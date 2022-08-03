# ----------------------------------------------------------------------------#
# *** dcube.sh README ***
# Filename:     dcube.sh
# Author:       Michael Baddeley
# Date:         06/07/21
# Description:  Script to post jobs to and get results from D-Cube.
# ----------------------------------------------------------------------------#
function help {
echo " ./dcube.sh -LIST        RETURN JOBS SUMMARY"
echo "     --last=             get last x number of jobs"
echo "     --days=             get last x days of jobs"
echo "     -n                  get by job name"
echo ""
echo " ./dcube.sh -GET         RETURN COMPLETED JOB SERIAL LOGS OR RESULTS"
echo "     -l                  get serial logs"
echo "     -r                  get job result summary"
echo "     --last=             get last x number of jobs"
echo "     --days=             get last x days of jobs"
echo "     -n                  get by job name"
echo "     --job=xxxxx-yyyyy   get jobs from xxxxx to yyyyy"
echo "     -clean              clean the ./results folder where logs are saved"
echo "     -a                  append to previously returned list"
echo ""
echo " ./dcube.sh -DELETE      MASS DELETION OF PENDING JOBS"
echo "     --job=xxxxx-yyyyy   get jobs from xxxxx to yyyyy"
echo ""
echo " ./dcube.sh -POST        QUEUE NEW JOBS"
echo "     -e                  name of folder to compile in .../dcube"
echo "     -n                  job name"
echo "     -d                  job description"
echo "     -sha                add git sha to job description"
echo "     -p                  protocol number to run on dcube"
echo "     -nopatch            turn off binary patching"
echo "     -templab            use templab based on templab.csv"
echo "     --j=*               dcube jamming level"
echo "     --s=*               dcube output serial logs"
echo "     -m                  specify a makearg to be included in compilation"
echo "     --range=xxx-yyy     specify the range of the makearg"
echo "     --dur=*             dcube job duration (120/180/300/600)"
echo "     --layout=xxx-yyy    run this job across layouts x-y (e.g. 1, or 1-4)"
echo "     --data=*            dcube packet size (8/32/64)"
echo "     --period=*          dcube packet periodicity in ms (0 (aperiodic))"
echo ""
echo " ---------"
echo " EXAMPLES:"
echo " ---------"
echo " Get a summary of the last 10 jobs:"
echo "  \$ ./dcube.sh -LIST --last=10"
echo " Get jobs 1234 to 1300:"
echo "  \$ ./dcube.sh -GET --job=1234-1300"
echo " Compile examples/dcube for sky and post to dcube protocol 6775."
echo "  - NO BINARY PATCHING"
echo "  \$ ./dcube.sh -POST TARGET=sky -e dcube -n TEST -p 6775 -d test_0 -nopatch"
echo " Compile examples/dcube for nrf52840 and post to dcube protocol 6667."
echo "  - WITH BINARY PATCHING"
echo "  - JAMMING LEVEL 3"
echo "  - DURATION 10 MINS (dflt 3 mins)"
echo "  \$ ./dcube.sh -POST TARGET=nrf52840 -e dcube -n TEST -p 6667 --dur=600 --j=3 -d test_1"
exit 1
}

# --------------------------------------------------------------------------- #
# View CSV responses from D-Cube
# --------------------------------------------------------------------------- #
function pretty_csv {
    column -t -s, -n "$@" | less -F -S -X -K
}

# --------------------------------------------------------------------------- #
# PHP scripts to access the API
# --------------------------------------------------------------------------- #
function create_job {
  php dc_queue_job.php $KEY $PROTOCOL $LAYOUT $TRAFFIC $DATALEN $PATCHING "$NAME" \
  "$DESC$1" $DUR $SERIAL $JAMMING $PRIORITY "../../examples/$EXAMPLE/node.ihex" $TEMPLAB \
  $START_DELAY $MSG_DELTA $SITE
}

function list_jobs {
  echo "$KEY $1 $2 $3 $4 $SITE"
  php -f dc_list_jobs.php $KEY $1 $2 $3 $4 $SITE
}

function get_job {
  # FIXME: We can actually filter by DAYS or NAME in here but for now let's use
  #        list_jobs in the script
  php -f dc_get_job.php $KEY $1 $2 $3 0 0 $SITE
}

function get_job_description {
  DESC=$(php -f dc_get_job_description.php $KEY $1 $SITE)
}

function delete_job {
  php dc_delete_job.php $KEY $1 $SITE
}

function compile {
  # save the pwd before we start going into example directories
  CURRENT_DIR=$PWD
  cd "../../examples/$EXAMPLE"
  echo "Compiling..."
  echo " > EXAMPLE: $PWD"
  echo " > TARGET: $TARGET"
  # echo " > MAKEARGS: $@"
  echo " > CMD: make clean TARGET=$TARGET && make -j16 TARGET=$TARGET BOARD=dk DEPLOYMENT=dcube $@"


  # set objcopy according to target
  if [[ $TARGET == "nrf52840" ]]; then
    echo " > BOARD: dk"
    make clean TARGET=$TARGET && make -j16 TARGET=$TARGET BOARD=dk DEPLOYMENT=dcube $@
    arm-none-eabi-objcopy ./build/$TARGET/dk/node.hex -O ihex node.ihex
  elif [[ $TARGET == "sky" ]]; then
    make clean TARGET=$TARGET && make -j16 TARGET=$TARGET DEPLOYMENT=dcube $@
    msp430-objcopy ./build/$TARGET/node.sky -O ihex node.ihex
  else
    echo "ERROR: Unknown TARGET $TARGET! Valid targets are nrf52840 and sky!"
    exit 1;
  fi

  # back to reality
  cd $CURRENT_DIR
}

# --------------------------------------------------------------------------- #
# Bash Arguments
# --------------------------------------------------------------------------- #
PARAMS=""
while (( "$#" )); do
  case "$1" in
    TARGET=*)
      TARGET=${1:7}
      shift
      ;;
    TESTBED=*)
      TESTBED=${1:8}
      shift
      ;;
    KEY=*)
      KEY=${1:4}
      shift
      ;;
    SITE=*)
      SITE=${1:5}
      shift
      ;;
    # POST jobs
    -POST)
      POST=1
      shift
      ;;
    # POST jobs
    -DELETE)
      DELETE=1
      shift
      ;;
    -e)
      EXAMPLE=$2
      shift 2
      ;;
    -n)
      NAME=$2
      shift 2
      ;;
    -d)
      DESC=$2
      shift 2
      ;;
    -sha)
      SHA=1
      shift
      ;;
    -p)
      PROTOCOL=$2
      shift 2
      ;;
    -templab)
      TEMPLAB=$2
      shift 2
      ;;
    -pattern)
      PATTERN=$2
      shift 2
      ;;
    -nopatch)
      PATCHING=0
      shift
      ;;
    --j=*)
      JAMMING=${1:4}
      shift
      ;;
    -pri)
      PRIORITY=1
      shift
      ;;
    --s=*)
      SERIAL=${1:4}
      shift
      ;;
    -m)
      METRIC=$2
      shift 2
      ;;
    --range=*)
      RANGE=${1:8}
      START_RANGE=$(cut -d'-' -f1 <<< $RANGE)
      END_RANGE=$(cut -d'-' -f2 <<<  $RANGE)
      shift
      ;;
    --dur=*)
      DUR=${1:6}
      shift
      ;;
    --layout=*)
      LAYOUTS=${1:9}
      START_LAYOUT=$(cut -d'-' -f1 <<< $LAYOUTS)
      END_LAYOUT=$(cut -d'-' -f2 <<<  $LAYOUTS)
      shift
      ;;
    --data=*)
      DATALEN=${1:7}
      shift
      ;;
    --period=*)
      TRAFFIC=${1:9}
      shift
      ;;
    --start=*)
      START_DELAY=${1:8}
      shift
      ;;
    --delta=*)
      MSG_DELTA=${1:8}
      shift
      ;;
    # GET results
    -GET)
      GET=1
      shift
      ;;
    --job=*)
      JOBS=${1:6}
      START_JOB=$(cut -d'-' -f1 <<< $JOBS)
      END_JOB=$(cut -d'-' -f2 <<<  $JOBS)
      shift
      ;;
    --last=*)
      LAST=${1:7}
      shift
      ;;
    -a)
      APPEND=1
      shift
      ;;
    -l)
      LOGS=1
      shift
      ;;
    -r)
      RESULTS=1
      shift
      ;;
    # PLOT results
    -PLOT)
      PLOT=1
      shift
      ;;
    -suites)
      SUITES=$2
      shift 2
      ;;
    -x)
      X=$2
      shift 2
      ;;
    -y)
      Y=$2
      shift 2
      ;;
    -step)
      STEP=$2
      shift 2
      ;;
    -c)
      COMPARE=1
      shift
      ;;
    --title=*)
      TITLE=${1:8}
      shift
      ;;
    --legend=*)
      LEGEND=${1:9}
      shift
      ;;
    # Get a summary of all jobs
    -LIST)
      LIST=1
      shift
      ;;
    --days=*)
      DAYS=${1:7}
      shift
      ;;
    -v)
      VERBOSE=1
      shift
      ;;
    -clean)
      CLEAN=1
      shift
      ;;
    --f=*)
      FILE=${1:4}
      shift
      ;;
    --help)
      help
      shift 2
      ;;
    # MISC
    -*|--*=)
      echo "Error: Unsupported arg $1" >&2
      exit 1
      ;;
    *) # preserve positional arguments
      PARAMS="$PARAMS $1"
      shift
      ;;
  esac
done

# D-CUBE Private Key (DO NOT SHARE!!!) Check to see if a key filename is passed,
# if not then use default key.pub
if [[ -v KEY ]]; then
  KEY=`cat $KEY.pub`
else
  KEY=`cat key.pub`
fi

# D-CUBE site (e.g., Graz)
[ -z "$SITE" ]    && SITE="https://iti-testbed.tugraz.at"

# set positional arguments in their proper place
eval set -- "$PARAMS"
echo "Positional ARGS: "$@
MAKEARGS="$@"
if [[ -z $@ ]]; then
  ARGS=""
else
  # lowercase
  ARGS=$(sed -e 's/\(.*\)/\L\1/ ' <<< $@)
  # get rid of repeated words after '='
  ARGS=$(sed -e 's/\b\([a-z]\+\)[=,\n]\1/\1/g' <<< $ARGS)
  # remove 'board=dk' and '='
  ARGS=$(sed -r 's/board=[a-zA-Z0-9_]+\s//g' <<< $ARGS)
  # remove '_' and '='
  ARGS=$(sed -r 's/[_=]+//g' <<< $ARGS)
  # convert spaces to '_'
  ARGS=$(sed -r 's/[ ]+/_/g' <<< $ARGS)
  ARGS="_"$ARGS
fi
echo $ARGS

if [[ -v SHA ]]; then
  # add git SHA to descriptor
  SHA=$(git rev-parse --short HEAD)
  DESC=$DESC$ARGS"_"$SHA
fi

if [[ -z $POST && -z $DELETE && -z $GET && -z $LIST && -z $PLOT ]]; then
  echo 'Error: Need either -POST -DELETE -GET or -LIST opton!'
  exit 1
fi

# --------------------------------------------------------------------------- #
# DELETE Jobs
# --------------------------------------------------------------------------- #
if [[ -v DELETE ]]; then
  if [[ (-z $JOBS) ]]; then
    echo 'Error: Must specify job number! (--job=)'
    exit 1
  fi

  echo "Delete jobs..."
  if ! [[ $START_JOB =~ ^[0-9]+$ || $END_JOB =~ ^[0-9]+$ ]]; then
    echo "Error: Incorrect job number!"
    exit 1
  fi
  if [[ $START_JOB == $END_JOB ]]; then
    # Single Job
    echo " > DELETE job $START_JOB...";
    delete_job $START_JOB;
    exit 1
  else
    # Range of jobs
    echo " > DELETE jobs: $START_JOB to $END_JOB..."
    for ((i=$START_JOB; i<=$END_JOB; i++)) do
      echo " > DELETE job $i...";
      delete_job $i;
    done
  fi

fi

# --------------------------------------------------------------------------- #
# POST Jobs
# --------------------------------------------------------------------------- #
if [[ -v POST ]]; then
  if [[ (-z $EXAMPLE || -z $NAME || -z $DESC || -z $PROTOCOL) ]]; then
    echo 'Error: One or more required args are undefined! (-e -p -n -d)'
    exit 1
  fi

  [ -z "$SERIAL" ]      && SERIAL=1
  [ -z "$LAYOUT" ]      && LAYOUT=1
  [ -z "$TRAFFIC" ]     && TRAFFIC=0
  [ -z "$DATALEN" ]     && DATALEN=64
  [ -z "$DUR" ]         && DUR=120
  [ -z "$JAMMING" ]     && JAMMING=0
  [ -z "$PRIORITY" ]    && PRIORITY=0
  [ -z "$START_DELAY" ] && START_DELAY=0
  [ -z "$MSG_DELTA" ]   && MSG_DELTA=0

  if [[ -v TEMPLAB ]]; then
    TEMPLAB=$(cat $TEMPLAB | base64 -w0)
  else
    TEMPLAB=0
  fi

  [ -z "$PATCHING" ]     && PATCHING=1
  if [[ $PATCHING == 1 ]]; then
    # Onlt add TESTBED=dcube if we are using patching
    [ -z "$TESTBED" ]    && TESTBED=dcube
  else
    [ -z "$TESTBED" ]    && TESTBED=nulltb
  fi
  MAKEARGS+=" TESTBED=$TESTBED PATCHING=$PATCHING"

  if [[ -v START_LAYOUT && -v END_LAYOUT ]]; then
    echo " > POST layout suite..."
    compile $MAKEARGS;
    for i in $(seq $START_LAYOUT  1 $END_LAYOUT); do
      LAYOUT=$i;
      echo " ... POST job $DESC to layout $LAYOUT ..."
      create_job "_LAYOUT_$i";
      sleep 1
    done
    exit 1
  fi

  if [[ -v METRIC ]]; then
    echo " > POST $METRIC suite from $START_METRIC to $END_METRIC..."
    for i in $(seq $START_METRIC 1 $END_METRIC); do
      compile $METRIC=$i $MAKEARGS;
      $echo " - POST job $DESC_$METRIC_$i"
      create_job "_$METRIC""_$i";
    done
  fi

  echo " > POST job $DESC..."
  compile $MAKEARGS;
  create_job

  exit 1
fi

# --------------------------------------------------------------------------- #
# GET Jobs
# TODO: Make this much faster by filtering within the query. I mean, do we even
#       need a "list" of jobs to fetch? Really we just need days and name...
#       ...although, there is the fetching of logs... hmmm....
# --------------------------------------------------------------------------- #
if [[ -v GET ]]; then

  [ -z "$APPEND" ]      && APPEND=0

  if [[ (-z $LOGS && -z $RESULTS) ]]; then
    echo 'Error: Must choose to GET logs or results! (-l -r)'
    exit 1
  fi

  if [[ (-z $JOBS && -z $DAYS && -z $LAST && -z $NAME) ]]; then
    echo 'Error: Must define which jobs to get! (--job=* or --days=* or --last=* or -n "name")'
    exit 1
  fi

  # by days or name
  if [[ -v DAYS || -v NAME || -v LAST ]]; then
    if [[ -v DAYS ]]; then
      echo "GET logs from the past $DAYS days..."
    else
      DAYS=0
    fi
    if [[ -v NAME ]]; then
      echo "GET logs with job name $NAME..."
    else
      NAME=0
    fi
    if [[ -v LAST ]]; then
      echo "GET last $LAST jobs..."
    else
      LAST=0
    fi
    list_jobs $DAYS $NAME 0 $LAST
    FILE=dcube_jobs_list.csv
    while IFS= read -r line; do
        arr=("${arr[@]}" $line)
    done < $FILE
  fi
  # by sequence of job numbers
  if [[ -v JOBS ]]; then
    if ! [[ $START_JOB =~ ^[0-9]+$ || $END_JOB =~ ^[0-9]+$ ]]; then
      echo "Error: Incorrect job number!"
      exit 1
    fi
    for ((i=$START_JOB; i<=$END_JOB; i++)) do
      arr=($i "${arr[@]}")
    done
    echo "GET logs ${arr[0]} to ${arr[-1]}..."
  fi

  # Get LOGS...
  if [[ -v LOGS ]]; then
    CURRENT_DIR=$PWD
    mkdir -p ./results
    if [[ -v CLEAN ]]; then
      if [ -d ./results ]; then rm -Rf ./results; fi
      mkdir -p ./results
    fi
    # get jobs
    for i in "${arr[@]}"; do
      echo -ne " > GET log for $i... "
      get_job_description $i
      if [ -z "$DESC" ]; then
        echo " ERROR: Couldn't find job $i!";
      else
        FILE="${i}_$DESC"
        echo " SUCCESS! $FILE"
        cd ./results
        curl "$SITE/api/queue/logs/$i?key=$KEY" -L -o $FILE.zip
        unzip -q $FILE.zip -d $FILE
        rm $FILE.zip
      fi
      cd $CURRENT_DIR
    done
    exit 1
  fi

  # Get RESULTS summary...
  if [[ -v RESULTS ]]; then
    # get jobs
    for i in "${arr[@]}"; do
      echo " > GET metrics for job: $i "
      if [ "$APPEND" -eq "0" ]; then
        get_job $i 0 1;
        APPEND=1
      else
        get_job $i 1 0;
      fi
    done
    pretty_csv dcube_results_readable.csv
  fi

fi

# --------------------------------------------------------------------------- #
# LIST Jobs
# --------------------------------------------------------------------------- #
[ -z "$DAYS" ]    && DAYS=0
[ -z "$NAME" ]    && NAME=0
[ -z "$LAST" ]    && LAST=0

if [[ -v LIST ]]; then
  list_jobs $DAYS $NAME 1 $LAST
  pretty_csv dcube_jobs_list_readable.csv
fi


# --------------------------------------------------------------------------- #
# END
# --------------------------------------------------------------------------- #
echo "FINISHED!"
exit 1
