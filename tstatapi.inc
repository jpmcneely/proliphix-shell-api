# Proliphix Thermostat API Implementation
# Version 1.0
# Release 2013-11-30
# API Revision 1.11 (2007-06-20)
# Copyright (C) 2013 - J.P. McNeely
#
# GPL 2.0 License - See LICENSE for redistribution requirements.

# ------------------------------------------------------------------------------
# Data Send/Receive Functions
# To be used by other functions - Do not call these directly.
# ------------------------------------------------------------------------------

function setData {
  # Writes data to the thermostat
  # In : OID_String Value
  # Out: Return status

  OID=$1
  VAL=$2

  RTNDATA=`curl -s --data ${OID}=${VAL} --data submit=Submit http://${USERID}:${PASSWD}@${IPADDR}/pdp`
  RTNDATAOID=`echo $RTNDATA | awk -F= '{print $1}'`
  RTNDATAVAL=`echo $RTNDATA | awk -F= '{print $2}' | tr -d "&" | tr "+" " "`
  RTNDATA="SET:$RTNDATAOID=$RTNDATAVAL"
}

function getData {
  # Collects data from the thermostat and returns it to calling function
  # In : OID_String Variable_Type
  # Out: Formatted Data

  OID=$1
  TYP=$2

  RTNDATA=`curl -s --data ${OID}= http://${USERID}:${PASSWD}@${IPADDR}/get | tr "+" " "`
  if [ ! "$RTNDATA" ]; then
    RTNDATA="ERR-$OID Communication Failure (getData): $0"
    return 1
  fi

  case $TYP in
  "str")
    RTNDATA=`echo $RTNDATA | awk -F= '{print $2}' | tr -d "&" | tr "+" " "`
    return 0
    ;;
  "int")
    RTNDATA=`echo $RTNDATA | awk -F= '{print $2}' | tr -d "&"`
    return 0
    ;;
  "temp")
    RTNDATA=`echo $RTNDATA | awk -F= '{print $2}' | tr -d "&"`
    RTNDATA=`echo "scale = 1; $RTNDATA / 10.0" | bc`
    return 0
    ;;
  "date")
    RTNDATA=`echo $RTNDATA | awk -F= '{print $2}' | tr -d "&"`
    RTNDATA=`date -u +"%Y-%m-%d %H:%M:%S" -d@${RTNDATA}`
    return 0
    ;;
  "raw")
    echo $RTNDATA
    return 0
    ;;
  *)
    RTNDATA="ERR-$OID: Invalid VarType (getData:Type=$TYP): $0"
    return 1
    ;;
  esac
}

# ------------------------------------------------------------------------------
# Read Functions
# These collect and return read-only data.
# ------------------------------------------------------------------------------

function readCommonDevName {
  # OID1.2 - Device Name
  # In : No Input
  # Out: Current Device Name String
  OID="OID1.2"
  getData $OID str
  if [ $? -ne 1 ]; then
    echo "$RTNDATA"
    return 0
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
}

function readCommonAlarmStatus {
  # OID1.13.2.1-3 - Current Alarm Status
  # In : LowTemp HighTemp Filter
  # Out: Green Yellow Red
  ALARMNAME=$1
  case $ALARMNAME in
  "LowTemp")
    ALRMID=1
    ;;
  "HighTemp")
    ALRMID=2
    ;;
  "Filter")
    ALRMID=3
    ;;
  *)
    RTNDATA="ERR-OID1.13.2.X - Invalid Alarm Filter Name: $0"
    return 1
    ;;
  esac
  OID="OID1.13.2.${ALRMID}"
  getData $OID int
  case $RTNDATA in
  1)
    RTNDATA="Green"
    ;;
  2)
    RTNDATA="Yellow"
    ;; 
  3)
    RTNDATA="Red"
    ;;
  "ERR")
    RTNDATA="ERR-$OID - Invalid Alarm Status: $0"
    return 1
    ;;
  esac
  echo $RTNDATA
  return 0
}

function readSystemUptime {
  # OID2.1.1 - Time Since Boot
  # In : No Input
  # Out: HH:MM:SS Since Boot
  OID="OID2.1.1"
  getData $OID str
  if [ $? -ne 1 ]; then
    echo $RTNDATA
    return 0
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
}

function readSystemTimeSecs {
  # OID2.5.1 - Seconds Since Jan 1 1970
  # In : No Input
  # Out: Seconds
  OID="OID2.5.1"
  getData $OID int
  if [ $? -ne 1 ]; then
    echo $RTNDATA
    return 0
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
}

function readSystemTime {
  # OID2.5.1 - Current System Time
  # In : No Input
  # Out: Currect Time YYYY-MM-DD HH:MM:SS
  OID="OID2.5.1"
  getData $OID date
  if [ $? -ne 1 ]; then
    echo $RTNDATA
    return 0
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
}

function readSystemMimModelNumber {
  # OID2.7.1 - System Model Number
  # In : No Input
  # Out: Model Number
  OID="OID2.7.1"
  getData $OID str
  if [ $? -ne 1 ]; then
    echo $RTNDATA
    return 0
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
}

function readThermHvacMode {
  # OID4.1.1 - Current Mode of HVAC Operation
  # In : No Input
  # Out: Mode String
  OID="OID4.1.1"
  getData $OID int
  if [ $? -ne 1 ]; then
    case $RTNDATA in
    1)
      echo "Off"
      return 0
      ;;
    2)
      echo "Heat"
      return 0
      ;;
    3)
      echo "Cool"
      return 0
      ;;
    4)
      echo "Auto"
      return 0
      ;;
    *)
      echo "ERR-$OID - Invalid Mode: $0"
      return 1
      ;;
    esac
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
}

function readThermHvacState {
  # OID4.1.2 - Current State of HVAC Operation
  # In : No Input
  # Out: State String
  OID="OID4.1.2"
  getData $OID int
  if [ $? -ne 1 ]; then
    case $RTNDATA in
    1)
      echo "Initializing"
      return 0
      ;;
    2)
      echo "Off"
      return 0
      ;;
    3)
      echo "Heat"
      return 0
      ;;
    4)
      echo "Heat - First and Second Stage"
      return 0
      ;;
    5)
      echo "Heat - Aux"
      return 0
      ;;
    6)
      echo "Cool"
      return 0
      ;;
    7)
      echo "Cool - First and Second Stage"
      return 0
      ;;
    8)
      echo "Wait: Compressor Delay"
      return 0
      ;;
    9)
      echo "Wait: Reset Relays"
      return 0
      ;;
    *)
      echo "ERR-$OID - Invalid State: $0"
      return 1
      ;;
    esac
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
}

function readThermFanMode {
  # OID4.1.3 - Current Mode of Fan Operation
  # In : No Input
  # Out: Mode String
  OID="OID4.1.3"
  getData $OID int
  if [ $? -ne 1 ]; then
    case $RTNDATA in
    1)
      echo "Auto"
      return 0
      ;;
    2)
      echo "On"
      return 0
      ;;
    3)
      echo "Schedule"
      return 0
      ;;
    *)
      echo "ERR-$OID - Invalid Mode: $0"
      return 1
      ;;
    esac
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
}

function readThermFanState {
  # OID4.1.4 - Current State of Fan Operation
  # In : No Input
  # Out: State String
  OID="OID4.1.4"
  getData $OID int
  if [ $? -ne 1 ]; then
    case $RTNDATA in
    1)
      echo "Init"
      return 0
      ;;
    2)
      echo "Off"
      return 0
      ;;
    3)
      echo "On"
      return 0
      ;;
    *)
      echo "ERR-$OID - Invalid State: $0"
      return 1
      ;;
    esac
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
}

function readThermSetbackHeat {
  # OID4.1.5 - Current Heat SetPoint in Fahrenheit
  # In : No Input
  # Out: Heat Temp Setting
  OID="OID4.1.5"
  getData $OID temp
  if [ $? -ne 1 ]; then
    echo $RTNDATA
    return 0
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
}

function readThermSetbackCool {
  # OID4.1.6 - Current Cool SetPoint in Fahrenheit
  # In : No Input
  # Out: Cool Temp Setting
  OID="OID4.1.6"
  getData $OID temp
  if [ $? -ne 1 ]; then
    echo $RTNDATA
    return 0
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
}

function readThermSetbackStatus {
  # OID4.1.9 - Current State of Setback Operation
  # In : No Input
  # Out: Status String
  OID="OID4.1.9"
  getData $OID int
  if [ $? -ne 1 ]; then
    case $RTNDATA in
    1)
      echo "Normal"
      return 0
      ;;
    2)
      echo "Hold"
      return 0
      ;;
    3)
      echo "Override"
      return 0
      ;;
    *)
      echo "ERR-$OID - Invalid State: $0"
      return 1
      ;;
    esac
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
}

function readThermCurrentPeriod {
  # OID4.1.10 - Current Schedule Period
  # In : No Input
  # Out: Schedule Period String
  OID="OID4.1.10"
  getData $OID int
  if [ $? -ne 1 ]; then
    case $RTNDATA in
    1)
      echo "Morning"
      return 0
      ;;
    2) 
      echo "Day"
      return 0
      ;;
    3)
      echo "Evening"
      return 0
      ;;
    4)
      echo "Night"
      return 0
      ;;
    *)
      echo "ERR-$OID - Invalid Period: $0"
      return 1
      ;;
    esac
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
}

function readThermCurrentClass {
  # OID4.1.11 - Current Schedule Class
  # In : No Input
  # Out: Schedule Class String
  OID="OID4.1.11"
  getData $OID int
  if [ $? -ne 1 ]; then
    case $RTNDATA in
    1)
      echo "In"
      return 0
      ;;
    2)
      echo "Out"
      return 0
      ;;
    3)
      echo "Away"
      return 0
      ;;
    *)
      echo "ERR-$OID - Invalid Class: $0"
      return 1
      ;;
    esac
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
}

function readThermActivePeriod {
  # OID4.1.12 - Current Active Period
  # In : No Input
  # Out: Active Period String
  OID="OID4.1.12"
  getData $OID int
  if [ $? -ne 1 ]; then
    case $RTNDATA in
    1)
      echo "Scheduled Morning"
      return 0
      ;;
    2)
      echo "Scheduled Day"
      return 0
      ;;
    3)
      echo "Scheduled Evening"
      return 0
      ;;
    4)
      echo "Scheduled Night"
      return 0
      ;;
    5)
      echo "Hold"
      return 0
      ;;
    6)
      echo "Override"
      return 0
      ;;
    *)
      echo "ERR-$OID - Invalid Period: $0"
      return 1
      ;;
    esac
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
}

function readThermAverageTemp {
  # OID4.1.13 - Current Averaged Temperature
  # In : No Input
  # Out: Averaged Temp
  OID="OID4.1.13"
  getData $OID temp
  if [ $? -ne 1 ]; then
    echo $RTNDATA
    return 0
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
}

function readThermConfigFilterReminderPending {
  # OID4.2.9 - Filter Reminder Alarm Pending State
  # In : No Input
  # Out: State String
  OID="OID4.2.9"
  getData $OID int
  if [ $? -ne 1 ]; then
    case $RTNDATA in
    1)
      echo "No"
      return 0
      ;;
    2)
      echo "Yes"
      return 0
      ;;
    *)
      echo "ERR-$OID - Invalid State: $0"
      return 1
      ;;
    esac
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
}

function readThermConfigLowTempPending {
  # OID4.2.11 - Zone Low Temperature Alarm Pending State
  # In : No Input
  # Out: State String
  OID="OID4.2.11"
  getData $OID int
  if [ $? -ne 1 ]; then
    case $RTNDATA in
    1)
      echo "No"
      return 0
      ;;
    2)
      echo "Yes"
      return 0
      ;;
    *)
      echo "ERR-$OID - Invalid State: $0"
      return 1
      ;;
    esac
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
}

function readThermConfigHighTempPending {
  # OID4.2.13 - Zone High Temperature Alarm Pending State
  # In : No Input
  # Out: State String
  OID="OID4.2.13"
  getData $OID int
  if [ $? -ne 1 ]; then
    case $RTNDATA in
    1)
      echo "No"
      return 0
      ;;
    2)
      echo "Yes"
      return 0
      ;;
    *)
      echo "ERR-$OID - Invalid State: $0"
      return 1
      ;;
    esac
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
}

function readThermSensorTemp {
  # OID4.3.2.1-3 - Current Sensor Temperature
  # In : Sensor Name
  # Out: Current Temp
  SENSOR=$1
  case $SENSOR in
  "Local")
    DEV=1
    ;;
  "RS1")
    DEV=2
    ;;
  "RS2")
    DEV=3
    ;;
  *)
    echo "ERR-OID4.3.2.X - Invalid Sensor ID: $0"
    return 1
    ;;
  esac
  OID="OID4.3.2.${DEV}"
  getData $OID temp
  if [ $? -ne 1 ]; then
    echo $RTNDATA
    return 0
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
}

function readThermSensorCorrection {
  # OID4.3.4.2-3 - Remote Sensor Correction
  # In : Remote Sensor ID
  # Out: Temp Offset in Fahrenheit
  if [ $1 ]; then
    case $1 in
    "RS1")
      DEV=2
      ;;
    "RS2")
      DEV=3
      ;;
    *)
      echo "ERR-OID4.3.4.X - Invalid Sensor ID: $0"
      return 1
      ;;
    esac
  else
    echo "ERR-OID4.3.4.X - Invalid Parameter Count: $0"
    return 1
  fi
  OID="OID4.3.4.${DEV}"
  getData $OID temp
  if [ $? -ne 1 ]; then
    echo $RTNDATA
    return 0
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
}

function readThermSensorName {
  # OID4.3.5.2-3 - Remote Sensor Name
  # In : Remote Sensor ID
  # Out: Current Sensor Name String
  if [ $1 ]; then
    case $1 in
    "RS1")
      DEV=2
      ;;
    "RS2")
      DEV=3
      ;;
    *)
      echo "ERR-OID4.3.5.X - Invalid Sensor ID: $0"
      return 1
      ;;
    esac
  else
    echo "ERR-OID4.3.5.X - Invalid Parameter Count: $0"
    return 1
  fi
  OID="OID4.3.5.${DEV}"
  getData $OID str
  if [ $? -ne 1 ]; then
    echo "$RTNDATA"
    return 0
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
}

function readThermSensorState {
  # OID4.3.6.1-3 - Current Sensor State
  # In : Sensor ID
  # Out: State String
  SENSOR=$1
  case $SENSOR in
  "Local")
    DEV=1
    ;;
  "RS1")
    DEV=2
    ;;
  "RS2")
    DEV=3
    ;;
  *)
    echo "ERR-OID4.3.6.X - Invalid Sensor ID: $0"
    return 1
    ;;
  esac
  OID="OID4.3.6.${DEV}"
  getData $OID int
  if [ $? -ne 1 ]; then
    case $RTNDATA in
    0)
      echo "Not Present"
      return 0
      ;;
    1)
      echo "Disabled"
      return 0
      ;;
    2)
      echo "Enabled"
      return 0
      ;;
    *)
      echo "ERR-$OID - Invalid State: $0"
      return 1
      ;;
    esac
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
}

function readThermSensorAverage {
  # OID4.3.8.1-3 - Current Sensor Average Setting
  # In : Sensor ID
  # Out: State String
  SENSOR=$1
  case $SENSOR in
  "Local")
    DEV=1
    ;;
  "RS1")
    DEV=2
    ;;
  "RS2")
    DEV=3
    ;;
  *)
    echo "ERR-OID4.3.8.X - Invalid Sensor ID: $0"
    return 1
    ;;
  esac
  OID="OID4.3.8.${DEV}"
  getData $OID int
  if [ $? -ne 1 ]; then
    case $RTNDATA in
    1)
      echo "Disabled"
      return 0
      ;;
    2)
      echo "Enabled"
      return 0
      ;;
    *)
      echo "ERR-$OID - Invalid Setting: $0"
      return 1
      ;;
    esac
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
}

function readThermSensorType {
  # OID4.3.9.2-3 - Current Remote Sensor Type
  # In : Sensor ID
  # Out: Type String
  SENSOR=$1
  case $SENSOR in
  "RS1")
    DEV=2
    ;;
  "RS2")
    DEV=3
    ;;
  *)
    echo "ERR-OID4.3.9.X - Invalid Sensor ID: $0"
    return 1
    ;;
  esac
  OID="OID4.3.9.${DEV}"
  getData $OID int
  if [ $? -ne 1 ]; then
    case $RTNDATA in
    1)
      echo "Analog"
      return 0
      ;;
    2)
      echo "Thermistor"
      return 0
      ;;
    *)
      echo "ERR-$OID - Invalid Type: $0"
      return 1
      ;;
    esac
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
}

function readThermPeriodStart {
  # OID4.4.1.3.1-3.1-4 - Schedule Period Start Value
  # In : Schedule_Class, Chedule_Period
  # Out: Start Time in Minutes (Min After Midnight)
  OID="OID4.4.1.3"
  CLASS=$1
  PERIOD=$2
  case $CLASS in
  "In")
    INCLASS=1
    ;;
  "Out")
    INCLASS=2
    ;;
  "Away")
    INCLASS=3
    ;;
  *)
    echo "ERR-${OID}.X.X - Invalid Class: $0"
    return 1
    ;;
  esac
  case $PERIOD in
  "Morning")
    INPERIOD=1
    ;;
  "Day")
    INPERIOD=2
    ;;
  "Evening")
    INPERIOD=3
    ;;
  "Night")
    INPERIOD=4
    ;;
  *)
    echo "ERR-${OID}.X.X - Invalid Period: $0"
    return 1
    ;;
  esac
  OID="${OID}.${INCLASS}.${INPERIOD}"
  getData ${OID} int
  if [ $? -ne 1 ]; then
    echo $RTNDATA
    return 0
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
}

function readThermPeriodStartHHMM {
  # Based on OID4.4.1.3.1-3.1-4 - Schedule Period Start Value
  # In : Schedule_Class, Chedule_Period
  # Out: Start Time in HH:MM
  CLASS=$1
  PERIOD=$2
  STARTTIMEMIN=`readThermPeriodStart $CLASS $PERIOD`
  echo `toMilTime $STARTTIMEMIN`
  return 0
}

function readThermPeriodSetbackHeat {
  # OID4.4.1.4.1-3.1-4 - Schedule Period Heat Setback Value
  # In : Schedule_Class, Chedule_Period
  # Out: Heat Setback Temp
  OID="OID4.4.1.4"
  CLASS=$1
  PERIOD=$2
  case $CLASS in
  "In")
    INCLASS=1
    ;;
  "Out")
    INCLASS=2
    ;;
  "Away")
    INCLASS=3
    ;;
  *)
    echo "ERR-${OID}.X.X - Invalid Class: $0"
    return 1
    ;;
  esac
  case $PERIOD in
  "Morning")
    INPERIOD=1
    ;;
  "Day")
    INPERIOD=2
    ;;
  "Evening")
    INPERIOD=3
    ;;
  "Night")
    INPERIOD=4
    ;;
  *)
    echo "ERR-${OID}.X.X - Invalid Period: $0"
    return 1
    ;;
  esac
  OID="${OID}.${INCLASS}.${INPERIOD}"
  getData ${OID} temp
  if [ $? -ne 1 ]; then
    echo $RTNDATA
    return 0
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
}

function readThermPeriodSetbackCool {
  # OID4.4.1.5.1-3.1-4 - Schedule Period Cool Setback Value
  # In : Schedule_Class, Chedule_Period
  # Out: Cool Setback Temp
  OID="OID4.4.1.5"
  CLASS=$1
  PERIOD=$2
  case $CLASS in
  "In")
    INCLASS=1
    ;;
  "Out")
    INCLASS=2
    ;;
  "Away")
    INCLASS=3
    ;;
  *)
    echo "ERR-${OID}.X.X - Invalid Class: $0"
    return 1
    ;;
  esac
  case $PERIOD in
  "Morning")
    INPERIOD=1
    ;;
  "Day")
    INPERIOD=2
    ;;
  "Evening")
    INPERIOD=3
    ;;
  "Night")
    INPERIOD=4
    ;;
  *)
    echo "ERR-${OID}.X.X - Invalid Period: $0"
    return 1
    ;;
  esac
  OID="${OID}.${INCLASS}.${INPERIOD}"
  getData ${OID} temp
  if [ $? -ne 1 ]; then
    echo $RTNDATA
    return 0
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
}

function readThermHeat1Usage {
  # OID4.5.1 - HEAT1 State Usage in Minutes
  # In : No Input
  # Out: Usage in Minutes
  OID="OID4.5.1"
  getData $OID int
  if [ $? -ne 1 ]; then
    echo $RTNDATA
    return 0
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
}

function readThermHeat2Usage {
  # OID4.5.2 - HEAT2 State Usage in Minutes
  # In : No Input
  # Out: Usage in Minutes
  OID="OID4.5.2"
  getData $OID int
  if [ $? -ne 1 ]; then
    echo $RTNDATA
    return 0
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
}

function readThermCool1Usage {
  # OID4.5.3 - COOL1 State Usage in Minutes
  # In : No Input
  # Out: Usage in Minutes
  OID="OID4.5.3"
  getData $OID int
  if [ $? -ne 1 ]; then
    echo $RTNDATA
    return 0
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
}

function readThermCool2Usage {
  # OID4.5.4 - COOL2 State Usage in Minutes
  # In : No Input
  # Out: Usage in Minutes
  OID="OID4.5.4"
  getData $OID int
  if [ $? -ne 1 ]; then
    echo $RTNDATA
    return 0
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
}

function readThermFanUsage {
  # OID4.5.5 - Fan On Usage in Minutes
  # In : No Input
  # Out: Usage in Minutes
  OID="OID4.5.5"
  getData $OID int
  if [ $? -ne 1 ]; then
    echo $RTNDATA
    return 0
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
}

function readThermLastUsageResetSecs {
  # OID4.5.6 - Last Reset Time in Seconds Since Jan 1 1970
  # In : No Input
  # Out: Time Since Reset in Epoch Seconds
  OID="OID4.5.6"
  getData $OID int
  if [ $? -ne 1 ]; then
    echo $RTNDATA
    return 0
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
}

function readThermLastUsageReset {
  # OID4.5.6 - Last Reset Time in Date/Time Format
  # In : No Input
  # Out: Date/Time String
  OID="OID4.5.6"
  getData $OID date
  if [ $? -ne 1 ]; then
    echo $RTNDATA
    return 0
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
}

function readThermUsageOptions {
  # OID4.5.8 - If 1, Include Heat1/Heat2 in Fan Usage Results
  # In : No Input
  # Out: Option Bit
  OID="OID4.5.8"
  getData $OID int
  if [ $? -ne 1 ]; then
    echo $RTNDATA
    return 0
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
}

function readThermHeat3Usage {
  # OID4.5.9 - HEAT3 (Aux) State Usage in Minutes
  # In : No Input
  # Out: Usage in Minutes
  OID="OID4.5.9"
  getData $OID int
  if [ $? -ne 1 ]; then
    echo $RTNDATA
    return 0
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
}

# ------------------------------------------------------------------------------
# Write Functions
# These set read-write data.
# ------------------------------------------------------------------------------

function writeCommonDevName {
  # OID1.2 - Device Name
  # In : Name - Replace ' ' (space) character with '+' (plus) character
  #             (ex: 'Zone+One' sets to 'Zone One')
  # Out: Return status
  newName=$1
  OID="OID1.2"
  setData $OID $newName
  if [ $? -ne 1 ]; then
    return 0
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
}

function writeThermHvacMode {
  # OID4.1.1 - Set Current Mode of HVAC Operation
  # In : Mode String
  # Out: Mode String After Command
  newMode=$1
  OID="OID4.1.1"
  case $newMode in
  "Off")
    setMode=1
    ;;
  "Heat")
    setMode=2
    ;;
  "Cool")
    setMode=3
    ;;
  "Auto")
    setMode=4
    ;;
  *)
    echo "ERR-$OID - Invalid Mode: $0"
    return 1
    ;;
  esac
  setData $OID $setMode
  if [ $? -ne 1 ]; then
    readThermHvacMode 
    return 0
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
}

function writeThermFanMode {
  # OID4.1.3 - Set Current Mode of Fan Operation
  # In : Mode String
  # Out: Mode String After Command
  newMode=$1
  OID="OID4.1.3"
  case $newMode in
  "Auto")
    setMode=1
    ;;
  "On")
    setMode=2
    ;;
  "Schedule")
    setMode=3
    ;;
  *)
    echo "ERR-$OID - Invalid Mode: $0"
    return 1
    ;;
  esac
  setData $OID $setMode
  if [ $? -ne 1 ]; then
    readThermFanMode
    return 0
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
} 

function writeThermSetbackHeat {
  # OID4.1.5 - Set Heat SetPoint in Fahrenheit
  # In : New Setting
  # Out: Setting After Command
  newTemp=$1
  newTemp=`echo "$newTemp * 10" | bc | awk -F. '{print $1}'`
  OID="OID4.1.5"
  setData $OID $newTemp
  if [ $? -ne 1 ]; then
    readThermSetbackHeat
    return 0
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
}

function writeThermSetbackCool {
  # OID4.1.6 - Set Cool SetPoint in Fahrenheit
  # In : New Setting
  # Out: Setting After Command
  newTemp=$1
  newTemp=`echo "$newTemp * 10" | bc | awk -F. '{print $1}'`
  OID="OID4.1.6"
  setData $OID $newTemp
  if [ $? -ne 1 ]; then
    readThermSetbackCool
    return 0
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
}

function writeThermSetbackStatus {
  # OID4.1.9 - Set Current Mode of Setback Operation
  # In : Mode String
  # Out: Mode String After Command
  newMode=$1
  OID="OID4.1.9"
  case $newMode in
  "Normal")
    setMode=1
    ;;
  "Hold")
    setMode=2
    ;;
  *)
    echo "ERR-OID4.1.9 - Invalid Mode: $0"
    return 1
    ;;
  esac
  setData $OID $setMode
  if [ $? -ne 1 ]; then
    readThermSetbackStatus
    return 0
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
}

function writeThermConfigFilterReminderPending {
  # OID4.2.9 - Clear Filter Reminder Alarm
  # In : No Input
  # Out: State String
  OID="OID4.2.9"
  setClear=3
  setData $OID $setClear
  if [ $? -ne 1 ]; then
    readThermConfigFilterReminderPending
    return 0
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
}

function writeThermConfigLowTempPending {
  # OID4.2.11 - Clear Zone Low Temperature Alarm
  # In : No Input
  # Out: State String
  OID="OID4.2.11"
  setClear=3
  setData $OID $setClear
  if [ $? -ne 1 ]; then
    readThermConfigLowTempPending
    return 0
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
}

function writeThermConfigHighTempPending {
  # OID4.2.13 - Clear Zone High Temperature Alarm
  # In : No Input
  # Out: State String
  OID="OID4.2.13"
  setClear=3
  setData $OID $setClear
  if [ $? -ne 1 ]; then
    readThermConfigHighTempPending
    return 0
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
}

function writeThermSensorCorrection {
  # OID4.3.4.2-3 - Set Remote Sensor Correction
  # In : Sensor_Name Offset_in_Fahrenheit
  # Out: Offset After Setting
  if [ $1 ]; then
    if [ $2 ]; then
      case $1 in
      "RS1")
        DEV=2
        ;;
      "RS2")
        DEV=3
        ;;
      *)
        echo "ERR-OID4.3.4.X - Invalid Sensor ID: $0"
        return 1
        ;;
      esac
      OFFSET=$2
      OFFSET=`echo "$OFFSET * 10" | bc | awk -F. '{print $1}'`
    else
      echo "ERR-OID4.3.4.X - Invalid Parameter Count: $0"
      return 1
    fi
  else
    echo "ERR-OID4.3.4.X - Invalid Parameter Count: $0"
    return 1
  fi
  OID="OID4.3.4.${DEV}"
  setData $OID $OFFSET
  if [ $? -ne 1 ]; then
    readThermSensorCorrection $1
    return 0
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
}

function writeThermSensorName {
  # OID4.3.5.2-3 - Remote Sensor Name
  # In : Sensor ID, Name - Replace ' ' (space) character with '+' (plus) character
  #                        (ex: 'Zone+Two' sets to 'Zone Two')
  # Out: Return status
  if [ $1 ]; then
    if [ $2 ]; then
      case $1 in
      "RS1")
        DEV=2
        ;;
      "RS2")
        DEV=3
        ;;
      *)
        echo "ERR-OID4.3.5.X - Invalid Sensor ID: $0"
        return 1
        ;;
      esac
      newName=$2
    else
      echo "ERR-OID4.3.5.X - Invalid Parameter Count: $0"
      return 1
    fi
  else
    echo "ERR-OID4.3.5.X - Invalid Parameter Count: $0"
    return 1
  fi
  OID="OID4.3.5.${DEV}"
  setData $OID $newName
  if [ $? -ne 1 ]; then
    return 0
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
}

function writeThermSensorState {
  # OID4.3.6.2-3 - Set Sensor State
  # In : Sensor_ID, State
  # Out: State After Setting
  if [ $1 ]; then
    if [ $2 ]; then
      case $1 in
      "RS1")
        DEV=2
        ;;
      "RS2")
        DEV=3
        ;;
      *)
        echo "ERR-OID4.3.6.X - Invalid Sensor ID: $0"
        return 1
        ;;
      esac
      RAWSTATE=$2
    else
      echo "ERR-OID4.3.6.X - Invalid Parameter Count: $0"
      return 1
    fi
  else
    echo "ERR-OID4.3.6.X - Invalid Parameter Count: $0"
    return 1
  fi
  case $RAWSTATE in
  "Disable")
    STATE=1
    ;;
  "Enable")
    STATE=2
    ;;
  *)
    echo "ERR-OID4.3.6.X - Invalid State: $0"
    return 1
    ;;
  esac
  OID="OID4.3.6.${DEV}"
  setData $OID $STATE
  if [ $? -ne 1 ]; then
    readThermSensorState $1
    return 0
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
}

function writeThermSensorAverage {
  # OID4.3.8.1-3 - Set Sensor Average Inclusion
  # In : Sensor_ID, Setting
  # Out: Setting Status
  if [ $1 ]; then
    if [ $2 ]; then
      case $1 in
      "Local")
        DEV=1
        ;;
      "RS1")
        DEV=2
        ;;
      "RS2")
        DEV=3
        ;;
      *)
        echo "ERR-OID4.3.8.X - Invalid Sensor ID: $0"
        return 1
        ;;
      esac
      RAWSET=$2
    else
      echo "ERR-OID4.3.8.X - Invalid Parameter Count: $0"
      return 1
    fi
  else
    echo "ERR-OID4.3.8.X - Invalid Parameter CountL $0"
    return 1
  fi
  case $RAWSET in
  "Disable")
    STATE=1
    ;;
  "Enable")
    STATE=2
    ;;
  *)
    echo "ERR-OID4.3.8.X - Invalid Setting: $0"
    return 1
    ;;
  esac
  OID="OID4.3.8.${DEV}"
  setData $OID $STATE
  if [ $? -ne 1 ]; then
    readThermSensorAverage $1
    return 0
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
}

function writeThermSensorType {
  # OID4.3.9.2-3 - Set Remote Sensor Type
  # In : Sensor_ID, Type
  # Out: Type After Setting
  if [ $1 ]; then
    if [ $2 ]; then
      case $1 in
      "RS1")
        DEV=2
        ;;
      "RS2")
        DEV=3
        ;;
      *)
        echo "ERR-OID4.3.9.X - Invalid Sensor ID: $0"
        return 1
        ;;
      esac
      RAWTYPE=$2
    else
      echo "ERR-OID4.3.9.X - Invalid Parameter Count: $0"
      return 1
    fi
  else
    echo "ERR-OID4.3.9.X - Invalid Parameter Count: $0"
    return 1
  fi
  case $RAWTYPE in
  "Analog")
    TYPE=1
    ;;
  "Thermistor")
    TYPE=2
    ;;
  *)
    echo "ERR-OID4.3.9.X - Invalid Type: $0"
    return 1
    ;;
  esac
  OID="OID4.3.9.${DEV}"
  setData $OID $TYPE
  if [ $? -ne 1 ]; then
    readThermSensorType $1
    return 0
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
}

function writeThermHeat1Usage {
  # OID4.5.1 - Set HEAT1 State Usage in Minutes
  # In : Number of Minutes
  # Out: Minutes After Setting
  if [ $1 ]; then
    OID="OID4.5.1"
    setData $OID $1
    if [ $? -ne 1 ]; then
      readThermHeat1Usage
      return 0
    else
      echo "ERR-$OID - Failed Return Code: $0"
      return 1
    fi
  else
    echo "ERR-$OID - Invalid Parameter Count: $0"
    return 1
  fi
}

function writeThermHeat2Usage {
  # OID4.5.2 - Set HEAT2 State Usage in Minutes
  # In : Number of Minutes
  # Out: Minutes After Setting
  if [ $1 ]; then
    OID="OID4.5.2"
    setData $OID $1
    if [ $? -ne 1 ]; then
      readThermHeat2Usage
      return 0
    else
      echo "ERR-$OID - Failed Return Code: $0"
      return 1
    fi
  else
    echo "ERR-$OID - Invalid Parameter Count: $0"
    return 1
  fi
}

function writeThermCool1Usage {
  # OID4.5.3 - Set COOL1 State Usage in Minutes
  # In : Number of Minutes
  # Out: Minutes After Setting
  if [ $1 ]; then
    OID="OID4.5.3"
    setData $OID $1
    if [ $? -ne 1 ]; then
      readThermCool1Usage
      return 0
    else
      echo "ERR-$OID - Failed Return Code: $0"
      return 1
    fi
  else
    echo "ERR-$OID - Invalid Parameter Count: $0"
    return 1
  fi
}

function writeThermCool2Usage {
  # OID4.5.4 - Set COOL2 State Usage in Minutes
  # In : Number of Minutes
  # Out: Minutes After Setting
  if [ $1 ]; then
    OID="OID4.5.4"
    setData $OID $1
    if [ $? -ne 1 ]; then
      readThermCool2Usage
      return 0
    else
      echo "ERR-$OID - Failed Return Code: $0"
      return 1
    fi
  else
    echo "ERR-$OID - Invalid Parameter Count: $0"
    return 1
  fi
}

function writeThermFanUsage {
  # OID4.5.5 - Set FAN State Usage in Minutes
  # In : Number of Minutes
  # Out: Minutes After Setting
  if [ $1 ]; then
    OID="OID4.5.5"
    setData $OID $1
    if [ $? -ne 1 ]; then
      readThermFanUsage
      return 0
    else
      echo "ERR-$OID - Failed Return Code: $0"
      return 1
    fi
  else
    echo "ERR-$OID - Invalid Parameter Count: $0"
    return 1
  fi
}

function writeThermLastUsageReset {
  # OID4.5.6 - Set Last Usage Reset Time
  # In : Date (YYYY-MM-DD HH:MM:SS)
  # Out: Reset Date after Setting
  OID="OID4.5.6"
  if [ $2 ]; then
    setData $OID $1 $2
    if [ $? -ne 1 ]; then
      readThermLastUsageResetDateTime
      return 0
    else
      echo "ERR-$OID - Failed Return Code: $0"
      return 1
    fi
  else
    echo "ERR-$OID - Invalid Parameter Count: $0"
    return 1
  fi
}

function writeThermLastUsageResetNow {
  # OID4.5.6 - Set Last Usage Reset Time
  # In : No Input
  # Out: Reset Date after Setting = Now
  OID="OID4.5.6"
  setData $OID `dateEpoch`
  if [ $? -ne 1 ]; then
    readThermLastUsageReset
    return 0
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
}

function writeThermHeat3Usage {
  # OID4.5.9 - Set HEAT3 (Aux) State Usage in Minutes
  # In : Number of Minutes
  # Out: Minutes After Setting
  if [ $1 ]; then
    OID="OID4.5.9"
    setData $OID $1
    if [ $? -ne 1 ]; then
      readThermHeat3Usage
      return 0
    else
      echo "ERR-$OID - Failed Return Code: $0"
      return 1
    fi
  else
    echo "ERR-$OID - Invalid Parameter Count: $0"
    return 1
  fi
}

# ------------------------------------------------------------------------------
# Undocumented Function
# These are functions found in the Web UI that are not in the API docs.
# Due to the undocumented status, these are read-only.
# ------------------------------------------------------------------------------

function readSWVer {
  # OID1.1 - Read Software Version
  # In : No Input
  # Out: Version
  OID="OID1.1"
  getData $OID str
  if [ $? -ne 1 ]; then 
    echo "$RTNDATA" | awk '{print $2}'
    return 0
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
}

function readIPAddress {
  # OID1.4 - Read IP Address
  # In : No Input
  # Out: IP Address
  OID="OID1.4"
  getData $OID str
  if [ $? -ne 1 ]; then
    echo "$RTNDATA"
    return 0
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
}

function readNetmask {
  # OID1.5 - Read Netmask
  # In : No Input
  # Out: Netmask
  OID="OID1.5"
  getData $OID str
  if [ $? -ne 1 ]; then
    echo "$RTNDATA"
    return 0
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
}

function readGateway {
  # OID1.6 - Read Gateway
  # In : No Input
  # Out: Gateway IP Address
  OID="OID1.6"
  getData $OID str
  if [ $? -ne 1 ]; then
    echo "$RTNDATA"
    return 0
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
}

function readSerialNo {
  # OID1.8 - Read Serial Number
  # In : No Input
  # Out: Serial Number
  OID="OID1.8"
  getData $OID str
  if [ $? -ne 1 ]; then
    echo "$RTNDATA"
    return 0
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
}

function writeSystemReset {
  # OID1.9 - Reset the System
  # In : No Input
  # Out: Return Code
  OID="OID1.9"
  setData $OID 2
  if [ $? -ne 1 ]; then
    return 0
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
}

function readRemoteAccessServerAddress {
  # OID1.10.3 - Remote Access Server IP
  # In : No Input
  # Out: Server Address
  OID="OID1.10.3"
  getData $OID str
    if [ $? -ne 1 ]; then
    echo "$RTNDATA"
    return 0
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
}

function readRemoteAccessServerPort {
  # OID1.10.4 - Remote Access Server Port
  # In : No Input
  # Out: Server Port
  OID="OID1.10.4"
  getData $OID str
  if [ $? -ne 1 ]; then
    echo "$RTNDATA"
    return 0
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
}

function readRemoteAccessInterval {
  # OID1.10.5 - RA Check Interval in Minutes
  # In : No Input
  # Out: Minutes
  OID="OID1.10.5"
  getData $OID int
  if [ $? -ne 1 ]; then
    echo "$RTNDATA"
    return 0
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
}

function readCustomerID {
  # OID1.10.10 - Customer ID
  # In : No Input
  # Out: Customer ID
  OID="OID1.10.10"
  getData $OID str
  if [ $? -ne 1 ]; then
    echo "$RTNDATA"
    return 0
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
}

function readSiteName {
  # OID1.10.9 - Read Site Name
  # In : No Input
  # Out: Site Name
  OID="OID1.10.9"
  getData $OID str
  if [ $? -ne 1 ]; then
    echo "$RTNDATA"
    return 0
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
}

function readMacAddress {
  # OID1.15 - Read MAC Address
  # In : No Input
  # Out: MAC
  OID="OID1.15"
  getData $OID str
  if [ $? -ne 1 ]; then
    echo "$RTNDATA"
    return 0
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
}

function readUsername {
  # OID2.3.3 - Read Username
  # In : No Input
  # Out: Username
  OID="OID2.3.3"
  getData $OID str
  if [ $? -ne 1 ]; then
    echo "$RTNDATA"
    return 0
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
}

function readPassword {
  # OID2.3.4 - Read Password
  # In : No Input
  # Out: Password
  OID="OID2.3.4"
  getData $OID str
  if [ $? -ne 1 ]; then
    echo "$RTNDATA"
    return 0
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
}

function readHWVer {
  # OID2.7.2 - Read Hardware Version
  # In : No Input
  # Out: Version
  OID="OID2.7.2"
  getData $OID str
  if [ $? -ne 1 ]; then
    echo "$RTNDATA"
    return 0
  else
    echo "ERR-$OID - Failed Return Code: $0"
    return 1
  fi
}

# ------------------------------------------------------------------------------
# Utility Functions
# Generally useful function.
# ------------------------------------------------------------------------------

function dateEpoch {
  # Return current UTC date/time in seconds since Jan 1 1970
  date -u +"%s"
}

function toMilTime
{
  # Convert Minutes After Midnight to 24Hr Clock Time
  MIN=$1
  HOUR=$[$MIN / 60]
  HMIN=$[$HOUR * 60]
  RMIN=$[$MIN - $HMIN]
  printf "%02d:" ${HOUR}
  printf "%02d\n" ${RMIN}
}

# EOF
