CREATE TABLE `weatherlog` (
 `time` timestamp NOT NULL DEFAULT current_timestamp() ON UPDATE current_timestamp(),
 `temperature` float NOT NULL,
 `humidity` float NOT NULL,
 `voltage` float NOT NULL,
 PRIMARY KEY (`time`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci
