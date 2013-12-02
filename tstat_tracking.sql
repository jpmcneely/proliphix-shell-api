--
-- Database: `automation`
--

-- --------------------------------------------------------

--
-- Table structure for table `tstat_tracking`
--

CREATE TABLE IF NOT EXISTS `tstat_tracking` (
  `GUID` bigint(20) NOT NULL AUTO_INCREMENT,
  `TS_Date` datetime NOT NULL,
  `TS_MainName` varchar(20) NOT NULL,
  `TS_MainTemp` float(4,1) NOT NULL,
  `TS_MainAvg` varchar(10) NOT NULL,
  `TS_HVACMode` varchar(10) NOT NULL,
  `TS_HVACState` varchar(10) NOT NULL,
  `TS_FanMode` varchar(10) NOT NULL,
  `TS_FanState` varchar(10) NOT NULL,
  `TS_RS1State` varchar(10) NOT NULL,
  `TS_RS1Name` varchar(20) DEFAULT NULL,
  `TS_RS1Temp` float(4,1) DEFAULT NULL,
  `TS_RS1Avg` varchar(10) DEFAULT NULL,
  `TS_RS2State` varchar(10) NOT NULL,
  `TS_RS2Name` varchar(20) DEFAULT NULL,
  `TS_RS2Temp` float(4,1) DEFAULT NULL,
  `TS_RS2Avg` varchar(10) DEFAULT NULL,
  `TS_AvgTemp` float(4,1) NOT NULL,
  `TS_HeatSetpoint` float(4,1) NOT NULL,
  `TS_CoolSetpoint` float(4,1) NOT NULL,
  `TS_SetbackStatus` varchar(10) NOT NULL,
  `TS_SchedClass` varchar(10) NOT NULL,
  `TS_SchedPeriod` varchar(10) NOT NULL,
  `TS_ActivePeriod` varchar(10) NOT NULL,
  `TS_AlarmLowTemp` varchar(10) NOT NULL,
  `TS_AlarmHighTemp` varchar(10) NOT NULL,
  `TS_AlarmFilterReminder` varchar(10) NOT NULL,
  `TS_UsageHeat1` bigint(20) NOT NULL,
  `TS_UsageHeat2` bigint(20) NOT NULL,
  `TS_UsageHeat3` bigint(20) NOT NULL,
  `TS_UsageCool1` bigint(20) NOT NULL,
  `TS_UsageCool2` bigint(20) NOT NULL,
  `TS_UsageFan` bigint(20) NOT NULL,
  `TS_Uptime` varchar(20) NOT NULL,
  `TS_UsageReset` datetime NOT NULL,
  PRIMARY KEY (`GUID`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='Proliphix Thermostat Tracking' AUTO_INCREMENT=1 ;

