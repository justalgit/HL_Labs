CREATE USER 'lab'@'localhost' IDENTIFIED BY '12345';

CREATE DATABASE itlabs;
GRANT ALL PRIVILEGES ON itlabs.* TO 'lab'@'localhost';
FLUSH PRIVILEGES;

use itlabs;
CREATE TABLE IF NOT EXISTS Person (
    id INT NOT NULL AUTO_INCREMENT, 
    login VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
    first_name VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
    last_name VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
    age TINYINT UNSIGNED NULL,
    PRIMARY KEY (id), UNIQUE KEY login_hash (login), KEY fn (first_name), KEY ln (last_name));
