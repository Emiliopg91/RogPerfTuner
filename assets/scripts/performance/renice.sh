PID="$1"
NICE_VAL="$2"
IONICE_CL="$3"
IONICE_VAL="$4"

renice -n $NICE_VAL -p $PID && ionice -c $IONICE_CL -n $IONICE_VAL -p $PID