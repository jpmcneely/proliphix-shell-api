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
  #echo $RTNDATA
}

function getData {
  # Collects data from the thermostat and returns it to calling function
  # In : OID_String Variable_Type
  # Out: Formatted Data

  OID=$1
  TYP=$2

  RTNDATA=`curl -s --data ${OID}= http://${USERID}:${PASSWD}@${IPADDR}/get | tr "+" " "`
  if [ ! "$RTNDATA" ]; then
    RTNDATA="NoResp"
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
  *)
    RTNDATA="ERR"
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
    echo "ERR"
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
    RTNDATA="ERR"
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
    RTNDATA="ERR"
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
    echo "ERR"
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
    echo "ERR"
    return 1
  fi
}

function readSystemTime {
  # Calls OID2.5.1 - Currect System Time
  # In : No Input
  # Out: Currect Time YYYY-MM-DD HH:MM:SS

  CURSECS=`readSystemTimeSecs`
  if [ $CURSECS != "ERR" ]; then
    date -u +"%Y-%m-%d %H:%M:%S" -d@${CURSECS}
    return 0
  else
    echo "ERR"
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
    echo "ERR"
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
      echo "ERR"
      return 1
      ;;
    esac
  else
    echo "ERR"
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
      echo "ERR"
      return 1
      ;;
    esac
  else
    echo "ERR"
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
      echo "ERR"
      return 1
      ;;
    esac
  else
    echo "ERR"
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
      echo "ERR"
      return 1
      ;;
    esac
  else
    echo "ERR"
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
    echo "ERR"
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
    echo "ERR"
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
      echo "ERR"
      return 1
      ;;
    esac
  else
    echo "ERR"
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
      echo "ERR"
      return 1
      ;;
    esac
  else
    echo "ERR"
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
      echo "ERR"
      return 1
      ;;
    esac
  else
    echo "ERR"
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
      echo "ERR"
      return 1
      ;;
    esac
  else
    echo "ERR"
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
    echo "ERR"
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
    echo "ERR"
    return 1
    ;;
  esac
  OID="OID4.3.2.${DEV}"
  getData $OID temp
  if [ $? -ne 1 ]; then
    echo $RTNDATA
    return 0
  else
    echo "ERR"
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
    echo "ERR"
    return 1
    ;;
  esac
  setData $OID $setMode
  if [ $? -ne 1 ]; then
    readThermHvacMode 
    return 0
  else
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
    echo "ERR"
    return 1
    ;;
  esac
  setData $OID $setMode
  if [ $? -ne 1 ]; then
    readThermFanMode
    return 0
  else
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
    echo "ERR"
    return 1
    ;;
  esac
  setData $OID $setMode
  if [ $? -ne 1 ]; then
    readThermSetbackStatus
    return 0
  else
    return 1
  fi
}

# EOF
