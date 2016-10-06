DROP DATABASE IF EXISTS `hackathon`;
CREATE DATABASE IF NOT EXISTS `hackathon`;
use hackathon;

CREATE TABLE IF NOT EXISTS `firewall-policy` (
    `policy-id` INT(1) UNSIGNED NOT NULL,
    `policy-name` VARCHAR(50) NOT NULL,
    PRIMARY KEY(`policy-id`)
) ENGINE=InnoDB CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `firewall-rule` (
    `rule-id` INT(11) UNSIGNED AUTO_INCREMENT NOT NULL,
    `saddr` INT(1) UNSIGNED DEFAULT NULL,
    `daddr` INT(1) UNSIGNED DEFAULT NULL,
    `stime` TINYINT(1) UNSIGNED,
    `etime` TINYINT(1) UNSIGNED,
    `action` TINYINT(1) UNSIGNED NOT NULL,
    `policy-id` INT(1) UNSIGNED NOT NULL,

    PRIMARY KEY(`rule-id`),
    FOREIGN KEY (`policy-id`) REFERENCES `firewall-policy` (`policy-id`) ON DELETE CASCADE ON UPDATE CASCADE

) ENGINE=InnoDB CHARSET=utf8;
