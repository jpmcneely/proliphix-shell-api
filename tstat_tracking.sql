--
-- Database: `automation`
--

--
-- Table structure for table `tstat_tracking`
--

CREATE TABLE IF NOT EXISTS `tstat_tracking` (
  `GUID` bigint(20) NOT NULL AUTO_INCREMENT,
  `TS_DateTime` datetime NOT NULL,
  `TS_Uptime` time NOT NULL,
  `TS_MainTemp` float(4,1) NOT NULL,
  `TS_RS1State` varchar(10) NOT NULL,
  `TS_RS1Temp` float(4,1) DEFAULT NULL,
  `TS_RS2State` varchar(10) NOT NULL,
  `TS_RS2Temp` float(4,1) DEFAULT NULL,
  `TS_AvgTemp` float(4,1) NOT NULL,
  `TS_HeatSetpoint` float(4,1) NOT NULL,
  `TS_CoolSetpoint` float(4,1) NOT NULL,
  `TS_SetbackStatus` varchar(10) NOT NULL,
  `TS_HVACMode` varchar(10) NOT NULL,
  `TS_HVACState` varchar(10) NOT NULL,
  `TS_FanMode` varchar(10) NOT NULL,
  `TS_SchedClass` varchar(10) NOT NULL,
  `TS_SchedPeriod` varchar(10) NOT NULL,
  `TS_ActivePeriod` varchar(10) NOT NULL,
  `TS_LastUsageReset` datetime NOT NULL,
  `TS_RuntimeHeat` time NOT NULL,
  `TS_RuntimeCool` time NOT NULL,
  `TS_AlarmLowTemp` varchar(10) NOT NULL,
  `TS_AlarmHighTemp` varchar(10) NOT NULL,
  `TS_AlarmFilterReminder` varchar(10) NOT NULL,
  PRIMARY KEY (`GUID`),
  KEY `TS_DateTime` (`TS_DateTime`),
  KEY `TS_SchedClass` (`TS_SchedClass`,`TS_SchedPeriod`),
  KEY `TS_SetbackStatus` (`TS_SetbackStatus`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='Proliphix Thermostat Tracking' AUTO_INCREMENT=1 ;

