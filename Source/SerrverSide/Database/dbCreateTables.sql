-- ------------------------------------------------------------------------------------
--                                                                                    -
--   Copyright(C) 2017 Ali Seyedof	(seyedof@gmail.com)								  -
--                                                                                    -
-- ------------------------------------------------------------------------------------

-- --------------------------------------------
-- Create tables
-- --------------------------------------------

-- Change the database name to match your own pre-created database
USE dbModelPublish ;


-- This table holds file path
CREATE TABLE tblFileAddress
	(
		id int NOT NULL PRIMARY KEY AUTO_INCREMENT, -- id for file
		FilePathName varchar(128),					-- File path name on disk
		Size int NOT NULL							-- File size in bytes
	) ;


-- This table holds advertisemant
CREATE TABLE tblAdvertisement
	(
		id int NOT NULL PRIMARY KEY AUTO_INCREMENT, -- id for ad
		OwnerId int NOT NULL,						-- Ad owner id
		FilePathName varchar(128),					-- File path name of ad image
		AdUrl varchar(128)							-- Ad forwarding url
	) ;


-- This table holds model resource
CREATE TABLE tblModelDesc
	(
		id int NOT NULL PRIMARY KEY AUTO_INCREMENT, -- id for model
		PCFileAddressId int default -1,				-- File id for PC version
		MobileFileAddressId int default -1,			-- File id for mobile version
		ModelName varchar(32),						-- Model name
		ModelDesc varchar(512)						-- Model description
	) ;


-- This table holds owners
CREATE TABLE tblOwnerDesc
	(
		id int NOT NULL PRIMARY KEY AUTO_INCREMENT, -- id for owner
		FullName varchar(64),						-- Full name of owner
		Username varchar(64) NOT NULL,				-- Owner username
		Password varchar(64)						-- Owner password
	) ;


-- This table holds subscriptions
CREATE TABLE tblSubscription
	(
		id int NOT NULL PRIMARY KEY AUTO_INCREMENT, -- id for session entry
		OwnerId int NOT NULL,						-- Owner id for this subscription
		ModelId int NOT NULL,						-- Model id for this subscription
		AdId int,									-- Advertisement logo id
		Status int default -1						-- Subscription status (active, expired, banned, etc.)
	) ;

-- INSERT INTO tblDummySessionIds (SessionId, UserId) VALUES( 'Token aa12345654321bb', 1396 ) ;
-- INSERT INTO tblDummyProductIds (PackageName, ProductId) VALUES( 'alakigame', 34 ) ;
